/*
												Amrita Vishwa Vidyapeetham
												Amrita School of Computing, Coimbatore
												B.Tech Lab session– December 2022
												Fifth Semester
												Computer Science and Engineering
												19CSE303 Embedded Systems 
Questions
-------------
1.	Demonstrate “password authentication system” using LPC2148 microcontroller.
a.	Receive the password from user through UART (RX) and compare the string, if matches switch on LED (p1.16). otherwise buzzer should generate waring sound.

Note: password may be single character or string.

*/



#include <lpc214x.h>

#define LED1_ON() IO1SET=(1<<16)
#define LED2_ON() IO1SET=(1<<17)
#define LED3_ON() IO1SET=(1<<18)
#define LED4_ON() IO1SET=(1<<19)

#define LED1_OFF() IO1CLR=(1<<16)
#define LED2_OFF() IO1CLR=(1<<17)
#define LED3_OFF() IO1CLR=(1<<18)
#define LED4_OFF() IO1CLR=(1<<19)

#define BUZZER_ON() IO0CLR=(1<<11)		 //Macro Functions to turn ON Buzzer
#define BUZZER_OFF() IO0SET=(1<<11)		 //Macro Functions to turn OFF Buzzer


#define PLOCK 0x00000400 // for PLL
#define THRE (1<<5) // Transmit Holding Register Empty
#define RDR (1<<0) // Receiver Data Ready
#define MULVAL 15
#define DIVADDVAL 1
#define NEW_LINE 0xA // Character for new line .. analogus to '\n'
#define ENTER 0xD // Ascii code for Enter 

void initUART0(void);
void U0Write(char data);
char U0Read(void);
void initClocks(void);

void setupPLL0(void);
void feedSeq(void);
void connectPLL0(void);

char pass[] = "embedded";
int n=8;
int i=0;
int main(void)
{
	PINSEL0 = 0x00000000;		// Enable GPIO on all pins	
 PINSEL1 = 0x00000000;
 PINSEL2 = 0x00000000;
IO1DIR = (1<<16) | (1<<17) | (1<<18) | (1<<19);
IO0DIR = (1<<11) ;
	initClocks(); // Set CCLK=60Mhz and PCLK=60Mhz 
	initUART0();
	LED1_OFF();
	BUZZER_OFF();
	while(1)		
	{
		char c = U0Read(); // Read Data from Rx
		U0Write(c);
		//U0Write(pass[i]);
		//U0Write(NEW_LINE);
		if (i>=n-1){
			LED1_ON();
			break;
		}
		if( c == '\n' ) // Check if user pressed Enter key
		{
			i=0;
			LED1_OFF();
			BUZZER_OFF();
		}
		else if (c == pass[i]){
			i++;
		}
		else{
			BUZZER_ON();
		}
	}
	
	return 0;
}

void initUART0(void)
{
	PINSEL0 = 0x5;  /* Select TxD for P0.0 and RxD for P0.1 */
	U0LCR = 3 | (1<<7) ; /* 8 bits, no Parity, 1 Stop bit | DLAB set to 1  */
	U0DLL = 110;
	U0DLM = 1;   
	U0FDR = (MULVAL<<4) | DIVADDVAL; /* MULVAL=15(bits - 7:4) , DIVADDVAL=0(bits - 3:0)  */
	U0LCR &= 0x0F; // Set DLAB=0 to lock MULVAL and DIVADDVAL
	//BaudRate is now ~9600 and we are ready for UART communication!
}

void U0Write(char data)
{
	while ( !(U0LSR & THRE ) ); // wait till the THR is empty
	// now we can write to the Tx FIFO
	U0THR = data;
}

char U0Read(void)
{
	while( !(U0LSR & RDR ) ); // wait till any data arrives into Rx FIFO
	return U0RBR;
}

void initClocks(void)
{
	setupPLL0();
	feedSeq(); //sequence for locking PLL to desired freq.
	connectPLL0();
	feedSeq(); //sequence for connecting the PLL as system clock

	//SysClock is now ticking @ 60Mhz!
	   
	VPBDIV = 0x01; // PCLK is same as CCLK i.e 60Mhz

	//Using PLL settings as shown in : http://www.ocfreaks.com/lpc214x-pll-tutorial-for-cpu-and-peripheral-clock/
	//PLL0 Now configured!
}


//---------PLL Related Functions :---------------

void setupPLL0(void)
{
	//Note : Assuming 12Mhz Xtal is connected to LPC2148.

	PLL0CON = 0x01; // PPLE=1 & PPLC=0 so it will be enabled
					// but not connected after FEED sequence
	PLL0CFG = 0x24; // set the multipler to 5 (i.e actually 4)
					// i.e 12x5 = 60 Mhz (M - 1 = 4)!!!
					// Set P=2 since we want FCCO in range!!!
					// So , Assign PSEL =01 in PLL0CFG as per the table.
}

void feedSeq(void)
{
	PLL0FEED = 0xAA;
	PLL0FEED = 0x55;
}

void connectPLL0(void)
{
	// check whether PLL has locked on to the desired freq by reading the lock bit
	// in the PPL0STAT register

	while( !( PLL0STAT & PLOCK ));

	// now enable(again) and connect
	PLL0CON = 0x03;
}
