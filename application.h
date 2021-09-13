#ifndef CG_APPLICATION_H
#define CG_APPLICATION_H

#include <wayland-server-core.h>
#include "server.h"

struct cg_application {
    struct cg_server *server;
    pid_t pid;

    struct wl_event_source *sigchld_source;

    struct wl_list link;

    char** argv;
};

struct cg_application* application_new(struct cg_server *server, char* executable_list);

#endif
