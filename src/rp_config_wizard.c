#include "rp_config_wizard.h"

static FILE* alphanum_write_handle;
static FILE* alphanum_read_handle;

void rpconf_wiz_init(struct rp_config_struct){
	
	int alphanum_write_pipe[2];
	int alphanum_read_pipe[2];
	pipe(alphanum_write_pipe);
	pipe(alphanum_read_pipe);
	
	
	
	
	
}
void rpconf_wiz_input(char input);
void rpconf_wiz_quit();

