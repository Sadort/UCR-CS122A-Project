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

#define FULL_ROTATE 512

unsigned char projector = 0;
unsigned char rec_projector = 0;
unsigned char rec_tmp = 0;
unsigned char st_num = 0;
unsigned char steppermove[8] = { 0x01, 0x03, 0x02, 0x06, 0x04, 0x0c, 0x08, 0x09 };
int stepcnt = 0;
int step = 0;
float adc_result;

/*ADC*/
void InitADC()
{
	ADMUX=(1<<REFS0);                         // For Aref=AVcc;
	ADCSRA=(1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0); //Rrescalar div factor =128
}

uint16_t ReadADC(uint8_t ch)
{
	//Select ADC Channel ch must be 0-7
	ch=ch&0b00000111;
	ADMUX|=ch;

	//Start Single conversion
	ADCSRA|=(1<<ADSC);

	//Wait for conversion to complete
	while(!(ADCSRA & (1<<ADIF)));

	//Clear ADIF by writing one to it
	//Note you may be wondering why we have write one to clear it
	//This is standard way of clearing bits in io as said in datasheets.
	//The code writes '1' but it result in setting bit to '0' !!!

	ADCSRA|=(1<<ADIF);

	return(ADC);
}
/*ADC*/

/*temperature sensor*/
#define DHT11_PIN 0
uint8_t c=0,I_RH,D_RH,I_Temp,D_Temp,CheckSum;

void Request()				/* Microcontroller send start pulse/request */
{
	DDRB |= (1<<DHT11_PIN);
	PORTB &= ~(1<<DHT11_PIN);	/* set to low pin */
	_delay_ms(20);			/* wait for 20ms */
	PORTB |= (1<<DHT11_PIN);	/* set to high pin */
}

void Response()				/* receive response from DHT11 */
{
	DDRB &= ~(1<<DHT11_PIN);
	while(PINB & (1<<DHT11_PIN));
	while((PINB & (1<<DHT11_PIN))==0);
	while(PINB & (1<<DHT11_PIN));
}

uint8_t Receive_data()			/* receive data */
{
	for (int q=0; q<8; q++)
	{
		while((PINB & (1<<DHT11_PIN)) == 0);  /* check received bit 0 or 1 */
		_delay_us(30);
		if(PINB & (1<<DHT11_PIN))/* if high pulse is greater than 30ms */
		c = (c<<1)|(0x01);	/* then its logic HIGH */
		else			/* otherwise its logic LOW */
		c = (c<<1);
		while(PINB & (1<<DHT11_PIN));
	}
	return c;
}
/*temperature sensor*/

enum BlueTooth_States { BlueTooth_Init, Receiving, Sending };
int TickFct_BlueTooth(int state) {
	switch(state){ //State Actions
		case BlueTooth_Init:
			initUSART(0);
			USART_Flush(0);
			initUSART(1);
			USART_Flush(1);
		break;
		
		case Receiving:
			if(USART_HasReceived(0))
			{
				rec_tmp = USART_Receive(0);
				if ((rec_tmp & 0xF0) == 0x10)
				{
					if(USART_IsSendReady(1))
					{
						USART_Send(rec_tmp,1);
						_delay_ms(1);
					}
				}
				else if ((rec_tmp & 0xF0) == 0x20)
				{
					if(USART_IsSendReady(1))
					{
						USART_Send(rec_tmp,1);
						_delay_ms(1);
					}
				}
				else if ((rec_tmp & 0xF0) == 0x30)
				{
					rec_projector = (rec_tmp & 0x0F);
				}
			}
		break;
		
		case Sending:
			if(USART_IsSendReady(0))
			{
				USART_Send(st_num,0);
				_delay_ms(1);
			}
		break;
		
		default:
		break;
	}
	switch(state){ //Transactions
		case BlueTooth_Init:
			state = Receiving;
		break;
		
		case Receiving:
			state = Sending;
		break;
		
		case Sending:
			state = Receiving;
		break;
		
		default:
			state = BlueTooth_Init;
		break;
	}
	return state;
};

