/*********************************************************
 * Open Loop DC Motor Speed Monitor and Control System
 * Thomas Gartlan October 2016
 * This is part 1 of project to look at control of a DC motor.
 * In this version it was assumed ony 1 interrupter on motor shaft
 * In this case we look at Open Loop control
 * The Speed of the DC Motor is controlled via PWM
 * The speed is monitored by means of an opto sensor conencted
 * to CCP block in Capture mode. The microconttoller used is the PIC18F4520.
 * The code is written for the XC8 compiler within the 
 * MPLABX environment.
 * Speed is shown on an LCD display
 * Designed for when there was only one interrupter on the motor shaft:
 * System clock is 2MHz
 * Fosc/4 = 500KHz
 * Timer 1 is prescaled by 8 and is used by CCP1 in capture mode
 * So Timer 1 clock is 500KHz/8 = 62,500 Hz
 * So we could measure sown to 1Hz signal from opto but any slower and Timer1
 * could overflow!!!!
 * PORTS used
 * PORTA   LED Block (not really used)
 * PORTB   Not used (Used in later version for Buttons)
 * PORTC   CCP blocks used for PWM and Capture
 * PORTD   LCD Display
***********************************************************/

/**********************************************
Libraries
**********************************************/

#include <xc.h>
#include "../../LCD_library/lcdlib_2016.h"


/**********************************************
*  Configuration Selection bits
**********************************************/

#define _XTAL_FREQ 2000000      //Note will switch to Oscillator during startup
////used by delay library to work out exact delays
//#pragma CLOCK_FREQ 2000000
//
////see pic18F452.h file c:/program files/sourceboost/include
//#pragma config OSC = HS, LVP = OFF, WDT = OFF
//#pragma config PWRT= OFF, BOREN = OFF, MCLRE = ON
//
//   //#pragma config DEBUG = OFF
//#pragma DATA _CONFIG4L, 0x80
//#pragma DATA _CONFIG3H, 0x81 //configures the CCP2 to RC1



/****************************************
*  Global Variables
****************************************/
const unsigned char *Speed = "Speed"; 
const unsigned char *Motor = "Motor Control"; 

unsigned char i = 0;
unsigned int captured = 0;
/****************************************
*  Function Prototypes
****************************************/
void Initial(void);
void delay_1s(void);

/****************************************
*  Interrupt required to capture speed and
   reset the timer
****************************************/
void __interrupt ISR (void)
{

	if (PIR1bits.CCP1IF)
	{
	    PIR1bits.CCP1IF = 0;
		TMR1H = 0;
		TMR1L = 0;
		captured = (CCPR1H << 8);
		captured = captured + CCPR1L;
	}
}
/****************************************
*  Main Program (Foreground Program)
****************************************/
void main(void)
{
		
    unsigned int tclk = 62500;
    unsigned int rev_s = 0;
    
    lcd_start () ;
    lcd_cursor ( 2, 1 ) ;
    lcd_print ( Speed ) ;
    lcd_cursor ( 2, 0 ) ;
    lcd_print ( Motor ) ;
    //lcd_cursor_on();
    
    Initial();
	PIE1bits.CCP1IE = 1;	
    
    while(1) 
    {
		CCPR2L = 10;//Duty Cycle = (ccpr2l/pr2)*100
		for(i=0;i<5;i++)
		{
		 rev_s = tclk/captured;//calculates a value for speed
		   lcd_print_ch ( ' ' ) ;
		   lcd_cursor(9,1);
		   lcd_display_value(rev_s);//displays value on LCD
		   delay_1s();
		}
			 
		CCPR2L = 15;
		for(i=0;i<5;i++)
		{
		 rev_s = tclk/captured;
		   lcd_print_ch ( ' ' ) ;
		   lcd_cursor(9,1);
		   lcd_display_value(rev_s);
		   delay_1s();
		}

		CCPR2L = 17;
		for(i=0;i<5;i++)
		{
		 rev_s = tclk/captured;
		   lcd_print_ch ( ' ' ) ;
		   lcd_cursor(9,1);
		   lcd_display_value(rev_s);
		   delay_1s();
		}		
	   
	}
	
}


/****************************************
*  Initial
*  This function initialises SFRs and 
*  global variables
******************************************/

void Initial(void)
{
    TRISB = 0x00;
    PORTB = 0x00;
	TRISD = 0x00;
	TRISA = 0x00;
	TRISCbits.TRISC1 = 0; //These pins are configured individually
	TRISCbits.TRISC2 = 1; //as CCP1 is an input and CCP2 an output
	PORTC = 0x00;
	
	PORTB = 0x00;
	
	//Flash LED block as test
	PORTA = 0xff;
	delay_1s();
	PORTA = 0x00;
	delay_1s();
	
	OSCCONbits.IRCF2 = 1;
	OSCCONbits.IRCF1 = 0; // 2MHz Clock
	OSCCONbits.IRCF0 = 1;
	
	OSCCONbits.SCS0 = 0; //Internal Oscillator block is selected
	OSCCONbits.SCS1 = 1;
	
	CCP1CON = 0b00000101; // Capture mode; every rising edge
	
	T1CONbits.TMR1ON = 1;  // enables timer1
	T1CONbits.T1CKPS0 = 1; //Timer1 prescaled by 8
    T1CONbits.T1CKPS1 = 1; // 500KHz/8 = 62.6KHz
    T1CONbits.RD16 = 1; // Enables 16-bit operation
    
    INTCONbits.GIE = 1; //Global Interrupt enabled and 
	INTCONbits.PEIE = 1;//Peripheral interrupts unmasked
	
	CCP2CON = 0b00001100; //PWM mode
	PR2 = 20; // PR2 = (fosc/4)/Frequency = 500KHz/25KHz  = 20
	T2CON = 0b00000100; // Timer 2 is turned on
}


void delay_1s(void)
{
    unsigned char i;
    for (i=0;i<25;i++)
            __delay_ms(40);  //max value is 40 since this depends on the _delay() function which has a max number of cycles
}
