#ifndef IO_MAPPING_H
#define IO_MAPPING_H

#include <wayland-server-core.h>

struct io_mapping {
    char* device_name;
    char* output_name;

    struct wl_list link;
};

struct io_mapping* io_mapping_new(char* device, char* output);

#endif
