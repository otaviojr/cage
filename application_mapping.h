#ifndef CG_APPLICATION_MAPPING_H
#define CG_APPLICATION_MAPPING_H

#include <wayland-server-core.h>
#include "server.h"

struct cg_server;
struct cg_application_mapping {
    char* application_name;
    char* output_name;

    struct wl_list link;
};

struct cg_application_mapping* application_mapping_new(char* application, char* output);
void cleanup_all_applications_mappings(struct cg_server* server);

#endif
