#ifndef IO_MAPPING_H
#define IO_MAPPING_H

#include <wayland-server-core.h>

struct cg_io_mapping {
    char* device_name;
    char* output_name;

    struct wl_list link;
};

struct cg_io_mapping* io_mapping_new(char* device, char* output);

#endif
