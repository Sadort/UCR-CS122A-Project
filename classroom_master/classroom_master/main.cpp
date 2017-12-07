/*
 * classroom_master.cpp
 *
 * Created: 2017/11/14 12:15:54
 * Author : sadort
 */ 

// The following is sample code for how to create a main.c that works with scheduler.h
// This program will cause D0 to blink on and off every 1500ms. D4D5D6 will rotate every 500ms.
// This code will work with the ATMega1284 or ATMega32

#include <avr/io.h>
#include <scheduler.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "lcd.h"
#include "keypad.h"
#include <usart_ATmega1284.h>

#define D0 eS_PORTA0
#define D1 eS_PORTA1
#define D2 eS_PORTA2
#define D3 eS_PORTA3
#define D4 eS_PORTA4
#define D5 eS_PORTA5
#define D6 eS_PORTA6
#define D7 eS_PORTA7
#define RS eS_PORTB6
#define EN eS_PORTB7

//global variables
unsigned char send_tmp = 0;
unsigned char rec_tmp = 0;
unsigned char tmp_key = 0;
unsigned char light = 7;//front, middle, back
unsigned char projector = 0;
unsigned char room = 1;//available
unsigned char temp = 0;
char PSW[5] = "1234";
char input[9];
char ch_stnum[5];
int i_stnum = 0;
int num = 0;
int cnt = 0;
int i = 0;

//stored strings
char Welcome[16] = "      Welcome!";
char AccRefused[20] = "    Access Refused ";
char EnterPSW[30] = "   Please Enter The Password:";
char LogAccepted[22] = "   Login Successfully";
char MainInter[42] = "   1.Room Set 2.Light Set 3.Projector Set";
char StNumber[4] = "No.";
char RoomInter[39] = "   1.Available 2.Recommended Lec 3.Lec";
char LightInter[33] = "   1.Front 2.Middle 3.Back 4.OFF";
char ProjectorInter[24] = "   1.Turn on 2.Turn off";

