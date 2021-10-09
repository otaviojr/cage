#ifndef CG_CONFIGURATION_H
#define CG_CONFIGURATION_H

#include "server.h"

enum cg_multi_output_mode config_get_output_mode(struct cg_server* server);
enum wl_output_transform config_output_get_transform(struct cg_server* server, const char* output_name);
double config_output_get_scale(struct cg_server* server, const char* output_name);
struct cg_application* config_output_get_application(struct cg_server* server, const char* output_name);

#endif
