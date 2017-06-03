#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "string_s.h"
#include "rp_config.h"


static void get_config_path(char* config_path, int config_path_buffer){
	config_path[0]=0x00;
	strcat_s(config_path, config_path_buffer, getenv("HOME"));
	strcat_s(config_path, config_path_buffer, "/.retrophone_config.dat");
}

void rpconf_read(struct rp_config_struct config){

	int config_path_buffer = 1024;
	char config_path[config_path_buffer];
	get_config_path(config_path, config_path_buffer);
	
	if( access( config_path, R_OK ) != -1 ) {
		FILE* config_file = fopen(config_path, "rb");
		fread(&config,sizeof(struct rp_config_struct),1,config_file);
		fclose(config_file);
	}else{
		fprintf(stderr, "Unable to read config file: %s", config_path);
	}
	
}
void rpconf_write(struct rp_config_struct config){
	
	int config_path_buffer = 1024;
	char config_path[config_path_buffer];
	get_config_path(config_path, config_path_buffer);
	
	FILE* config_file = fopen(config_path, "wb");
	fwrite(&config,sizeof(struct rp_config_struct),1,config_file);
	fclose(config_file);
	/*}else{
		fprintf(stderr, "Unable to write config file: %s", config_path);
	}*/
	
}

void rpconf_setup(struct rp_config_struct config){
	
	//generate ~/.linphonerc
	
	int linphone_sed_buffer = 1024;
	char linphone_sed[linphone_sed_buffer];
	linphone_sed[0] = 0x00;
	strcat_s(linphone_sed, linphone_sed_buffer, "sed '");
	strcat_s(linphone_sed, linphone_sed_buffer, "s/\\[sip_server\\]/");
	strcat_s(linphone_sed, linphone_sed_buffer, config.sip_server);
	strcat_s(linphone_sed, linphone_sed_buffer, "/g; s/\\[sip_username\\]/");
	strcat_s(linphone_sed, linphone_sed_buffer, config.sip_username);
	strcat_s(linphone_sed, linphone_sed_buffer, "/g; s/\\[sip_password\\]/");
	strcat_s(linphone_sed, linphone_sed_buffer, config.sip_password);
	strcat_s(linphone_sed, linphone_sed_buffer, "/g; s/\\[sip_realm\\]/");
	strcat_s(linphone_sed, linphone_sed_buffer, config.sip_realm);
	strcat_s(linphone_sed, linphone_sed_buffer, "/g; s/\\[sip_realm\\]/");
	strcat_s(linphone_sed, linphone_sed_buffer, config.stun_server);
	strcat_s(linphone_sed, linphone_sed_buffer, "/g' ~/.linphonerc.template > ~/.linphonerc");
	
	system(linphone_sed);
	
	
	//generate /etc/wpa_supplicant/wpa_supplicant.conf
	
	int wpa_supplicant_sed_buffer = 1024;
	char wpa_supplicant_sed[wpa_supplicant_sed_buffer];
	wpa_supplicant_sed[0] = 0x00;
	strcat_s(wpa_supplicant_sed, wpa_supplicant_sed_buffer, "sed '");
	strcat_s(wpa_supplicant_sed, wpa_supplicant_sed_buffer, "s/\\[wifi_ssid\\]/");
	strcat_s(wpa_supplicant_sed, wpa_supplicant_sed_buffer, config.wifi_ssid);
	strcat_s(wpa_supplicant_sed, wpa_supplicant_sed_buffer, "/g; s/\\[wifi_password\\]/");
	strcat_s(wpa_supplicant_sed, wpa_supplicant_sed_buffer, config.wifi_password);
	strcat_s(wpa_supplicant_sed, wpa_supplicant_sed_buffer, "/g' /etc/wpa_supplicant/wpa_supplicant.conf.template > /etc/wpa_supplicant/wpa_supplicant.conf");
	
	system(wpa_supplicant_sed);
}