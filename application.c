#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <proc/readproc.h>
#include <wlr/util/log.h>

#include "application.h"
#include "util.h"

void application_new(struct cg_server *server, char* executable_list){
    struct cg_application* application;
    char * executable, *argv, *end_executable, *end_argv;
    int argc;

    end_executable = NULL;
    executable = strtok_r(executable_list, ";", &end_executable);
    while( executable != NULL ) {
        application = calloc(1, sizeof(struct cg_application));
        application->server = server;
        application->pid = 0;
        application->argv = NULL;

        end_argv = NULL;
        argc = 0;
        argv = strtok_r(executable, " ", &end_argv);
        while( argv != NULL ) {
            argc++;
            if(application->argv){
                application->argv = realloc(application->argv,argc*sizeof(char*));
            } else {
                application->argv = malloc(argc*sizeof(char*));
            }
            application->argv[argc-1] = strdup(argv);
            argv = strtok_r(NULL, " ", &end_argv);
        }

        application->argv[argc] = NULL;

        wl_list_insert(&server->applications, &application->link);
        executable = strtok_r(NULL, ";",&end_executable);
    }
}

static bool
set_cloexec(int fd)
{
	int flags = fcntl(fd, F_GETFD);

	if (flags == -1) {
		wlr_log(WLR_ERROR, "Unable to set the CLOEXEC flag: fnctl failed");
		return false;
	}

	flags = flags | FD_CLOEXEC;
	if (fcntl(fd, F_SETFD, flags) == -1) {
		wlr_log(WLR_ERROR, "Unable to set the CLOEXEC flag: fnctl failed");
		return false;
	}

	return true;
}

static int
sigchld_handler(int fd, uint32_t mask, void *data)
{
    struct cg_application* application = data;

    application->pid = 0;

	/* Close Cage's read pipe. */
	close(fd);

	if (mask & WL_EVENT_HANGUP) {
		wlr_log(WLR_DEBUG, "Child process closed normally: %s", application->argv[0]);
	} else if (mask & WL_EVENT_ERROR) {
		wlr_log(WLR_DEBUG, "Connection closed by server: %s", application->argv[0]);
	}

	//wl_display_terminate(application->server->wl_display);
	return 0;
}

bool application_spawn(struct cg_application* application)
{
    int fd[2];
	if (pipe(fd) != 0) {
		wlr_log(WLR_ERROR, "Unable to create pipe");
		return false;
	}

	pid_t pid = fork();
	if (pid == 0) {
		sigset_t set;
		sigemptyset(&set);
		sigprocmask(SIG_SETMASK, &set, NULL);
		/* Close read, we only need write in the primary client process. */
		close(fd[0]);
		execvp(application->argv[0], application->argv);
		_exit(1);
	} else if (pid == -1) {
		wlr_log_errno(WLR_ERROR, "Unable to fork");
		return false;
	}

    if(!pid) return false;

	/* Set this early so that if we fail, the client process will be cleaned up properly. */
	application->pid = pid;

	if (!set_cloexec(fd[0]) || !set_cloexec(fd[1])) {
		return false;
	}

	/* Close write, we only need read in Cage. */
	close(fd[1]);

	struct wl_event_loop *event_loop = wl_display_get_event_loop(application->server->wl_display);
	uint32_t mask = WL_EVENT_HANGUP | WL_EVENT_ERROR;
	application->sigchld_source = wl_event_loop_add_fd(event_loop, fd[0], mask, sigchld_handler, application);

	wlr_log(WLR_DEBUG, "Child process created with pid %d", pid);
	return true;
}

bool application_next_spawn(struct cg_server* server)
{
    struct cg_application *application;
	wl_list_for_each (application, &server->applications, link) {
        if(application->pid == 0){
            return application_spawn(application);
        }
    }
    return false;
}

static void get_proc(pid_t pid, proc_t* proc_info)
{
    memset(proc_info, 0, sizeof(proc_t));
    PROCTAB *pt_ptr = openproc(PROC_FILLSTATUS | PROC_PID, &pid);
    if(readproc(pt_ptr, proc_info) != 0) {
        wlr_log(WLR_DEBUG, "Program: %s", proc_info->cmd);
        wlr_log(WLR_DEBUG, "PID: %d", pid);
        wlr_log(WLR_DEBUG, "PPID: %d", proc_info->ppid);
    } else {
        wlr_log(WLR_ERROR, "PID %d not found", pid);
    }
    closeproc(pt_ptr);
}

struct cg_application *application_find_by_pid(struct cg_server* server, pid_t pid)
{
    struct cg_application *application;
    proc_t process_info;
    pid_t ppid = 0;

	wl_list_for_each (application, &server->applications, link) {

        if(application->pid == pid){
            return application;
        }

        ppid = pid;
        do{
            get_proc(ppid, &process_info);
            ppid = process_info.ppid;
            if(ppid == application->pid){
                return application;
            }
        } while(ppid > 0);
    }
    return NULL;
}

void cleanup_all_applications(struct cg_server* server)
{
    struct cg_application *application;
	wl_list_for_each (application, &server->applications, link) {
        int status;
    	waitpid(application->pid, &status, 0);

    	if (WIFEXITED(status)) {
    		wlr_log(WLR_DEBUG, "Child exited normally with exit status %d", WEXITSTATUS(status));
    	} else if (WIFSIGNALED(status)) {
    		wlr_log(WLR_DEBUG, "Child was terminated by a signal (%d)", WTERMSIG(status));
    	}
	}
}

void application_end_signal(struct cg_application* application)
{
    int ret;

    ret = kill(application->pid,SIGINT);
    printf("Signaling application to quit : %d",ret);
}