enum Stepper_States { Stepper_Init, Stepper_Wait, Stepper_FRolling, Stepper_BRolling };
int TickFct_Stepper(int state) {
	switch(state){ //State Actions
		case Stepper_Init:
			
		break;
		
		case Stepper_Wait:
			if (projector == 0)
			{
				PORTC |= 0x00;
			}
			else if (projector == 1)
			{
				PORTC |= 0x10;
			}
		break;
		
		case Stepper_FRolling:
			for (step=0;step<=7;step++)
			{
				PORTC = steppermove[step];
				_delay_ms(3);
			}
			stepcnt++;
		break;
		
		case Stepper_BRolling:
			for (step=7;step>=0;step--)
			{
				PORTC = steppermove[step];
				_delay_ms(3);
			}
			stepcnt++;
		break;
		
		default:
		break;
	}
	switch(state){ //Transactions
		case Stepper_Init:
			state = Stepper_Wait;
		break;
		
		case Stepper_Wait:
			if ((rec_projector!=projector) && (projector == 0))//open
			{
				projector = rec_projector;
				state = Stepper_FRolling;
			}
			else if ((rec_projector!=projector) && (projector == 1))//close
			{
				projector = rec_projector;
				state = Stepper_BRolling;
			}
			else{
				state = Stepper_Wait;
			}
		break;
		
		case Stepper_FRolling:
			if (stepcnt == 5 * FULL_ROTATE)
			{
				stepcnt = 0;
				state = Stepper_Wait;
			}
			else{
				state = Stepper_FRolling;
			}
		break;
		
		case Stepper_BRolling:
			if (stepcnt == 5 * FULL_ROTATE)
			{
				stepcnt = 0;
				state = Stepper_Wait;
			}
			else{
				state = Stepper_BRolling;
			}
		break;
		
		default:
			state = Stepper_Init;
		break;
	}
	return state;
};

enum TSensor_States { TSensor_Init, TSensor_Get };
int TickFct_TSensor(int state) {
	switch(state){ //State Actions
		case TSensor_Init:
			PORTD &= 0xEF;
		break;
		
		case TSensor_Get:
			Request();
			Response();
			I_RH=Receive_data();	/* store first eight bit in I_RH */
			D_RH=Receive_data();	/* store next eight bit in D_RH */
			I_Temp=Receive_data();	/* store next eight bit in I_Temp */
			D_Temp=Receive_data();	/* store next eight bit in D_Temp */
			CheckSum=Receive_data();
			if ((I_RH + D_RH + I_Temp + D_Temp) != CheckSum)
			{
				break;
			}
			else{
				if (I_Temp>26)
				{
					PORTD &= 0xEF;
				} 
				else
				{
					PORTD |= 0x10;
				}
			}
		break;
		
		default:
		break;
	}
	switch(state){ //Transactions
		case TSensor_Init:
		state = TSensor_Get;
		break;
		
		case TSensor_Get:
		state = TSensor_Get;
		break;
		
		default:
		state = TSensor_Init;
		break;
	}
	return state;
};

enum FSensor_States { FSensor_Init, FSensor_Get };
int TickFct_FSensor(int state) {
	switch(state){ //State Actions
		case FSensor_Init:
			InitADC();
		break;
		
		case FSensor_Get:
			adc_result=ReadADC(0)/204.8;	 
			if (adc_result > 1.8)
			{
				st_num = 1;
			} 
			else
			{
				st_num = 0;
			}
		break;
		
		default:
		break;
	}
	switch(state){ //Transactions
		case FSensor_Init:
		state = FSensor_Get;
		break;
		
		case FSensor_Get:
		state = FSensor_Get;
		break;
		
		default:
		state = FSensor_Init;
		break;
	}
	return state;
};

int main(void) {
	
	// initialize ports
	DDRC = 0xFF;
	DDRA = 0x00;
	DDRD = 0xFA;
	//enabling the ADC, setting free running mode, setting prescalar 2
	
	tasksNum = 4; // declare number of tasks
	task tsks[4]; // initialize the task array
	tasks = tsks; // set the task array

	// define tasks
	unsigned char i=0; // task counter
	tasks[i].state = BlueTooth_Init;
	tasks[i].period = 50;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_BlueTooth;
	++i;
	tasks[i].state = Stepper_Init;
	tasks[i].period = 10;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_Stepper;
	++i;
	tasks[i].state = TSensor_Init;
	tasks[i].period = 200;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_TSensor;
	++i;
	tasks[i].state = FSensor_Init;
	tasks[i].period = 50;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_FSensor;
	
	TimerSet(5); // value set should be GCD of all tasks
	TimerOn();

	while(1) {} // task scheduler will be called by the hardware interrupt
	
}




