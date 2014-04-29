#ifndef __DEBUG_H__
#define __DEBUG_H__

// arduino
#define DEBUG_LED_PIN 5
#define DEBUG_LED_PIN_LOW() PORTB |= (1<<DEBUG_LED_PIN)
#define DEBUG_LED_PIN_HIGH() PORTB &= ~(1<<DEBUG_LED_PIN)
#define DEBUG_LED_PIN_TOGGLE() PORTB ^= (1<<DEBUG_LED_PIN)

/*
// attiny44
#define DEBUG_LED_PIN PA5
#define DEBUG_LED_PIN_LOW() PORTA |= (1<<DEBUG_LED_PIN)
#define DEBUG_LED_PIN_HIGH() PORTA &= ~(1<<DEBUG_LED_PIN)
#define DEBUG_LED_PIN_TOGGLE() PORTA ^= (1<<DEBUG_LED_PIN)
*/

volatile uint8_t debug_char;

void debug_init(void);
int debug_putc(char c);
char getch(void);

#endif // __DEBUG_H__