/*
 *  Copyright (C) 2014 Patrick Keroulas.
 *  Author: Patrick Keroulas <patkarbo@patkarbo.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

//avr
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
//local
#include "display.h"
#include "debug.h"


#define  sbi(addr,bm) addr |= (1<<bm)
#define  cbi(addr,bm) addr &= ~(1<<bm)


//////////////////////////////////////////////////////////////////////////

// TLC driver
// port B
//#define XERR	2		// Error output
// port A
#define MODE	0		// Dot Correction (MODE = high) or Grayscale (MODE = low)
#define XLAT	3		// Edge triggered latch signal. At the rising edge of XLAT, the TLC59461 writes data from the input shift register to the DC or the GS register
#define BLANK	4		// When BLANK is high, all constant-current outputs are forced off, the Grayscale PWM timing controller initializes and the Grayscale counter resets to '0'.
//#define XHALF	2		// Extend serial interface
// #define  	5		// Reference clock for Grayscale PWM control.
#define SCLK	2		// Reference clock for DC
#define STX		1		// Serial data to the driver 1

//#define DRIVER_PORT PORTA
//#define PWM_PIN		2
#define DRIVER_PORT PORTC // Arduino

#define ON						4095
#define OFF						0
#define DELAY_STX				2

/*--------------------------------------------------------------------------------*/

uint16_t scale[RANGE] = {1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,3,3,3,3,3,4,4,4,5,5,5,6,6,7,7,7,8,9,9,10,11,11,12,13,14,15,16,17,18,19,21,22,24,25,27,29,31,33,35,38,40,43,46,49,52,56,59,63,68,72,77,82,88,94,100,107,114,122,130,139,148,158,169,181,193,206,219,234,250,267,285,304,324,346,369,394,421,449,479,511,546,583,622,663,708,756,806,861,918,980,1046,1116,1191,1271,1357,1448,1545,1649,1759,1878,2004,2138,2282,2435,2599,2773,2959,3158,3370,3596,3838};

void display_latch(void);
uint16_t display_linearize(uint8_t x);
void pwm_init(void);
void pwm_start(void);
void pwm_stop(void);

/*--------------------------------------------------------------------------------*/

void display_init(void){
    //attiny44
    //DDRA |= (1<<MODE) | (1<<BLANK) | (1<<XLAT) | (1<<MODE)  | (1<<STX) | (1<<SCLK);
    //DDRB |= (1<<PWM_PIN);
    // Arduino
    DDRC |= (1<<MODE) | (1<<BLANK) | (1<<XLAT) | (1<<MODE)  | (1<<STX) | (1<<SCLK);	
    DRIVER_PORT = 0;	// latch low and mode=greyscale
    DRIVER_PORT |= (1<<BLANK);

    pwm_init();
}

void display_start(void){
    pwm_start();
}

/*--------------------------------------------------------------------------------*/
// serial data functions

void display_run(uint8_t * data, uint8_t length){
    uint16_t value;
    int16_t j,k;

    cbi(DRIVER_PORT,MODE);		// greyscale mode

    for(j=length-1 ; j>=0 ; j--){		// for each pixel
        data[j] = (data[j] > RANGE-1)? RANGE-1 : data[j];	//clip
        value = scale[data[j]];
        //value = display_linearize(data[j]);

        for(k=0 ; k<12 ; k++){	// for each bit			
            cbi(DRIVER_PORT,SCLK); _delay_us(DELAY_STX);		// falling edge of the clock
            if(value & (1 << (11-k))) {sbi(DRIVER_PORT,STX);}  // MSB first						
            else 	cbi(DRIVER_PORT,STX);
            _delay_us(DELAY_STX);
            sbi(DRIVER_PORT,SCLK); _delay_us(DELAY_STX);			// rising edge of the clock	
        }			
    }
    cbi(DRIVER_PORT,STX);			// keep the pins low
    cbi(DRIVER_PORT,SCLK);

    display_latch();
}


void display_latch(){
    // latch, data from IN regiter to GS register/ DC register
    sbi(DRIVER_PORT,XLAT); _delay_us(DELAY_STX);	cbi(DRIVER_PORT,XLAT);
}

