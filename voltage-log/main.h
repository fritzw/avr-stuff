#include <avr/io.h>

#define F_CPU 8000000

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
