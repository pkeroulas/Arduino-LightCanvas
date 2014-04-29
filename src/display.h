#ifndef _LEDDRIVER_H
#define _LEDDRIVER_H

#define RANGE	128

void display_start(void);
void display_init(void);
void display_run(uint8_t * data, uint8_t length);

volatile uint8_t pwm_packet_counter;

#endif
