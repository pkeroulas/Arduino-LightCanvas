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

// avr
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
// local
#include "debug.h"
// #include "content.h"

/* set the UART baud rate */
/* 20060803: hacked by DojoCorp */

#define BAUD_RATE   57600


#define MAX_TIME_COUNT (F_CPU>>4)

/* Adjust to suit whatever pin your hardware uses to enter the bootloader */
/* ATmega128 has two UARTS so two pins are used to enter bootloader and select UART */
/* ATmega1280 has four UARTS, but for Arduino Mega, we will only use RXD0 to get code */
/* BL0... means UART0, BL1... means UART1 */
#ifdef __AVR_ATmega128__
#define BL_DDR  DDRF
#define BL_PORT PORTF
#define BL_PIN  PINF
#define BL0     PINF7
#define BL1     PINF6
#elif defined __AVR_ATmega1280__
/* we just don't do anything for the MEGA and enter bootloader on reset anyway*/
#else
/* other ATmegas have only one UART, so only one pin is defined to enter bootloader */
#define BL_DDR  DDRD
#define BL_PORT PORTD
#define BL_PIN  PIND
#define BL      PIND6
#endif

#define TBUFSIZE	(uint16_t)512
#define TMASK   	(uint16_t)(TBUFSIZE-1)
volatile uint8_t tbuf[TBUFSIZE];
volatile uint16_t t_in;
volatile uint16_t t_out;
#define  TBUFLEN()  (t_in - t_out)


void debug_init(void){
    /* We run the bootloader regardless of the state of this pin.  Thus, don't
       put it in a different state than the other pins.  --DAM, 070709
       This also applies to Arduino Mega -- DC, 080930
       BL_DDR &= ~_BV(BL);
       BL_PORT |= _BV(BL);
       */
    /* initialize UART(s) depending on CPU defined */

    UBRR0L = (uint8_t)(F_CPU/(BAUD_RATE*16L)-1);
    UBRR0H = (F_CPU/(BAUD_RATE*16L)-1) >> 8;
    UCSR0B = (1<<RXCIE0) | (1<<RXEN0) | (1<<TXEN0);
    UCSR0C = (1<<UCSZ00) | (1<<UCSZ01);

    debug_char = 0xFF;

    /* Enable internal pull-up resistor on pin D0 (RX), in order
       to supress line noise that prevents the bootloader from
       timing out (DAM: 20070509) */
    DDRD &= ~_BV(PIND0);
    PORTD |= _BV(PIND0);
}

/*---------------------------------------------------------------------------------*/
// TX

int debug_putc(char c) {
    uint16_t timeout = 0;

    // Fills the transmit buffer, if it is full wait
    while((TBUFSIZE - TBUFLEN()) <= 2){
        if(timeout++>0xFFF0)
            return -1;
        _delay_us(10);
    }

    // Add data to the transmit buffer, enable TXCIE
    tbuf[t_in & TMASK] = c;
    t_in++;

    // Enable UDR empty interrupt
    UCSR0B |= (1<<UDRIE0);

    return 0;
}


// transmit ready interrupt
// USART Data Register Empty
ISR(USART_UDRE_vect) {
    if(t_in != t_out) {
        UDR0 = tbuf[t_out & TMASK];
        t_out++;
    }
    else {
        UCSR0B &= ~(1<<UDRIE0);
    }
}

/*---------------------------------------------------------------------------------*/
// RX
char getch(void)
{
    uint32_t count = 0;
    while(!(UCSR0A & _BV(RXC0))){
        count++;
        if (count > MAX_TIME_COUNT)
            return 0;
    }
    return UDR0;
}

ISR(USART_RX_vect ){
    debug_char = getch();
    debug_char -= 48;
}
