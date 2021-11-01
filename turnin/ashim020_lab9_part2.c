/*	Author: Andrew Shim
 *  	Partner(s) Name: 
 *	Lab Section: 21
 *	Assignment: Lab # 9 Exercise # 2
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *	Demo: https://drive.google.com/file/d/1sDNsRNQ3sLY3tEOEaiR3AYK2oXydl7-W/view?usp=sharing
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

enum States {Start, INIT, INC, DEC, RELEASE, POWER} state;

double array[8] = {261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88, 523.25};
unsigned char alt = 0x00;
unsigned char ind = 0x00;

volatile unsigned char TimerFlag = 0;

unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn() {
	TCCR1B = 0x0B;
	OCR1A = 125;
	TIMSK1 = 0x02;
	TCNT1 = 0;
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;
}

void TimerOff() {
	TCCR1B = 0x00;
}

void TimerISR(){
	TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect) {
	_avr_timer_cntcurr--;
	if (_avr_timer_cntcurr == 0) {
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

void TimerSet (unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

void set_PWM(double frequency) {
	static double current_frequency;
	if (frequency != current_frequency) {
		if (!frequency) { TCCR3B &= 0x08; }
		else { TCCR3B |= 0x03; }
		if (frequency < 0.954) { OCR3A = 0xFFFF; }
		else if (frequency > 31250) { OCR3A = 0x0000; }
		else { OCR3A = (short) (8000000 / (128 * frequency)) -1; }
		TCNT3 = 0;
		current_frequency = frequency;
	}
}

void PWM_on() {
	TCCR3A = (1 << COM3A0);
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	set_PWM(0);
}

void PWM_off() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}

void Tick(){
    switch(state) { //State Transitions
        case Start:
            state = INIT;
            break;

        case INIT:
	    if ((~PINA & 0x07) == 0x01) {	        
		state = INC;
	    }
	    else if ((~PINA & 0x07) == 0x02) {
                state = DEC;
	    }
	    else if ((~PINA & 0x07) == 0x04) {
		state = POWER;
	    }
	    else {
		state = INIT;
	    }
            break;

        case INC:
            state = RELEASE;
            break;

	case DEC:
            state = RELEASE;
            break;

	case RELEASE:
            if ((~PINA & 0x07) == 0x00) {	        
		state = INIT;
	    } else {
		state = RELEASE;
	    }
            break;
	
	case POWER:
	    state = RELEASE;
	    break;

        default:
            state = Start;
            break;
    }

    switch(state) { //State Actions
        case INIT:
            break;

        case INC:
	    if (ind < 0x07) {
		ind++;
	    }
	    if (alt == 0x01) {
		set_PWM(array[ind]);
	    }
            break;

        case DEC:
            if (ind > 0x00) {
		ind--;
	    }
	    if (alt == 0x01) {
		set_PWM(array[ind]);
	    }
            break;

        case RELEASE:
            break;
	
	case POWER:
	    if (alt == 0x00) {
		alt = 0x01;
		set_PWM(array[ind]);
	    } else {
		alt = 0x00;
		set_PWM(0);
	    }
	    break;

        default:
            break;
    }
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0x00; PORTA = 0xFF;
    DDRB = 0xFF; PORTB = 0x00;  

    TimerSet(100);
    TimerOn();
    PWM_on();

    /* Insert your solution below */
    while (1) {
        Tick();
        while(!TimerFlag) {};
        TimerFlag = 0;
    }
}
