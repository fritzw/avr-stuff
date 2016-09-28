#define F_CPU 1000000
#define EEPROM_SIZE 512

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>
#include <util/delay.h>

#define LEDPIN 3
#define ADCPIN 4

uint8_t adc_value = 0;

uint8_t eeprom_address = 0;
uint8_t adc_values[256];

ISR(ADC_vect) {
}


void init(void) {
	DDRB |= 1 << LEDPIN;
	DDRB |= 1 << ADCPIN;
	ADMUX |= (1 << ADLAR);  // Left adjust ADC result 
	ADMUX |= (1 << MUX1);   // Select ADC2 input (PB4)
	DIDR0 |= (1 << ADC2D);  // Disable digital input on pin ADC2 (PB4)
	ADCSRA |= (1 << ADIE);  // Enable ADC conversion complete interrupt
	ADCSRA |= (1 << ADIF);   // Clear ADC interrupt flag
//	ADCSRA |= (1 << ADATE); // Enable auto trigger (in free running mode)
	ADCSRA |= (1 << ADPS1) | (1 << ADPS0); // Select prescaler 8
	ADCSRA |= (1 << ADEN);  // Enable ADC

	
	PORTB |= (1 << LEDPIN);

	// Pull pin to high, then start conversion.
	PORTB |= (1 << ADCPIN);
	_delay_ms(1000);
	PORTB &= ~(1 << LEDPIN);
	//sei();

	uint8_t i = 0;
	do {
		set_sleep_mode(SLEEP_MODE_ADC);
		sei();
		sleep_mode();
		//ADCSRA |= (1 << ADSC);  // Start conversion
		while (ADCSRA & (1 << ADSC));

		DDRB &= ~(1 << ADCPIN);
		PORTB &= ~(1 << ADCPIN);

		adc_values[i] = ADCH;
	} while (++i != 0);
	
	PORTB |= (1 << LEDPIN);
	ADCSRA &= ~(1 << ADEN);

	do {
		while (EECR & (1 << EEPE));
		eeprom_write_byte((uint8_t *) i, adc_values[i]);
	} while (++i != 0);

	PORTB &= ~(1 << LEDPIN);
}

void loop(void) {
}


int main(void) {
	init();
	while (1) loop();
}
