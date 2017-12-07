/*
 * classroom_servant.cpp
 *
 * Created: 2017/11/18 22:41:01
 * Author : sadort
 */ 

#include <avr/io.h>
#include <scheduler.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <usart_ATmega1284.h>
#include <util/delay.h>


unsigned char light = 7;//front, middle, back
unsigned char room = 1;//available
unsigned char rec_light = 7;//front, middle, back, off
unsigned char rec_room = 1;//available
unsigned char rec_tmp = 0;
unsigned char light_tmp = 0;


void showPattern(unsigned char i)
{
	if (i == 1) //tick
	{
		PORTB = 0x10;
		PORTA = 0xEF;
		_delay_ms(2);
		PORTB = 0x28;
		PORTA = 0xF7;
		_delay_ms(2);
		PORTB = 0x44;
		PORTA = 0xFB;
		_delay_ms(2);
		PORTB = 0x02;
		PORTA = 0xFD;
		_delay_ms(2);
		PORTB = 0x01;
		PORTA = 0xFE;
		_delay_ms(2);
	}
	else if (i == 2) //circle
	{
		PORTB = 0x18;
		PORTA = 0xFD;
		_delay_ms(2);
		PORTB = 0x24;
		PORTA = 0xFB;
		_delay_ms(2);
		PORTB = 0x24;
		PORTA = 0xF7;
		_delay_ms(2);
		PORTB = 0x18;
		PORTA = 0xEF;
		_delay_ms(2);
	}
	else if (i == 3) //cross
	{
		PORTB = 0x44;
		PORTA = 0xFE;
		_delay_ms(2);
		PORTB = 0x28;
		PORTA = 0xFD;
		_delay_ms(2);
		PORTB = 0x10;
		PORTA = 0xFB;
		_delay_ms(2);
		PORTB = 0x28;
		PORTA = 0xF7;
		_delay_ms(2);
		PORTB = 0x44;
		PORTA = 0xEF;
		_delay_ms(2);
	}
}

enum USART_States { USART_Init, Receiving };
int TickFct_Usart(int state) {
	switch(state){ //State Actions
		case USART_Init:
			initUSART(0);
			USART_Flush(0);
		break;
		
		case Receiving:
			if(USART_HasReceived(0))
			{
				rec_tmp = USART_Receive(0);
				if ((rec_tmp & 0xF0) == 0x10)
				{
					rec_room = (rec_tmp & 0x0F);
				}
				else if ((rec_tmp & 0xF0) == 0x20)
				{
					rec_light = (rec_tmp & 0x0F);
				}
			}
		break;
		
		default:
		break;
	}
	switch(state){ //Transactions
		case USART_Init:
			state = Receiving;
		break;
		
		case Receiving:
			state = Receiving;
		break;
		
		default:
			state = USART_Init;
		break;
	}
	return state;
};

enum Light_States { Light_Init, Light_Wait };
int TickFct_Light(int state) {
	switch(state){ //State Actions
		case Light_Init:
		
		break;
		
		case Light_Wait:
			if (rec_light != light)
			{
				light = rec_light;
				PORTD &= 0x03;
			}
			if (light == 0)
			{
				PORTD &= 0x03;
				break;
			}
			if ((light & 0x01) == 0x01)
			{
				PORTD |= 0x0c;
			}
			if ((light & 0x02) == 0x02)
			{
				PORTD |= 0x30;
			}
			if ((light & 0x04) == 0x04)
			{
				PORTD |= 0xc0;
			}
			
		break;
		
		default:
		break;
	}
	switch(state){ //Transactions
		case Light_Init:
		state = Light_Wait;
		break;
		
		case Light_Wait:
		state = Light_Wait;
		break;
		
		default:
		state = Light_Init;
		break;
	}
	return state;
};

enum Matrix_States { Matrix_Init, Matrix_Show };
int TickFct_Matrix(int state) {
	switch(state){ //State Actions
		case Matrix_Init:
		
		break;
		
		case Matrix_Show:
			if (rec_room != room)
			{
				room = rec_room;
			}
			showPattern(room);
		break;
		
		default:
		break;
	}
	switch(state){ //Transactions
		case Matrix_Init:
		state = Matrix_Show;
		break;
		
		case Matrix_Show:
		state = Matrix_Show;
		break;
		
		default:
		state = Matrix_Init;
		break;
	}
	return state;
};

int main(void) {
	
	// initialize ports
	DDRD = 0xFE;
	DDRB = 0xFF;
	DDRA = 0xFF;
	DDRC = 0xFF;
	
	tasksNum = 3; // declare number of tasks
	task tsks[3]; // initialize the task array
	tasks = tsks; // set the task array
	
	// define tasks
	unsigned char i=0; // task counter
	tasks[i].state = USART_Init;
	tasks[i].period = 50;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_Usart;
	++i;
	tasks[i].state = Matrix_Init;
	tasks[i].period = 15;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_Matrix;
	++i;
	tasks[i].state = Light_Init;
	tasks[i].period = 30;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_Light;
	
	TimerSet(5); // value set should be GCD of all tasks
	TimerOn();

	while(1) {} // task scheduler will be called by the hardware interrupt
	
}




