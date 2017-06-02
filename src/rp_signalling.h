#ifndef _RP_SIGNALLING_H_
#define _RP_SIGNALLING_H_

extern void rps_init(int gpio_port, int output_pipe);
extern void rps_ring(int ring_tone);
extern void rps_quit();

#endif
