/*	Author: Andrew Shim
 *  	Partner(s) Name: 
 *	Lab Section: 21
 *	Assignment: Lab # 9 Exercise # 1
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *	Demo: https://drive.google.com/file/d/1LKQq1V9vp3I1erpV3V6-2R_7P0dpLE84/view?usp=sharing
 */
#include <avr/io.h>
//#include "io.h"
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

enum States {Start, INIT, C_NOTE, D_NOTE, E_NOTE} state;

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
    switch(state){ //State Transitions
        case Start:
            state = INIT;
            break;

        case INIT:
	    if ((~PINA & 0x07) == 0x01) {	        
		state = C_NOTE;
	    }
	    else if ((~PINA & 0x07) == 0x02) {
                state = D_NOTE;
	    }
	    else if ((~PINA & 0x07) == 0x04) {
		state = E_NOTE;
	    }
	    else {
		state = INIT;
	    }
            break;

        case C_NOTE:
            if ((~PINA & 0x07) == 0x01) {	        
		state = C_NOTE;
	    } else {
		state = INIT;
	    }
            break;

	case D_NOTE:
            if ((~PINA & 0x07) == 0x02) {	        
		state = D_NOTE;
	    } else {
		state = INIT;
	    }
            break;

	case E_NOTE:
            if ((~PINA & 0x07) == 0x04) {	        
		state = E_NOTE;
	    } else {
		state = INIT;
	    }
            break;

        default:
            state = Start;
            break;
    }

    switch(state){ //State Actions
        case INIT:
	    set_PWM(0);
            break;

        case C_NOTE:
	    set_PWM(261.63);
            break;

        case D_NOTE:
            set_PWM(293.66);
            break;

        case E_NOTE:
	    set_PWM(329.63);
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
