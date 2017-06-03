#ifndef _RP_CONFIG_STRUCT_H_
#define _RP_CONFIG_STRUCT_H_
#define RP_CONFIG_STRUCT_STRING_BUFFER_DEFINE 256

struct rp_config_struct {
   char  sip_server[RP_CONFIG_STRUCT_STRING_BUFFER_DEFINE];
   char  sip_username[RP_CONFIG_STRUCT_STRING_BUFFER_DEFINE];
   char  sip_password[RP_CONFIG_STRUCT_STRING_BUFFER_DEFINE];
   char  sip_realm[RP_CONFIG_STRUCT_STRING_BUFFER_DEFINE];
   char  stun_server[RP_CONFIG_STRUCT_STRING_BUFFER_DEFINE];
   char  wifi_ssid[RP_CONFIG_STRUCT_STRING_BUFFER_DEFINE];
   char  wifi_password[RP_CONFIG_STRUCT_STRING_BUFFER_DEFINE];
};

#endif