#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "rp_config_wizard.h"
#include "rp_speak.h"
#include "rp_config.h"
#include "rp_config_struct.h"
#include "rp_number2char.h"
#include "string_s.h"
#include "rp_speak.h"

static FILE* alphanum_write_handle;
static FILE* alphanum_read_handle;

static pthread_mutex_t dialTimeoutIndicator_mutex;
static int dialTimeoutIndicator = -1;
pthread_t input_timeout_thread_handle;
pthread_t read_alphanum_thread_handle;

enum config_wiz_state { MENU = 0, SIP_SERV, SIP_USER, SIP_PASS, SIP_RELM, STUN_SERV, WIFI_SSID, WIFI_PASS, INET_TEST };
static enum config_wiz_state config_wiz_current = MENU;
static int edit_enabled = 0;
static struct rp_config_struct config_wiz_config;

static char alphanum_input[RP_CONFIG_STRUCT_STRING_BUFFER_DEFINE];

void *input_timeout_thread(void *arg)
{
	sleep(10);
	
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
	switch(config_wiz_current){
		case SIP_SERV:
			strcpy_s(config_wiz_config.sip_server, RP_CONFIG_STRUCT_STRING_BUFFER_DEFINE, alphanum_input);
		break;
		case SIP_USER:
			strcpy_s(config_wiz_config.sip_username, RP_CONFIG_STRUCT_STRING_BUFFER_DEFINE, alphanum_input);
		break;
		case SIP_PASS:
			strcpy_s(config_wiz_config.sip_password, RP_CONFIG_STRUCT_STRING_BUFFER_DEFINE, alphanum_input);
		break;
		case SIP_RELM:
			strcpy_s(config_wiz_config.sip_realm, RP_CONFIG_STRUCT_STRING_BUFFER_DEFINE, alphanum_input);
		break;
		case STUN_SERV:
			strcpy_s(config_wiz_config.stun_server, RP_CONFIG_STRUCT_STRING_BUFFER_DEFINE, alphanum_input);
		break;
		case WIFI_PASS:
			strcpy_s(config_wiz_config.wifi_password, RP_CONFIG_STRUCT_STRING_BUFFER_DEFINE, alphanum_input);
		break;
	}
	edit_enabled=0;
	rpconf_wiz_input('\0');
	pthread_mutex_lock(&dialTimeoutIndicator_mutex);
	dialTimeoutIndicator=1;
	pthread_mutex_unlock(&dialTimeoutIndicator_mutex);
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
}

void *read_alphanum_thread(void *arg)
{
	while(feof(alphanum_read_handle)==0){
		char append[] = "_";
		append[0] = fgetc(alphanum_read_handle);
		rpsay_char(append[0]);
		strcat_s(alphanum_input, RP_CONFIG_STRUCT_STRING_BUFFER_DEFINE, append);
	}
}

static char* say_announce_item[] = {
	"",
	"Der aktuelle SIP-Server lautet",
	"Der aktuelle SIP-Benutzernamen lautet",
	"Das aktuelle SIP-Passwort lautet",
	"Der aktuelle SIP-Realm lautet",
	"Der aktuelle STUN-Server lautet",
	"Das aktuelle WLAN-Netz lautet",
	"Das aktuelle WLAN-Passwort lautet"
};

static char* say_announce_change[] = {
	"",
	"Einrichtung des SIP-Server",
	"Einrichtung des SIP-Benutzernamen",
	"Einrichtung des SIP-Passworts",
	"Einrichtung des SIP-Realm",
	"Einrichtung des STUN-Server",
	"Einrichtung des WLAN-Netz",
	"Einrichtung des WLAN-Passwort"
};

static char say_change_alphanum[] = "Buchstaben-Eingabe aktiviert. Eingabe endet wenn 10 Sekunden keine Eingabe erfolgt.";

static char say_change_question[] = "Wählen Sie eins zum ändern oder zwei um zum Konfigurationsmenü zurückzukehren";

