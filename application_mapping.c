#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wlr/util/log.h>
#include "server.h"
#include "application_mapping.h"

struct cg_application_mapping* application_mapping_new(char* application, char* output){
    struct cg_application_mapping* mapping = calloc(1, sizeof(struct cg_application_mapping));
    mapping->application_name = strdup((const char*)application);
    mapping->output_name = strdup((const char*)output);

    return mapping;
}

char* find_output_from_application(struct cg_server* server, char* application)
{
    struct cg_application_mapping *mapping;
	wl_list_for_each (mapping, &server->application_mappings, link) {
		if(strstr(application, mapping->application_name) != NULL){
            return mapping->output_name;
        }
	}
    return NULL;
}
