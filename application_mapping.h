#ifndef CG_APPLICATION_MAPPING_H
#define CG_APPLICATION_MAPPING_H

#include <wayland-server-core.h>

struct cg_application_mapping {
    char* application_name;
    char* output_name;

    struct wl_list link;
};

struct cg_application_mapping* application_mapping_new(char* application, char* output);
char* find_output_from_application(struct cg_server* server, char* application);

#endif
