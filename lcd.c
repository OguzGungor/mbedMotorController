#include <reg51.h>
#include "lcd.h"


sbit DB7 = P2^7;
sbit DB6 = P2^6;
sbit DB5 = P2^5;
sbit DB4 = P2^4;
sbit RS = P1^3;
sbit E = P1^2;


void clearDisplay(){
	/*returnHome();
	sendString("                ");		
	setDdRamAddress(0x40); // set address to start of second line	
	sendString("                ");		
	setDdRamAddress(0x00); // set address to start of second line	
	returnHome();*/
	sendString("   ");	
	returnHome();
	sendString(" ");

}

void disp(char *text){
	
	sendString(text);
	returnHome();

}



//void main(void) {
void lcdDisplay(){
	functionSet();
	entryModeSet(1, 0); // increment and no shift
	displayOnOffControl(1, 0, 0); // display on, cursor on and blinking on
	
	/*sendString(text);
	setDdRamAddress(0x40); // set address to start of second line
	sendString(text);*/

	// The program can be controlled via some of the switches on port 2.
	// If switch 5 is closed the cursor returns home (address 0).
	// Otherwise, switches 6 and 7 are read - if both switches are open or both switches 
	//      are closed, the display does not shift.
	// If switch 7 is closed, continuously shift left.
	// If switch 6 is closed, continuously shift right.
	//while (1) {
		/*if (ret == 0) {
			returnHome();
		}
		else {	
			if (left == 0 && right == 1) {
				cursorOrDisplayShift(1, 0); // shift display left
			}
			else if (left == 1 && right == 0) {
				cursorOrDisplayShift(1, 1); // shift display right
			}	
		}	*/
	//}
}

// LCD Module instructions -------------------------------------------
// To understand why the pins are being set to the particular values in the functions
// below, see the instruction set.
// A full explanation of the LCD Module: HD44780.pdf

void returnHome(void) {
	RS = 0;
	DB7 = 0;
	DB6 = 0;
	DB5 = 0;
	DB4 = 0;
	E = 1;
	E = 0;
	DB5 = 1;
	E = 1;
	E = 0;
	//delay();
}	


void entryModeSet(bit id, bit s) {
	RS = 0;
	DB7 = 0;
	DB6 = 0;
	DB5 = 0;
	DB4 = 0;
	E = 1;
	E = 0;
	DB6 = 1;
	DB5 = id;
	DB4 = s;
	E = 1;
	E = 0;
	//delay();
}

void displayOnOffControl(bit display, bit cursor, bit blinking) {
	DB7 = 0;
	DB6 = 0;
	DB5 = 0;
	DB4 = 0;
	E = 1;
	E = 0;
	DB7 = 1;
	DB6 = display;
	DB5 = cursor;
	DB4 = blinking;
	E = 1;
	E = 0;
	//delay();
}



void functionSet(void) {
	DB7 = 0;
	DB6 = 0;
	DB5 = 1;
	DB4 = 0;
	RS = 0;
	E = 1;
	E = 0;
}

void functionSet2(void){
	E = 1;
	E = 0;
	DB7 = 1;
	E = 1;
	E = 0;

}

void setDdRamAddress(char address) {
	RS = 0;
	DB7 = 1;
	DB6 = getBit(address, 6);
	DB5 = getBit(address, 5);
	DB4 = getBit(address, 4);
	E = 1;
	E = 0;
	DB7 = getBit(address, 3);
	DB6 = getBit(address, 2);
	DB5 = getBit(address, 1);
	DB4 = getBit(address, 0);
	E = 1;
	E = 0;
	//delay();
}

// --------------------------------------------------------------------

void sendChar(char c) {
	DB7 = getBit(c, 7);
	DB6 = getBit(c, 6);
	DB5 = getBit(c, 5);
	DB4 = getBit(c, 4);
	RS = 1;
	E = 1;
	E = 0;
	DB7 = getBit(c, 3);
	DB6 = getBit(c, 2);
	DB5 = getBit(c, 1);
	DB4 = getBit(c, 0);
	E = 1;
	E = 0;
	//delay();
}

void sendString(char* str) {
	int index = 0;
	while (str[index] != 0) {
		sendChar(str[index]);
		index++;
	}
}

bit getBit(char c, char bitNumber) {
	return (c >> bitNumber) & 1;
}
/*
void delay(void) { //LCD Delay
	char c;
	for (c = 0; c < 50; c++);
}*/