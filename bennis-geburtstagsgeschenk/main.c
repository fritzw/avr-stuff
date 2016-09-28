#define F_CPU 16000000

#include <avr/io.h>
#include <util/delay.h>

#include <stdlib.h>
#include <string.h>

char string[] = "\0Fragt imp";
int idx = 0;
uint8_t i;

uint8_t button_old;

void init(void) {
	button_old = PINA & 1;
	DDRC = 0xFF;
	return;
}

void loop(void) {
	uint8_t button = PINA & 1;
	if (button != button_old && button == 1) {
		idx++;
		if (idx > strlen(string+1)) {
			idx = 0;
			PORTC = 0;

			_delay_ms(200);
			PORTC = ~PORTC;
			_delay_ms(50);
			PORTC = ~PORTC;

			_delay_ms(200);
			PORTC = ~PORTC;
			_delay_ms(50);
			PORTC = ~PORTC;

			_delay_ms(200);
			PORTC = ~PORTC;
			_delay_ms(50);
			PORTC = ~PORTC;
		}
		PORTC = string[idx];
		_delay_ms(300);
	}
	button_old = button;
	return;
}

int main(void) {
	init();
	while (1) loop();
}
