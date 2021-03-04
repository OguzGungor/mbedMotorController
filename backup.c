

#include <REG51.H>
#include <stdio.h>
#include <string.h>

//override switch
sbit P1_7 = P1^7;

//main while loop flags
int printMessage;
int parseFlag;

//UART receive buffer
char overrideBuffer [6]; //buffer
int overrideIndex; //uart index counter for buffer 

//string to be parsed(saved after convenient 6 chars buffered)
char *parseMessage;

//response string(ACK or NACK)
char *response;

//constant core speeds(will be altered with ADC and Keypad
int coreSpeed;
int coreGear;

//shared speed and gear values
int speed;
int gear;

void test0(void) interrupt 4 using 2{
	
	
	if(RI){ 																						//Serial RX flag check
		if(P1_7){ 																				//switch check
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
		}else{																						//if switch is closed
			response = "NACK\n";														//set response as NACK
			printMessage = 1;																//set print flag to print response
		}	
		RI = 0;																						//reset RI flag to prevent ISR loops
	}
}
void disable_interrupts(){	

	EA = 0;													//disable interrupts
	ES = 0;													//disable serial interrupt
}

void enable_interrupts(){

	EA = 1;													//enable interrupts
	ES = 1;													//enable serial interrupt

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

//parse function to parse message content
void parse(){
		char i;
		short temp;																						//temporary speed in function stack
		short temp2;																					//temporary gear in function stack
		short checksum; 																			//checksum value for received message
		temp = 0;																						
		temp2 = 0;																						//initial values of stack variables
		checksum = 0;
		char tempParseMessage[6];															//temporary char buffer to hold message to be parsed
		disable_interrupts();																	//entering critical section,interrupts are disabled
		
		enable_interrupts();																	//critical section ends,reenabling interrupts
		for(i = 0 ; i < 5 ; i++){
			checksum += parseMessage[i];												//checksum calculations
		}
		if(checksum%10 != parseMessage[5]-48){								//checksum validation
			//printf("checksum denied\n");
			response = "NACK\n";																//invalid checksum,set response message as NACK				
			printMessage = 1;																		//set print flag to print response
			return;																							//end parsing,message is invalid
		}
		if(parseMessage[0] == 's'){														//check if message is stop command
			gear = coreGear;																		//set coreValues to gear and speed values 
			speed = coreSpeed;
			response = "ACK\n";																	//message is valid, set response as ACK
			printMessage = 1;																		//set print flag to print response
		}else{
			if(parseMessage[1] < 58 && parseMessage[1] > 47){		//check if message element is a numeric
				temp2 = parseMessage[1] -48;											//set numeric as temporary gear values
			}
			else{
				response = "NACK\n";															//element is not numeric,message is invalid
				printMessage = 1;																	//set print flag to print response
				return;																						//end parsing,message is invalid
			}
			for(i = 2 ; i < 5 ; i++){
				if(parseMessage[i] < 58 && parseMessage[i] > 47){	//check is elements are numeric
					temp += ((parseMessage[i] -48)*pow(10,4-i));		//convert numeric chars to numeric values
				}else{
				response = "NACK\n";															//elements are not numeric,message is invalid
				printMessage = 1;																	//set print flag to print response
				return;																						//end parsing,message is invalid
				}
			}
			speed = temp;																				//message is valid and program in override state,set parsed speed as speed
			gear = temp2;																				//set parsed gear as gear
			response = "ACK\n";																	//message is valid, set response as ACK
			printMessage = 1;																		//set print flag to print response
		}
}


void main (void)  {	
	printMessage = 0;								//event Flag to proint message
	parseFlag = 0;									//event flag to parse message
	overrideIndex = 0;							//initial override index
	SCON  = 0x50;                   /* SCON: mode 1, 8-bit UART, enable rcvr    */
	TMOD |= 0x20;                   /* TMOD: timer 1, mode 2, 8-bit reload      */
	TH1   = 0xFE;                   /* TH1:  reload value for 19200 baud         */
	TR1   = 1;                      /* TR1:  timer 1 run                        */
	TI    = 1;                      /* TI:   set TI to send first char of UART  */
	coreGear = 3;										//predetermined coreGearValue(will be set as keypad value)
	coreSpeed = 130;								//predetermined coreSpeed value(will be set as ADC value)
	enable_interrupts();						//initially,interrupts are enabled
	
	
		while(1){
			
			if(printMessage){						//print event
				printf("response : %s",response);
				//printf("parseMessage : %s\n",parseMessage);				
				printf("gear : %d\nspeed : %d\n",gear,speed);
				printMessage = 0;
				
			}
			if(parseFlag){							//parse event
			
				parse();
				
				parseFlag = 0;
			}
			
		}
	
	
	
}