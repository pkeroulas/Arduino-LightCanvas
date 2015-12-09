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

/*--------------------------------------------------------
  CHECK LIST:

  Before flashing a pixel, check the following items and set
  them depending on the project/board:

  --------------------------------------------------------*/

// avr
#include <Arduino.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// local
#include "generator.h"
#include "sequence.h"
#include "display.h"
#include "debug.h"

#define BUTTON_PWR_PIN 3
#define BUTTON_PIN 7

int main (void){
    cli();

    // debug
    DDRB |= (1<<DEBUG_LED_PIN);
    debug_init();
    // push button
    DDRD |= (1<<BUTTON_PWR_PIN); PORTD |= (1<<BUTTON_PWR_PIN);
    DDRD &= ~(1<<BUTTON_PIN); PORTD |= (1<<BUTTON_PIN);
    uint8_t button_press_counter;

    // init
    sequence_init();
    generator_init();
    sequence_update(0);
    sequence_runmode = SEQ_RUNMODE_SEQUENTIAL;
    generator_update();
    uint16_t frame_counter;
    uint8_t seed=0;
    _delay_ms(10);

    // Global interrupt enable.
    sei();

    while(1)
    {
        srand(seed++);

        // the refresh rate depends on the driver rate
        if(pwm_packet_counter>20){
            pwm_packet_counter = 0;
            generator_run();
            DEBUG_LED_PIN_TOGGLE();

            if((frame_counter++ > 0x3FF)&&(sequence_runmode==SEQ_RUNMODE_TOGGLE)){
                sequence_skip();
                generator_update();
                frame_counter = 0;
            }
        }
        // serial cmd received
        if(debug_char != 0xFF){
            sequence_update(debug_char);
            generator_update();
            debug_char = 0xFF;
        }
        // push button
        if((PIND&(1<<BUTTON_PIN)) == 0){
            button_press_counter = 0;
            // measure the button press duration
            while((PIND&(1<<BUTTON_PIN)) == 0){
                _delay_ms(100);
                button_press_counter++;
            }
            // short press => next sequence
            if (button_press_counter < 10){
                frame_counter = 0;
                sequence_skip();
                generator_update();
                // debug_putc('d'); // skip sequence
            }
            // long press => switch between normal and sequential mode
            else{
                frame_counter = 0;
                sequence_runmode ^= SEQ_RUNMODE_TOGGLE;
                // debug_putc('c'); // switch normal(current sequence)<->sequential
            }
        }
    }
}