//LCD state machine
enum LCD_States { LCD_Init, Password, Refused, MainInterface, RoomInterface, LightInterface, ProjectorInterface };
int TickFct_LCD(int state) {
	switch(state) { // State Actions
		case LCD_Init: 
			Lcd8_Init();
			Lcd8_Set_Cursor(1,0);
			Lcd8_Write_String(Welcome);
			light = 7;
			projector = 0;
			room = 1;
			i_stnum = 0;
			cnt = 0;
			memset(input, 0, strlen(input));
			_delay_ms(200);
		break;
		
		case Password:
			if (cnt <= strlen(EnterPSW)-16)
			{
				Lcd8_Clear();
				Lcd8_Set_Cursor(1,0);
				Lcd8_Write_String(EnterPSW+cnt);
				cnt++;
				Lcd8_Set_Cursor(2,0);
				Lcd8_Write_String(input);
				_delay_ms(100);
			} 
			else
			{
				Lcd8_Clear();
				Lcd8_Set_Cursor(1,0);
				Lcd8_Write_String(EnterPSW+2*strlen(EnterPSW)-32-cnt);
				cnt++;
				Lcd8_Set_Cursor(2,0);
				Lcd8_Write_String(input);
				_delay_ms(100);
				if (cnt == 2*strlen(EnterPSW)-31)
				{
					cnt = 0;
				}
			}
		break;
		
		case Refused:
			Lcd8_Clear();
			Lcd8_Set_Cursor(1,0);
			Lcd8_Write_String(AccRefused);
			cnt = 0;
			memset(input, 0, strlen(input));
			_delay_ms(100);
		break;
		
		case MainInterface:
			if (cnt <= strlen(MainInter)-16)
			{
				Lcd8_Clear();
				Lcd8_Set_Cursor(1,0);
				Lcd8_Write_String(MainInter+cnt);
				cnt++;
				Lcd8_Set_Cursor(2,0);
				Lcd8_Write_String(input);
				Lcd8_Set_Cursor(2,9);
				Lcd8_Write_String(StNumber);
				Lcd8_Set_Cursor(2,13);
				itoa(i_stnum,ch_stnum,10);
				Lcd8_Write_String(ch_stnum);
				_delay_ms(100);
			}
			else
			{
				Lcd8_Clear();
				Lcd8_Set_Cursor(1,0);
				Lcd8_Write_String(MainInter+2*strlen(MainInter)-32-cnt);
				cnt++;
				Lcd8_Set_Cursor(2,0);
				Lcd8_Write_String(input);
				Lcd8_Set_Cursor(2,9);
				Lcd8_Write_String(StNumber);
				Lcd8_Set_Cursor(2,13);
				itoa(i_stnum,ch_stnum,10);
				Lcd8_Write_String(ch_stnum);
				_delay_ms(100);
				if (cnt == 2*strlen(MainInter)-31)
				{
					cnt = 0;
				}
			}
		break;
		
		case RoomInterface:
			if (cnt <= strlen(RoomInter)-16)
			{
				Lcd8_Clear();
				Lcd8_Set_Cursor(1,0);
				Lcd8_Write_String(RoomInter+cnt);
				cnt++;
				Lcd8_Set_Cursor(2,0);
				Lcd8_Write_String(input);
				_delay_ms(100);
			}
			else
			{
				Lcd8_Clear();
				Lcd8_Set_Cursor(1,0);
				Lcd8_Write_String(RoomInter+2*strlen(RoomInter)-32-cnt);
				cnt++;
				Lcd8_Set_Cursor(2,0);
				Lcd8_Write_String(input);
				_delay_ms(100);
				if (cnt == 2*strlen(RoomInter)-31)
				{
					cnt = 0;
				}
			}
		break;
		
		case LightInterface:
			if (cnt <= strlen(LightInter)-16)
			{
				Lcd8_Clear();
				Lcd8_Set_Cursor(1,0);
				Lcd8_Write_String(LightInter+cnt);
				cnt++;
				Lcd8_Set_Cursor(2,0);
				Lcd8_Write_String(input);
				_delay_ms(100);
			}
			else
			{
				Lcd8_Clear();
				Lcd8_Set_Cursor(1,0);
				Lcd8_Write_String(LightInter+2*strlen(LightInter)-32-cnt);
				cnt++;
				Lcd8_Set_Cursor(2,0);
				Lcd8_Write_String(input);
				_delay_ms(100);
				if (cnt == 2*strlen(LightInter)-31)
				{
					cnt = 0;
				}
			}
		break;
		
		case ProjectorInterface:
			if (cnt <= strlen(ProjectorInter)-16)
			{
				Lcd8_Clear();
				Lcd8_Set_Cursor(1,0);
				Lcd8_Write_String(ProjectorInter+cnt);
				cnt++;
				Lcd8_Set_Cursor(2,0);
				Lcd8_Write_String(input);
				_delay_ms(100);
			}
			else
			{
				Lcd8_Clear();
				Lcd8_Set_Cursor(1,0);
				Lcd8_Write_String(ProjectorInter+2*strlen(ProjectorInter)-32-cnt);
				cnt++;
				Lcd8_Set_Cursor(2,0);
				Lcd8_Write_String(input);
				_delay_ms(100);
				if (cnt == 2*strlen(ProjectorInter)-31)
				{
					cnt = 0;
				}
			}
		break;
		
		default:
			state = LCD_Init;
		break;
	}

	switch(state) { // Transactions
		case LCD_Init:
			state = Password;
		break;
		
		case Password:
			if (GetKeypadKey() == '#')
			{
				if (strcmp(input,PSW)!=0)
				{
					state = Refused;
				} 
				else if (strcmp(input,PSW)==0)
				{
					Lcd8_Clear();
					Lcd8_Set_Cursor(1,1);
					Lcd8_Write_String(LogAccepted);
					cnt = 0;
					memset(input, 0, strlen(input));
					_delay_ms(100);
					state = MainInterface;
				}
				else{
					cnt = 0;
					memset(input, 0, strlen(input));
					state = Password;
				}
			}
		break;
		
		case Refused:
			state = Password;
		break;
		
		case MainInterface:
			if (GetKeypadKey() == '*')
			{
				state = LCD_Init;
			}
			if (GetKeypadKey() == '#')
			{
				if (strcmp(input,"1")==0)
				{
					cnt = 0;
					memset(input, 0, strlen(input));
					state = RoomInterface;
				}
				else if (strcmp(input,"2")==0)
				{
					cnt = 0;
					memset(input, 0, strlen(input));
					state = LightInterface;
				}
				else if (strcmp(input,"3")==0)
				{
					cnt = 0;
					memset(input, 0, strlen(input));
					state = ProjectorInterface;
				}
				else{
					cnt = 0;
					memset(input, 0, strlen(input));
					state = MainInterface;
				}
			}
			
		break;
		
		case RoomInterface:
			if (GetKeypadKey() == '*')
			{
				state = LCD_Init;
			}
			if (GetKeypadKey() == '#')
			{
				if (strcmp(input,"1")==0)
				{
					room = 1;
					cnt = 0;
					memset(input, 0, strlen(input));
					state = MainInterface;
				}
				else if (strcmp(input,"2")==0)
				{
					room = 2;
					cnt = 0;
					memset(input, 0, strlen(input));
					state = MainInterface;
				}
				else if (strcmp(input,"3")==0)
				{
					room = 3;
					cnt = 0;
					memset(input, 0, strlen(input));
					state = MainInterface;
				}
				else{
					cnt = 0;
					memset(input, 0, strlen(input));
					state = RoomInterface;
				}
			}
		break;
		
		case LightInterface:
			if (GetKeypadKey() == '*')
			{
				state = LCD_Init;
			}
			if (GetKeypadKey() == '#')
			{
				if (strlen(input) > 3 || strlen(input) == 0)
				{
					cnt = 0;
					memset(input, 0, strlen(input));
					state = LightInterface;
					break;
				}
				for (temp=0,i=0;i<strlen(input);i++)
				{
					if (input[i] == '1')
					{
						temp += 1;
					}
					else if (input[i] == '2')
					{
						temp += 2;
					}
					else if (input[i] == '3')
					{
						temp += 4;
					}
					else if (input[i] == '4')
					{
						temp = 0;
						break;
					}
					else{
						;
					}
				}
				light = temp;
				cnt = 0;
				memset(input, 0, strlen(input));
				state = MainInterface;
			}
		break;
		
		case ProjectorInterface:
			if (GetKeypadKey() == '*')
			{
				state = LCD_Init;
			}
			if (GetKeypadKey() == '#')
			{
				if (strcmp(input,"1")==0)
				{
					projector = 1;
					cnt = 0;
					memset(input, 0, strlen(input));
					state = MainInterface;
				}
				else if (strcmp(input,"2")==0)
				{
					projector = 0;
					cnt = 0;
					memset(input, 0, strlen(input));
					state = MainInterface;
				}
				else{
					cnt = 0;
					memset(input, 0, strlen(input));
					state = ProjectorInterface;
				}
			}
		break;
		
		default:
			state = LCD_Init;
		break;
	}
	
	return state;
}

