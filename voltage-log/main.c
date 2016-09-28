#define F_CPU 8000000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>
#include <util/delay.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>


#define LEDPIN 4
#define ADCPIN 3
#define PWMPIN 1

#define SUART_PORT _SFR_IO_ADDR(PORTB)
#define SUART_DDR _SFR_IO_ADDR(DDRB)
#define SUART_PIN _SFR_IO_ADDR(PINB)
#define SUART_TXD 2
#define SUART_RXD 0

// Helper macros turn argument into string literal
#define STRING(arg) STRING_(arg)
#define STRING_(arg) #arg

#define ACOMP_VALUE r16
#define ACOMP_FLAG r17

#ifndef __ASSEMBLER__
register uint8_t acomp_value asm(STRING(ACOMP_VALUE));
register uint8_t acomp_flag asm(STRING(ACOMP_FLAG));
#endif

uint8_t adc_value = 0;

uint8_t eeprom_address = 0;
uint8_t adc_values[256];
uint8_t eeprom_values[256] EEMEM;
volatile uint32_t overflows = 0;
uint32_t seconds = 0;
char buffer[15];

extern void SUART_send(uint8_t byte); // From SUART.S

void SUART_send_string(char *str, uint8_t maxlen) {
	uint8_t i;
	for (i = 0; (str[i] != '\0') && (i < maxlen); i++) {
		SUART_send(str[i]);
	}
}

ISR(ANA_COMP_vect, ISR_NAKED) {
	//acomp_value = TCNT1;
	asm volatile ("in "STRING(ACOMP_VALUE)", 0x2F" ::);
	acomp_flag = 1;
	
	reti();
}

ISR(TIMER1_OVF_vect, ISR_NOBLOCK) {
	overflows++;
}

/*int send_timestamp() {
	itoa(overflows, buffer, 10);
	uint8_t len = strlen(buffer);
	buffer[len] = ':';
	itoa(acomp_value, buffer + len + 1, 10);
	SUART_send_string(buffer, sizeof buffer);
	SUART_send(',');
}*/

void init(void) {
	// Configure Timer0 in PWM Mode to generate HF signal
	//TCCR0A |= (1 << COM0B1); // Non-inverting PWM on pin OC0B (PB1)
	//TCCR0A |= (1 << WGM00) | (1 << WGM01);
	//TCCR0B |= (1 << WGM02);  // Fast PWM Mode, counting to OCR0A
	//OCR0A = 7;
	//OCR0B = 1; // Set OC0B in TCNT0=0, clear OC0B on 1<=TCNT0<=7
	//TCNT0 = 0;
	//TCCR0B |= (1 << CS00);   // Use IO clock without prescaler
	
	// Configure Timer1 to measure time
	TCCR1 |= (1 << CS10);     // No prescaler
	TIMSK |= (1 << TOIE1);    // Enable overflow interrupt

	// Configure Timer1 in PWM mode to drive LED
	//GTCCR |= (1 << PWM1B);  // Use PWM mode for OC1B (PB4)
	//GTCCR |= (1 << COM1B1); // Only drive the non-inverted pin
	//OCR1C = 255;            // Maximum value of Timer1
	//TCCR1 |= (1 << CS10);   // Use IO clock without prescaler
	
	//DDRB |= (1 << PWMPIN);
	//DDRB |= (1 << LEDPIN);
	DDRB |= (1 << SUART_TXD);
	//DDRB |= (1 << ADCPIN);

	// Configure ADC
	//ADMUX |= (1 << ADLAR);  // Left adjust ADC result 
	ADMUX |= (1 << MUX1);   // Select ADC2 input (PB4)
	//ADMUX |= (1 << MUX1) | (1 << MUX0);   // Select ADC3 input (PB3)
	DIDR0 |= (1 << ADC2D);  // Disable digital input on pin ADC2 (PB4)
	DIDR0 |= (1 << ADC3D);  // Disable digital input on pin ADC3 (PB3)
	//ADCSRA |= (1 << ADIE);  // Enable ADC conversion complete interrupt
	//ADCSRA |= (1 << ADIF);   // Clear ADC interrupt flag
	//ADCSRA |= (1 << ADATE); // Enable auto trigger (in free running mode)
	ADCSRA |= (1 << ADPS1) | (1 << ADPS0); // Select prescaler 8
	//ADCSRA |= (1 << ADPS2) | (1 << ADPS1); // Select prescaler 64
	ADCSRA |= (1 << ADEN);  // Enable ADC

	//PORTB |= (1 << LEDPIN);

	// Pull pin to high, then start conversion.
	//PORTB |= (1 << ADCPIN);
	_delay_ms(1000);
	//PORTB &= ~(1 << LEDPIN);
	//sei();
}

int32_t adc_measure(uint8_t adc_pin) {
        ADMUX &= ~(1 << MUX0 | 1 << MUX1 | 1 << MUX2 | 1 << MUX3);
        if (adc_pin == 2) {
                ADMUX |= (1 << MUX1);   // Select ADC2 input (PB4)
        } else if (adc_pin == 3) {
                ADMUX |= (1 << MUX1) | (1 << MUX0);   // Select ADC3 input (PB3)
        }

	int32_t value = 0;
	uint8_t i;
	const uint8_t count_exp = 4;
	for (i = 0; i < (1 << count_exp); i++) {
		//set_sleep_mode(SLEEP_MODE_IDLE);
		//sei();
		//sleep_mode(); // Start conversion
                ADCSRA |= (1 << ADSC);
		while (ADCSRA & (1 << ADSC));
		value += ADC;
	}
	return value >> count_exp;
}

void loop(void) {
	char buffer[100];
	//sprintf(buffer,  "OF: %ld\n", overflows);
	//SUART_send_string(buffer, sizeof buffer);

        if (overflows < 31250) return;
        seconds += 1;
        overflows = 0;
        if (seconds % 2 != 1) return;

        int32_t value = adc_measure(2);
        //int32_t volt = (value * 5) / 1024;
        //int32_t volt_comma = (value * 500) / 1024 - volt * 100;

        int32_t value2 = adc_measure(3);
        //int32_t volt2 = (value2 * 10) / 1024;
        //int32_t volt2_comma = (value2 * 1000) / 1024 - volt2 * 100;
	//sprintf(buffer,  "%ld.%02ld %ld.%02ld\n", volt, volt_comma, volt2, volt2_comma);
	sprintf(buffer,  "%ld %ld\n", value, value2);
	SUART_send_string(buffer, sizeof buffer);

	//_delay_ms(500);

//	PORTB ^= (1 << LEDPIN);
}


int main(void) {
    char buffer[] = "Hello world\n";
	init();
        SUART_send_string(buffer, sizeof buffer);
	while (1) loop();
}