void rpconf_wiz_init(struct rp_config_struct config){
	
	int alphanum_write_pipe[2];
	int alphanum_read_pipe[2];
	pipe(alphanum_write_pipe);
	pipe(alphanum_read_pipe);
	
	config_wiz_config = config;
	rpn2c_init(alphanum_read_pipe[0], alphanum_write_pipe[1]);
	
	alphanum_read_handle = fdopen(alphanum_write_pipe[0], "r");
	if(alphanum_read_handle == NULL){
		fprintf(stderr, "rp_config_wizard: Error open alphanum_read_handle");
	}
	
	alphanum_write_handle = fdopen(alphanum_read_pipe[1], "w");
	if(alphanum_write_handle == NULL){
		fprintf(stderr, "rp_config_wizard: Error open alphanum_write_handle");
	}
	
	pthread_create(&read_alphanum_thread_handle, NULL, read_alphanum_thread, NULL);
	pthread_detach(read_alphanum_thread_handle);
	
	rpconf_wiz_input('\0');
	
	
}
int rpconf_wiz_input(char input){
	
	switch(config_wiz_current){
		case MENU:
			switch(input){
				case 'D':
				break;
				case 'H':
					return 1; //leave config menu
					rpconf_wiz_input('\0');
				break;
				case '1':
					config_wiz_current = SIP_SERV;
					rpconf_wiz_input('\0');
				break;
				case '2':
					config_wiz_current = SIP_USER;
					rpconf_wiz_input('\0');
				break;
				case '3':
					config_wiz_current = SIP_PASS;
					rpconf_wiz_input('\0');
				break;
				case '4':
					config_wiz_current = SIP_RELM;
					rpconf_wiz_input('\0');
				break;
				case '5':
					config_wiz_current = STUN_SERV;
					rpconf_wiz_input('\0');
				break;
				case '6':
					config_wiz_current = WIFI_SSID;
					rpconf_wiz_input('\0');
				break;
				case '7':
					config_wiz_current = WIFI_PASS;
					rpconf_wiz_input('\0');
				break;
				default:
					rpsay_string("Sie befinden sich im Konfigurationsmenü");
					rpsay_string("Eins wählen um den SIP-Server einzurichten");
					rpsay_string("Zwei wählen um den SIP-Benutzernamen einzurichten");
					rpsay_string("Drei wählen um das SIP-Passwort einzurichten");
					rpsay_string("Vier wählen um den SIP-Realm einzurichten");
					rpsay_string("Fünf wählen um den STUN-Server einzurichten");
					rpsay_string("Sechs wählen um das WLAN-Netz einzurichten");
					rpsay_string("Sieben wählen um das WLAN-Passwort einzurichten");
					rpsay_string("Auflegen um das Konfigurationsmenü zu verlassen");
				break;
			}			
		break;
		default:
			if(edit_enabled==0){
				rpsay_string(say_announce_item[config_wiz_current]);
				switch(config_wiz_current){
					case SIP_SERV:
						rpsay_spell(config_wiz_config.sip_server);
					break;
					case SIP_USER:
						rpsay_spell(config_wiz_config.sip_username);
					break;
					case SIP_PASS:
						rpsay_spell(config_wiz_config.sip_password);
					break;
					case SIP_RELM:
						rpsay_spell(config_wiz_config.sip_realm);
					break;
					case STUN_SERV:
						rpsay_spell(config_wiz_config.stun_server);
					break;
					case WIFI_SSID:
						rpsay_spell(config_wiz_config.wifi_ssid);
					break;
					case WIFI_PASS:
						rpsay_spell(config_wiz_config.wifi_password);
					break;
				}
				rpsay_string(say_change_question);
				if(input == '1'){
					edit_enabled=1;
					rpconf_wiz_input('\0');
				}else if(input == '0'){
					config_wiz_current = MENU;
					rpconf_wiz_input('\0');
				}
			}else if (edit_enabled==1){
				rpsay_string(say_announce_change[config_wiz_current]);
				if(config_wiz_current==WIFI_SSID){
					rpsay_string("Wählen Sie aus folgenden WLAN-Netzen");
					edit_enabled==3;
				}else{
					rpsay_string(say_change_alphanum);
					alphanum_input[0] = 0x00;
					edit_enabled==2;
				}
			}else if (edit_enabled==2){
				pthread_mutex_unlock(&dialTimeoutIndicator_mutex);
				if(dialTimeoutIndicator==0){
					pthread_cancel(input_timeout_thread_handle);
				}
				dialTimeoutIndicator=0;
				pthread_mutex_unlock(&dialTimeoutIndicator_mutex);
				pthread_create(&input_timeout_thread_handle, NULL, input_timeout_thread, NULL);
				pthread_detach(input_timeout_thread_handle);
				fputc(input,alphanum_write_handle);
				fflush(alphanum_write_handle);
			}
		break;
		
	}
	return 0;
	
}
void rpconf_wiz_quit(){
	fclose(alphanum_write_handle);
	fclose(alphanum_read_handle);
	pthread_create(&read_alphanum_thread_handle, NULL, input_timeout_thread, NULL);
	pthread_detach(read_alphanum_thread_handle);
}

