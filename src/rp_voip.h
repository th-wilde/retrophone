#ifndef _RP_VOIP_H_
#define _RP_VOIP_H_
#include <stdbool.h>

enum rpvoip_state_enum {NORMAL, RING, CALL};

extern void rpvoip_init();
extern void rpvoip_dial(char* number);
extern enum rpvoip_state_enum rpvoip_update();
extern void rpvoip_answer();
extern void rpvoip_terminate();
extern void rpvoip_quit();

#endif