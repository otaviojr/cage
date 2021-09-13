#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wlr/util/log.h>

#include "io_mapping.h"

struct cg_io_mapping* io_mapping_new(char* device, char* output){
    struct cg_io_mapping* mapping = calloc(1, sizeof(struct cg_io_mapping));
    mapping->device_name = strdup((const char*)device);
    mapping->output_name = strdup((const char*)output);

    return mapping;
}
