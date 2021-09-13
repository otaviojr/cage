#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <wlr/util/log.h>

#include "application.h"
#include "util.h"

struct cg_application* application_new(struct cg_server *server, char* executable_list){
    struct cg_application* application;
    char * executable, *argv, *end_executable, *end_argv;
    int argc = 0;

    executable = strtok_r(executable_list, ";", &end_executable);
    while( executable != NULL ) {
        application = calloc(1, sizeof(struct cg_application));
        application->server = server;

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

        application->argv[argc] = (char*)NULL;

        wl_list_insert(&server->applications, &application->link);
        executable = strtok_r(NULL, ";",&end_executable);
    }

    return application;
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

	/* Close Cage's read pipe. */
	close(fd);

	if (mask & WL_EVENT_HANGUP) {
		wlr_log(WLR_DEBUG, "Child process closed normally");
	} else if (mask & WL_EVENT_ERROR) {
		wlr_log(WLR_DEBUG, "Connection closed by server");
	}

	wl_display_terminate(application->server->wl_display);
	return 0;
}

bool application_spawn(struct cg_application* application, struct wl_event_source **sigchld_source)
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
