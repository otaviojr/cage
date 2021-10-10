#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <wlr/util/log.h>

#include "server.h"
#include "output.h"

#include "libconfig.h"

static char* tolowercase(char* s) {
  for(char *p=s; *p; p++) *p=tolower(*p);
  return s;
}

bool config_get_decoration(struct cg_server* server)
{
    config_setting_t *setting;
    int cfg_decoration = 0;

    setting = config_lookup(&server->cage_cfg, "cage");
    if(setting != NULL){
        if(!config_setting_lookup_bool(setting, "decoration", &cfg_decoration)){
            goto config_get_decoration_end;
        }
    }

config_get_decoration_end:
    return cfg_decoration == 0;
}

enum cg_multi_output_mode config_get_output_mode(struct cg_server* server)
{
    config_setting_t *setting;
    char* cfg_mode;

    setting = config_lookup(&server->cage_cfg, "cage");
    if(setting != NULL){
        if(!config_setting_lookup_string(setting, "mode", (const char**)&cfg_mode)){
            goto config_get_mode_end;
        }

        if(strcmp("last", tolowercase(cfg_mode)) == 0){
            return CAGE_MULTI_OUTPUT_MODE_LAST;
        }
    }
config_get_mode_end:
    return CAGE_MULTI_OUTPUT_MODE_EXTEND;
}

enum wl_output_transform config_output_get_transform(struct cg_server* server, const char* output_name)
{
    config_setting_t *setting;
    config_setting_t *output;
    char* cfg_output_name;
    char* cfg_output_transform;

    setting = config_lookup(&server->cage_cfg, "cage.outputs");
    if(setting != NULL){
        int count = config_setting_length(setting);
        for(int i = 0; i < count; i++){
            output = config_setting_get_elem(setting, i);
            if(!config_setting_lookup_string(output, "name", (const char**)&cfg_output_name)){
                goto config_output_get_transform_end;
            }

            if(strcmp(output_name, cfg_output_name)==0){
                if(!config_setting_lookup_string(output, "transform", (const char**)&cfg_output_transform)){
                    goto config_output_get_transform_end;
                }

                if(strcmp("normal", tolowercase(cfg_output_transform)) == 0 ||
                        strcmp("0", tolowercase(cfg_output_transform)) == 0){
                    goto config_output_get_transform_end;
                } else if(strcmp("90", tolowercase(cfg_output_transform)) == 0){
                    return WL_OUTPUT_TRANSFORM_90;
                } else if(strcmp("180", tolowercase(cfg_output_transform)) == 0){
                    return WL_OUTPUT_TRANSFORM_180;
                } else if(strcmp("270", tolowercase(cfg_output_transform)) == 0){
                    return WL_OUTPUT_TRANSFORM_270;
                } else if(strcmp("flipped", tolowercase(cfg_output_transform)) == 0){
                    return WL_OUTPUT_TRANSFORM_FLIPPED;
                } else if(strcmp("flipped_90", tolowercase(cfg_output_transform)) == 0){
                    return WL_OUTPUT_TRANSFORM_FLIPPED_90;
                } else if(strcmp("flipped_180", tolowercase(cfg_output_transform)) == 0){
                    return WL_OUTPUT_TRANSFORM_FLIPPED_180;
                } else if(strcmp("flipped_270", tolowercase(cfg_output_transform)) == 0){
                    return WL_OUTPUT_TRANSFORM_FLIPPED_270;
                }
            }
        }
    }

config_output_get_transform_end:
    return WL_OUTPUT_TRANSFORM_NORMAL;
}

double config_output_get_scale(struct cg_server* server, const char* output_name)
{
    config_setting_t *setting;
    config_setting_t *output;
    char* cfg_output_name;
    double cfg_output_scale = 1.0;

    setting = config_lookup(&server->cage_cfg, "cage.outputs");
    if(setting != NULL){
        int count = config_setting_length(setting);
        for(int i = 0; i < count; i++){
            output = config_setting_get_elem(setting, i);
            if(!config_setting_lookup_string(output, "name", (const char**)&cfg_output_name)){
                goto config_output_get_scale_end;
            }

            if(strcmp(output_name, cfg_output_name)==0){
                if(!config_setting_lookup_float(output, "scale", &cfg_output_scale)){
                    wlr_log(WLR_ERROR, "Error parsing config file scale");
                    goto config_output_get_scale_end;
                }
            }
        }
    }

config_output_get_scale_end:
    return cfg_output_scale;
}

struct cg_application* config_output_get_application(struct cg_server* server, const char* output_name)
{
    config_setting_t *setting;
    config_setting_t *output;
    config_setting_t *application;
    char* cfg_output_name;
    char* cfg_output_application_name;
    char* cfg_output_application_exec;
    struct cg_application* cfg_output_application = NULL;

    setting = config_lookup(&server->cage_cfg, "cage.outputs");
    if(setting != NULL){
        int count = config_setting_length(setting);
        for(int i = 0; i < count; i++){
            output = config_setting_get_elem(setting, i);
            if(!config_setting_lookup_string(output, "name", (const char**)&cfg_output_name)){
                goto config_output_get_application_end;
            }

            if(strcmp(output_name, cfg_output_name)==0){
                application = config_setting_lookup(output, "application");
                if(application != NULL){
                    if(!config_setting_lookup_string(application, "name", (const char**)&cfg_output_application_name)){
                        goto config_output_get_application_end;
                    }

                    if(!config_setting_lookup_string(application, "exec", (const char**)&cfg_output_application_exec)){
                        goto config_output_get_application_end;
                    }

                    cfg_output_application = application_new(server, (const char*)cfg_output_application_exec, output_name);
                }
            }
        }
    }

config_output_get_application_end:
    return cfg_output_application;
}
