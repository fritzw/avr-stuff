;**** A P P L I C A T I O N   N O T E   A V R 3 0 5 ************************
;*
;* Title		: Half Duplex Interrupt Driven Software UART
;* Version		: rev. 1.2 (24-04-2002), reset vector added
;*			: rev. 1.1 (27.08.1997)
;* Last updated		: 24-04-2002
;* Target		: AT90Sxxxx (All AVR Device)
;*
;* Support email	: avr@atmel.com
;*
;* Code Size		: 32 Words
;* Low Register Usage	: 0
;* High Register Usage	: 4
;* Interrupt Usage	: None
;*
;* DESCRIPTION
;* This Application note contains a very code efficient software UART.
;* The example program receives one character and echoes it back.
;***************************************************************************

#include <avr/io.h>

;***** Pin definitions

.equ	PORT,	_SFR_IO_ADDR(PORTB)
.equ	DDR,	_SFR_IO_ADDR(DDRB)
.equ	PIN,	_SFR_IO_ADDR(PINB)
.equ	RxD,	1			;Receive pin is PD1
.equ	TxD,	0			;Transmit pin is PD0

;***** Global register variables

.equ	bitcnt,	18			;bit counter
.equ	temp,	19			;temporary storage register

.equ	_Rxbyte,	21			;Received data
.equ	Txbyte,	24			;Data to be transmitted, argument

;***** Exported symbols

.global SUART_send



;.cseg
;.org 0
;	rjmp	reset
;***************************************************************************
;*
;* "SUART_send"
;*
;* This subroutine transmits the byte stored in the "Txbyte" register
;* The number of stop bits used is set with the sb constant
;*
;* Number of words	:14 including return
;* Number of cycles	:Depens on bit rate
;* Low registers used	:None
;* High registers used	:2 (bitcnt,Txbyte)
;* Pointers used	:None
;*
;***************************************************************************

SUART_send:	ldi	bitcnt,10	;1 start bit, 8 data bits, 1 stop bit
		com	Txbyte		;Inverte everything
		sec			;Start bit

SUART_send0:	brcc	SUART_send1	;If carry set
		cbi	PORT,TxD	;    send a '0'
		rjmp	SUART_send2	;else	

SUART_send1:	sbi	PORT,TxD	;    send a '1'
		nop

SUART_send2:	rcall UART_delay	;One bit delay
		rcall UART_delay

		lsr	Txbyte		;Get next bit
		dec	bitcnt		;If not all bit sent
		brne	SUART_send0	;   send next
					;else
		ret			;   return


;***************************************************************************
;*
;* "SUART_recv"
;*
;* This subroutine receives one byte and returns it in the "Rxbyte" register
;*
;* Number of words	:14 including return
;* Number of cycles	:Depens on when data arrives
;* Low registers used	:None
;* High registers used	:2 (bitcnt,Rxbyte)
;* Pointers used	:None
;*
;***************************************************************************

SUART_recv:	ldi 	bitcnt,9	;8 data bit + 1 stop bit

SUART_recv1:	sbic 	PIN,RxD	;Wait for start bit
		rjmp 	SUART_recv1

		rcall UART_delay	;0.5 bit delay

SUART_recv2:	rcall UART_delay	;1 bit delay
		rcall UART_delay		

		clc			;clear carry
		sbic 	PIN,RxD	;if RX pin high
		sec			;

		dec 	bitcnt		;If bit is stop bit
		breq 	SUART_recv3	;   return
					;else
		ror 	_Rxbyte		;   shift bit into Rxbyte
		rjmp 	SUART_recv2	;   go get next

SUART_recv3:	ret


;***************************************************************************
;*
;* "UART_delay"
;*
;* This delay subroutine generates the required delay between the bits when
;* transmitting and receiving bytes. The total execution time is set by the
;* constant "b":
;*
;*	3·b + 7 cycles (including rcall and ret)
;*
;* Number of words	:4 including return
;* Low registers used	:None
;* High registers used	:1 (temp)
;* Pointers used	:None
;*
;***************************************************************************
; Some b values: 	(See also table in Appnote documentation)
;
; 1 MHz crystal:
;   9600 bps - b=14
;  19200 bps - b=5
;  28800 bps - b=2
;
; 2 MHz crystal:
;  19200 bps - b=14
;  28800 bps - b=8
;  57600 bps - b=2

; 4 MHz crystal:
;  19200 bps - b=31
;  28800 bps - b=19
;  57600 bps - b=8
; 115200 bps - b=2

;.equ	b,	31	;19200 bps @ 4 MHz crystal
;.equ 	b,	8	;115200 bps @ 8 MHz
.equ 	b,	135	;9600 bps @ 8 MHz
;.equ 	b,	255	;Very high value for debugging


UART_delay:	ldi	temp,b
UART_delay1:	dec	temp
		brne	UART_delay1

		ret

;***** Program Execution Starts Here

;***** Test program

;reset:		sbi	PORT,TxD	;Init port pins
;		sbi	DDR,TxD
;
;		ldi	Txbyte,12	;Clear terminal
;		rcall	SUART_send
;
;forever:	rcall	SUART_recv
;		mov	Txbyte,Rxbyte
;		rcall	SUART_send		;Echo received char
;		rjmp	forever