uint16_t display_linearize(uint8_t x) {
    uint16_t y = ((uint8_t)x)<<4;
    if (x < 10) return 0;
    if (x < 20) return 16;
    if (x < 50) return (y - 320)/5 + 32;
    if (x < 82) return (y - 800)/4 + 128;
    if (x < 94) return (y - 1312)/3 + 256;
    if (x < 174) return (y - 1504)/2 + 320;
    if (x < 199) return (y - 2784) + 960;
    if (x < 239) return (y - 3184)*2 + 1360;
    if (x < 243) return (y - 3824)*3 + 2640;
    if (x < 250) return (y - 3888)*5 + 2832;
    if (x < 255) return (y - 4000)*8 + 3392;
    return 4095;

    /*if (x < 10) return 0;
      if (x < 20) return 1;
      if (x < 50) return (x - 20)/5 + 2;
      if (x < 82) return (x - 50)/4 + 8;
      if (x < 94) return (x - 82)/3 + 16;
      if (x < 174) return (x - 94)/2 + 20;
      if (x < 199) return (x - 174) + 60;
      if (x < 239) return (x - 199)*2 + 85;
      if (x < 243) return (x - 239)*3 + 165;
      if (x < 250) return (x - 243)*5 + 177;
      if (x < 255) return (x - 250)*8 + 212;
      return 255;*/
}

/*--------------------------------------------------------------------------------*/
// PWM FUNCTIONS
volatile uint8_t pwm_pulse_counter;

#define PWM_PIN		1
#define PWM_PERIOD	4	// 1_prescaler * PWM_PERIOD = PWM_PERIOD // 2 not safe
#define PWM_PACKET	64 	// 256_prescaler * PWM_PACKET = 4096 * PWM_PERIOD // 32 not safe

void pwm_init(void){
    DDRB |= (1<<PWM_PIN);
    OCR1AH = 0; OCR1AL = PWM_PERIOD>>1;
    ICR1H = 0; ICR1L = PWM_PERIOD;
    TCCR1A = 0b10000010;	// Clear OC0A on compare match
    TCCR1B = 0b00011000;	// Fast PWM mode with top=ICR1

    OCR0A = PWM_PACKET;
    sbi(TIMSK0,OCIE0A);		// enable timer0 interrupt on compare match with OCR0A
}

void pwm_start(void){
    TCCR1B |= 1;			// start the timer1 to generate GSCLK. no prescaler
    TCCR0B |= 0b00000100;	// prescaler = 256

    cbi(DRIVER_PORT,BLANK);	// restart GSCLK counter
}

void pwm_stop(void){
    TCCR1B &= ~1;			// stop the timer1
    TCCR0B &= ~0b00000111;	// stop
    TIMSK0 = 0;			// disable timer0 overflow interrupt	
    sbi(DRIVER_PORT,BLANK);	// reset and stop the greyscale counter on the LED Driver
}

// arduino
ISR(TIMER0_COMPA_vect) {	// every PWM_PACKET ovf of timer0
    TIFR0 = 1;
    TCNT0 = 0;
    pwm_packet_counter++;

    sbi(DRIVER_PORT,BLANK);
    _delay_us(1);
    cbi(DRIVER_PORT,BLANK);	// restart GSCLK counter
}

/*
#define PWM_PIN		1
#define PWM_PERIOD	0x80 // of 0xFF because 50% duty cycle
#define PWM_PACKET	0x3FF

void pwm_init(void){
DDRD |= (1<<PWM_PIN);
OCR0A = PWM_PERIOD;
TCCR0A = 0b10000011;	// Clear OC0A on compare match, Fast PWM mode
TCCR0B = 0;

OCR1AH = (uint8_t)((PWM_PACKET>>8)&0xFF);
OCR1AL = (uint8_t)(PWM_PACKET&0xFF);
sbi(TIMSK1,OCIE1A);		// enable timer0 interrupt on compare match with OCR0A
}

void pwm_start(void){
TCCR0B |= 1;			// start the timer0 to generate GSCLK. no prescaler
TCCR1B |= 0b00000100;	// prescaler = 256

cbi(DRIVER_PORT,BLANK);	// restart GSCLK counter
}

void pwm_stop(void){
TCCR0B &= ~1;			// stop the timer0
TCCR1B &= ~0b00000111;	// stop
TIMSK1 = 0;			// disable timer0 overflow interrupt	
sbi(DRIVER_PORT,BLANK);	// reset and stop the greyscale counter on the LED Driver
}

// arduino
ISR(TIMER1_COMPA_vect) {	// every PWM_PACKET ovf of timer0
TIFR1 = 1;
TCNT1H = TCNT1L = 0;
pwm_packet_counter++;

sbi(DRIVER_PORT,BLANK);
_delay_us(1);
cbi(DRIVER_PORT,BLANK);	// restart GSCLK counter
}
*/

