

#include <REG51.H>
#include <stdio.h>
#include <string.h>
#include "lcd.h"

//override switch
sbit P1_7 = P1^7;

//Keypad pins
sbit row_0 = P0^0;
sbit row_1 = P0^1;
sbit row_2 = P0^2;
sbit row_3 = P0^3;
sbit col_0 = P0^4;
sbit col_1 = P0^5;
sbit col_2 = P0^6;

//ADCpins
sbit ADC_RD = P1^4; // ADC RD
sbit  ADC_WR = P1^5; // ADC WR
sbit ADC_INT = P1^6; // ADC INTR


//testLed
sbit task4 = P3^5;
sbit task3 = P3^7;
sbit task2 = P3^6;
sbit task1 = P3^4;

//Motor Pins
sbit motor_1 = P1^0;     
sbit motor_2 = P1^1; 

//keypad key
unsigned char pressedKey = 1;

//overrideFlag
bit overrideFlag;

//measure Values
unsigned int revolution;
unsigned int time;
unsigned int rps;
unsigned int desiredRPS;
unsigned int desiredSpeed;
unsigned char desiredGear;

//main while loop flags
bit printMessage;
bit parseFlag;

//UART receive buffer
char overrideBuffer [6]; //buffer
unsigned char overrideIndex; //uart index counter for buffer 

//string to be parsed(saved after convenient 6 chars buffered)
char *parseMessage;

//response string(ACK or NACK)
char *response;

//constant core speeds(will be altered with ADC and Keypad
unsigned int coreSpeed;
unsigned char coreGear;

//shared speed and gear values
unsigned int speed;
unsigned char gear;


void scanKeypad();


//motor interrupt
void motor_sensor_interrupt (void) interrupt 0 
{	
	revolution++;	
	
}


//timer ISR
void timer (void) interrupt 1{
	
	time++;
	if(revolution > 63534){
		revolution = 1;
		time = 1;
	}
}

//keypad interrupt
void external_interrupt_1 (void) interrupt 2 using 1
{		
	scanKeypad();
	row_0 = 0;
	row_1 = 0;
	row_2 = 0;
	row_3 = 0;
	coreGear = pressedKey;
	IE1 = 0;
}

//UART interrupt
void test0(void) interrupt 4 using 2{
	
	
	if(RI){ 																						//Serial RX flag check
		if(P1_7){ 																				//switch check
			
			response = "NACK\n";														//set response as NACK
			printMessage = 1;																//set print flag to print response
			
		}else{		
			if(overrideIndex == 0){													//checking first character of message to validate message structure
				if(SBUF == 's'){															
					overrideBuffer[overrideIndex] = SBUF;				//valid character, save to buffer
					overrideIndex++;
				}else if(SBUF == 'u'){
					overrideBuffer[overrideIndex] = SBUF;				//valid character, save to buffer
					overrideIndex++;
				}else{
					response = "NACK\n";												//invalid character, set response as NACK
					printMessage = 1;														//set main while loop flag to print response after ISR
				}
			}else if(overrideIndex <= 4){										
			overrideBuffer[overrideIndex] = SBUF;						//save following characters to buffer if first character is valid
			overrideIndex++;		
			}else{			
			overrideBuffer[overrideIndex] = SBUF;						//save last character to buffer, which is checksum
			overrideIndex = 0;															//set override index to 0 for new messages
			parseMessage = overrideBuffer;									//save message to parse buffer
			parseFlag = 1;																	//set parseFlag to parse message in main while loop
			
			}				
		}	
		RI = 0;																						//reset RI flag to prevent ISR loops
	}
}


void disable_interrupts(){	

	EA = 0;													//disable interrupts
/*	ES = 0;													//disable serial interrupt	
	EX1 = 0; 												// Enable External interrupt 1	
	EX0 = 0; 												// Enable External interrupt 0	
	ET0 = 0;											 // Enable Timer 0 interrupt */
}

void enable_interrupts(){

	EA = 1;													//enable interrupts
/*	ES = 1;													//enable serial interrupt
	EX1 = 1; 												// Enable External interrupt 1	
	EX0 = 1; 												// Enable External interrupt 0	
	ET0 = 1;											 // Enable Timer 0 interrupt */
}


//basic power method to make string to numeric calculations
int pow(int base , int power){
	int temp = 1;
	if(base == 0){
		return 0;
	}
	while(power>0){
		temp *=base;
		power--;
	}
	
	return temp;

}