enum Keypad_States { Keypad_Init, GetKey, DeleteKey};
int TickFct_Keypad(int state) {
	switch(state){ // State Actions
		case Keypad_Init:
		
		break;
		
		case GetKey:
			
		break;
		
		case DeleteKey:
		
		break;
		
		default:
		
		break;
	}
	
	switch(state){ // Transactions
		case Keypad_Init:
			state = GetKey;
		break;
		
		case GetKey:
			tmp_key = GetKeypadKey();
			if (tmp_key <= '9' && tmp_key >= '1')
			{
				if (strlen(input)<=9)
				{
					num = strlen(input);
					input[num] = tmp_key;
					input[num+1] = '\0';
					state = GetKey;
				}
			}
			else if (tmp_key == '0')
			{
				if (strlen(input)>0)
				{
					num = strlen(input)-1;
					input[num] = '\0';
					state = DeleteKey;
				}
			}
			else
			{
				state = GetKey;
			}
		break;
		
		case DeleteKey:
			state = GetKey;
		break;
		
		default:
		 state = Keypad_Init;
		break;
	}
	return state;
};

enum BlueTooth_States { BlueTooth_Init, Sending, Receiving };
int TickFct_BlueTooth(int state) {
	switch(state){ //State Actions
		case BlueTooth_Init:
			initUSART(0);
			USART_Flush(0);
		break;
		
		case Sending:
			if(USART_IsSendReady(0))
			{
				send_tmp = 0x10 + room;
				USART_Send(send_tmp,0);
				send_tmp = 0x20 + light;
				USART_Send(send_tmp,0);
				send_tmp = 0x30 + projector;
				USART_Send(send_tmp,0);
				_delay_ms(2);
			}
		break;
		
		case Receiving:
			if(USART_HasReceived(0))
			{
				rec_tmp = USART_Receive(0);
				i_stnum = rec_tmp - 0x00;
			}
		break;
		
		default:
		break;
	}
	switch(state){ //Transactions
		case BlueTooth_Init:
			state = Sending;
		break;
		
		case Sending:
			state = Receiving;
		break;
		
		case Receiving:
			state = Sending;
		break;
		
		default:
			state = BlueTooth_Init;
		break;
	}
	return state;
};
int main(void) {
	
	// initialize ports
	DDRA = 0xFF;
	DDRB = 0xFF;
	DDRC = 0xF0;
	
	tasksNum = 3; // declare number of tasks
	task tsks[3]; // initialize the task array
	tasks = tsks; // set the task array
	
	// define tasks
	unsigned char i=0; // task counter
	tasks[i].state = Keypad_Init;
	tasks[i].period = 600;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_LCD;
	++i;
	tasks[i].state = LCD_Init;
	tasks[i].period = 500;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_Keypad;
	++i;
	tasks[i].state = BlueTooth_Init;
	tasks[i].period = 500;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_BlueTooth;
	
	TimerSet(100); // value set should be GCD of all tasks
	TimerOn();

	while(1) {} // task scheduler will be called by the hardware interrupt
	
}
