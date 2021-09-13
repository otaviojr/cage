#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wlr/util/log.h>

#include "io_mapping.h"

static char* strdup (const char* s)
{
  size_t slen = strlen(s);
  char* result = malloc(slen + 1);
  if(result == NULL)
  {
    return NULL;
  }

  memcpy(result, s, slen+1);
  return result;
}

struct io_mapping* io_mapping_new(char* device, char* output){
    struct io_mapping* mapping = calloc(1, sizeof(struct io_mapping));
    mapping->device_name = strdup((const char*)device);
    mapping->output_name = strdup((const char*)output);

    return mapping;
}