//initializations for setup
void setup(){			
	functionSet();	
	printMessage = 0;								//event Flag to proint message
	parseFlag = 0;									//event flag to parse message
	overrideFlag = 0;
	overrideIndex = 0;							//initial override index
	revolution = 0;
	time = 1;
	rps = 0;
	
	//keypad row init
	row_0 = 0;
	row_1 = 0;
	row_2 = 0;
	row_3 = 0;
	
	motor_1 = 0;
	motor_2 = 1;
	/*************************************************
					UART	
	**************************************************/
	
	SCON  = 0x50;                   /* SCON: mode 1, 8-bit UART, enable rcvr    */
	TMOD |= 0x21;                   /* TMOD: timer 1, mode 2, 8-bit reload      */
	TH1   = 0xFE;                   /* TH1:  reload value for 19200 baud         */
	TR1   = 1;                      /* TR1:  timer 1 run                        */
	TI    = 1;                      /* TI:   set TI to send first char of UART  */
	
	/**************************************************
					ADC
	********************************************/
	ADC_RD = 1;
	
	
	coreGear = 9;										//predetermined coreGearValue(will be set as keypad value)
	coreSpeed = 255;								//predetermined coreSpeed value(will be set as ADC value)
	speed = coreSpeed;
	gear = coreGear;
	enable_interrupts();						//initially,interrupts are enabled
	
	ES = 1;													//enable serial interrupt
	EX1 = 1; 												// Enable External interrupt 1	
	EX0 = 1; 												// Enable External interrupt 0	
	ET0 = 1;											 // Enable Timer 0 interrupt	
	
	IT0 = 1;		
	IT1 = 1;
	
	TH0 = 0x48;	   /* Load 8-bit in TH0 (here Timer0 used) */
	TL0 = 0x14;		/* Load 5-bit in TL0 */
	TR0 = 1;			/* Start timer0 */ 
	
	//initializing display
	functionSet2();
	entryModeSet(1, 0); // increment and no shift
	displayOnOffControl(1, 0, 0); // display on, cursor on and blinking on
	setDdRamAddress(0x00); // back to start of LCD
	sendString("MS:"); // print on LCD			
	setDdRamAddress(0x40); // back to start of LCD	
	sendString("DS:"); // print on LCD			
	setDdRamAddress(0x4D); // back to start of LCD	
	sendString("O:"); // print on LCD					
	setDdRamAddress(0x08); // back to start of LCD	
	sendString("S:"); // print on LCD					
	setDdRamAddress(0x48); // back to start of LCD	
	sendString("G:"); // print on LCD		

}

//adc read task
void readADC(){
	
		P2 = 255;
		ADC_WR = 0;
		ADC_WR = 1;
		T1 = 0;			
		while(ADC_INT){;}	
		ADC_RD = 0;
		coreSpeed = P2;
		ADC_RD = 1;	
}

//task1
//parse function to parse message content
void parse(){
		char i;
		short temp;																										//temporary speed in function stack
		short temp2;																									//temporary gear in function stack
		short checksum; 																							//checksum value for received message	
		char tempParseMessage[6];																			//temporary char buffer to hold message to be parsed	
		temp = 0;																						
		temp2 = 0;																										//initial values of stack variables
		checksum = 0;
	
		disable_interrupts();																					//entering critical section,interrupts are disabled
		memcpy(tempParseMessage,parseMessage,6);
		enable_interrupts();																					//critical section ends,reenabling interrupts	
	
		for(i = 0 ; i < 5 ; i++){
			checksum += tempParseMessage[i];														//checksum calculations
		}
		if(checksum%10 != tempParseMessage[5]-48){										//checksum validation
			//printf("checksum denied\n");
			
			disable_interrupts();																				//entering critical section,interrupts are disabled
			response = "NACK\n";																				//invalid checksum,set response message as NACK				
			printMessage = 1;																						//set print flag to print response
			enable_interrupts();																				//critical section ends,reenabling interrupts			
			
			return;																											//end parsing,message is invalid			
		}

		if(tempParseMessage[0] == 's'){																//check if message is stop command
			
			disable_interrupts();																				//entering critical section,interrupts are disabled
			gear = coreGear;																						//set coreValues to gear and speed values 
			speed = coreSpeed;
			response = "ACK\n";																					//message is valid, set response as ACK
			printMessage = 1;																						//set print flag to print response
			overrideFlag = 0;
			enable_interrupts();																				//critical section ends,reenabling interrupts		
			
			return;
			
		}else{
			if(tempParseMessage[1] < 58 && tempParseMessage[1] > 47){		//check if message element is a numeric and less than 3
				temp2 = tempParseMessage[1] -48;													//set numeric as temporary gear values
			}
			else{
				
				disable_interrupts();																			//entering critical section,interrupts are disabled
				response = "NACK\n";																			//element is not numeric,message is invalid
				printMessage = 1;																					//set print flag to print response
				enable_interrupts();																			//critical section ends,reenabling interrupts
				
				return;																										//end parsing,message is invalid			
			}
			for(i = 2 ; i < 5 ; i++){				
				if(tempParseMessage[i] < 58 && tempParseMessage[i] > 47){	//check is elements are numeric					
					temp += ((tempParseMessage[i] -48)*pow(10,4-i));				//convert numeric chars to numeric values					
				}else{
					
				disable_interrupts();																			//entering critical section,interrupts are disabled
				response = "NACK\n";																			//elements are not numeric,message is invalid
				printMessage = 1;																					//set print flag to print response
				enable_interrupts();																			//critical section ends,reenabling interrupts	
					
				return;																										//end parsing,message is invalid
				}
			}
			if(temp > 255){
				
				disable_interrupts();																			//entering critical section,interrupts are disabled
				response = "NACK\n";																			//elements are not numeric,message is invalid
				printMessage = 1;																					//set print flag to print response
				enable_interrupts();																			//critical section ends,reenabling interrupts	
					
				return;																										//end parsing,message is invalid
			}			
			disable_interrupts();																				//entering critical section,interrupts are disabled
			speed = temp;																								//message is valid and program in override state,set parsed speed as speed
			gear = temp2;																								//set parsed gear as gear
			response = "ACK\n";																					//message is valid, set response as ACK
			printMessage = 1;																						//set print flag to print response
			overrideFlag = 1;
			enable_interrupts();																				//critical section ends,reenabling interrupts	
			
		}
}

