#ifndef _RP_VOIP_H_
#define _RP_VOIP_H_

extern void rpvoip_init();
extern void rpvoip_call(char* number);
extern void rpvoip_dial(char number);
extern void rpvoip_answer();
extern void rpvoip_terminate();
extern void rpvoip_quit();

#endif