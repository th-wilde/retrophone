#ifndef _RP_CONFIG_WIZARD_H_
#define _RP_CONFIG_WIZARD_H_
#include "rp_config_struct.h"

extern void rpconf_wiz_init(struct rp_config_struct);
extern int rpconf_wiz_input(char input);
extern void rpconf_wiz_quit();

#endif