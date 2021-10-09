#ifndef CG_APPLICATION_H
#define CG_APPLICATION_H

#include <wayland-server-core.h>

#include "server.h"
#include "output.h"

struct cg_output;

struct cg_application {
    struct cg_server *server;
    pid_t pid;

    struct wl_event_source *sigchld_source;

    char* output_name;

    char** argv;
};

struct cg_application* application_new(struct cg_server *server, const char* executable_list, const char* output_name);
void application_destroy(struct cg_application* application);
bool application_spawn(struct cg_application* application);
void application_end_signal(struct cg_application* application);
struct cg_application *application_find_by_pid(struct cg_server* server, pid_t pid);

#endif
