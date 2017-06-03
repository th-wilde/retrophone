#ifndef _RP_CONFIG_H_
#define _RP_CONFIG_H_
#include "rp_config_struct.h"

extern void rpconf_read(struct rp_config_struct config);
extern void rpconf_write(struct rp_config_struct config);
extern void rpconf_setup(struct rp_config_struct config);

#endif