//task 2
void measure(){
	
	disable_interrupts();																			//entering critical section,interrupts are disabled
				
	rps = ((revolution/time)*114)/100;
	if(P1_7){
		
		desiredSpeed = (coreSpeed*10)/23;
		desiredGear = coreGear;
		
	}else{
		
		if(overrideFlag){
			desiredSpeed = (speed*10)/23;
			desiredGear = gear;
		}
		else{
			desiredSpeed = (coreSpeed*10)/23;
			desiredGear = coreGear;
		}	
		
	}	
	desiredRPS = desiredSpeed*desiredGear;
	enable_interrupts();
	if(rps<desiredRPS){
		motor_2 = 1;
	}else{
		motor_2 = 0;
	}
	
}


//task 3
void display(){	
		char text[16];
		
		
		enable_interrupts();
		setDdRamAddress(0x03); // back to start of LCD
		sprintf(text,"%-3u",rps);
	  //sprintf(text,"%-3u:%-3u",coreSpeed,speed);
		sendString(text); // print on LCD		
		setDdRamAddress(0x43); // back to start of LCD
		sprintf(text,"%-3u",desiredRPS);	
		//sprintf(text,"%-3u:%-3u",coreGear,gear);		
		sendString(text); // print on LCD		
		setDdRamAddress(0x4F); // back to start of LCD			
		if(P1_7){					
			
			sendString("0"); // print on LCD	
			
		}else{
			
			disable_interrupts();		
			if(overrideFlag){				
				sendString("1"); // print on LCD	
			}else{
				sendString("0");
			}			
			enable_interrupts();
		}	
		
		setDdRamAddress(0x0A); // back to start of LCD			
		sprintf(text,"%-3u",desiredSpeed);
		sendString(text); // print on LCD					
		setDdRamAddress(0x4A); // back to start of LCD	
		sprintf(text,"%-1u",(unsigned int)desiredGear);
		sendString(text); // print on LCD
	

}



void main (void)  {	
		setup();	
	
		while(1){
			//task 0
			if(printMessage){						//print event
				printf("response : %s",response);
				//printf("parseMessage : %s\n",parseMessage);				
				//printf("gear : %d\nspeed : %d\n",gear,speed);
				printMessage = 0;					
				
			}
			
			//task1
			task1 = 0;
			readADC();
			task1 = 1;
			
			
			//task 2
			
			if(parseFlag){							//parse event
				task2 = 0;
				parse();
				
				parseFlag = 0;
				task2 = 1;
			}
			
			//task 3		
			task3 = 0;			
			measure();
			task3 = 1;
			
			//task 4
			task4 = 0;
			display();
			task4 = 1;
			
		}
	
	
	
}
#pragma registerbank(1)
void scanKeypad(){
	row_0 = 0;
	row_1 = 1;
	row_2 = 1;
	row_3 = 1;
	
	if(!col_0){
		return;
	}
	else if(!col_1){
		pressedKey = '0';
		return;
	}
	else if(!col_2){
		return;
	}
	
	row_0 = 1;
	row_1 = 0;
	row_2 = 1;
	row_3 = 1;
	if(!col_0){
		pressedKey = 9;
		return;
	}
	else if(!col_1){
		pressedKey = 8;
		return;
	}
	else if(!col_2){
		pressedKey = 7;
		return;
	}
	
	row_0 = 1;
	row_1 = 1;
	row_2 = 0;
	row_3 = 1;
	if(!col_0){
		pressedKey = 6;
		return;
	}
	else if(!col_1){
		pressedKey = 5;
		return;
	}
	else if(!col_2){
		pressedKey = 4;
		return;
	}
	
	row_0 = 1;
	row_1 = 1;
	row_2 = 1;
	row_3 = 0;
	if(!col_0){
		pressedKey = 3;
		return;
	}
	else if(!col_1){
		pressedKey = 2;
		return;
	}
	else if(!col_2){
		pressedKey = 1;
		return;
	}
	
}