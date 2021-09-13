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

void application_new(struct cg_server *server, char* executable_list);
struct cg_application *application_find_by_pid(struct cg_server* server, pid_t pid);
void cleanup_all_applications(struct cg_server* server);
bool application_spawn(struct cg_application* application);
bool application_next_spawn(struct cg_server* server);
void application_end_signal(struct cg_application* application);

#endif
