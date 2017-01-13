/*********************************************************
 * Open Loop DC Motor Speed Monitor and Control System
 * Thomas Gartlan October 2016
 * This is part 1 of project to look at control of a DC motor.
 * In this version there are 4 interrupter on motor shaft
 * In this case we look at Open Loop control
 * The Speed of the DC Motor is controlled via PWM
 * The speed is monitored by means of an opto sensor conencted
 * to CCP block in Capture mode. The microconttoller used is the PIC18F4520.
 * The code is written for the XC8 compiler within the 
 * MPLABX environment.
 * Speed is shown on an LCD display
 * Designed for when there is 4 interrupter on the motor shaft:
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

#define _XTAL_FREQ 8000000      //Note will switch to Oscillator during startup

/****************************************
*  Global Variables
****************************************/
const unsigned char *Speed = "Speed "; 
const unsigned char *Motor = "PWM_%"; 

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
		
    unsigned short long tclk = 250000; //note how it's defined
    unsigned int rev_s = 0;
    unsigned char duty[18] = {5,10,15,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90};
    unsigned char p =0;
    
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
		
        
        for (p=0;p<18;p++)
        {    
            CCPR2L = duty[p];//Duty Cycle = (ccpr2l/pr2)*100     
            for(i=0;i<5;i++)
            {
               (captured==0) ? (rev_s = 0): (rev_s = tclk/captured);//calculates a value for speed
               captured = 0; //rset to zero after value is used!!! otherwise if motor stops last captured value continues to be used
               lcd_cursor(9,0);
               lcd_display_value(duty[p]);
               //lcd_print_ch ( ' ' ) ;
               lcd_cursor(9,1);
               lcd_display_value(rev_s);
               //lcd_display_int_value(captured);//displays value on LCD
               delay_1s();
            }
        }
        p=18;
        do 
        {   
            p = p-1;
            CCPR2L = duty[p];//Duty Cycle = (ccpr2l/pr2)*100     
            for(i=0;i<5;i++)
            {
               (captured==0) ? (rev_s = 0): (rev_s = tclk/captured);//calculates a value for speed
               captured = 0; //rset to zero after value is used!!! otherwise if motor stops last captured value continues to be used
               lcd_cursor(9,0);
               lcd_display_value(duty[p]);
               //lcd_print_ch ( ' ' ) ;
               lcd_cursor(9,1);
               lcd_display_value(rev_s);
               //lcd_display_int_value(captured);//displays value on LCD
               delay_1s();
            }
            
        } while (p > 0);
        
			
	   
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
	OSCCONbits.IRCF1 = 1; 
	OSCCONbits.IRCF0 = 1;  //8MHz Clock
	
	OSCCONbits.SCS0 = 0; //Internal Oscillator block is selected
	OSCCONbits.SCS1 = 1;
	
	CCP1CON = 0b00000101; // Capture mode; every rising edge
	
	T1CONbits.TMR1ON = 1;  // enables timer1
	T1CONbits.T1CKPS0 = 1; //Timer1 prescaled by 8
    T1CONbits.T1CKPS1 = 1; // 2MHz/8 = 250,000
    T1CONbits.RD16 = 1; // Enables 16-bit operation
    
    INTCONbits.GIE = 1; //Global Interrupt enabled and 
	INTCONbits.PEIE = 1;//Peripheral interrupts unmasked
	
	CCP2CON = 0b00001100; //PWM mode
	PR2 = 100; // PR2 = (fosc/4)/Frequency = 2Mhz/20KHz  = 100
	T2CON = 0b00000100; // Timer 2 is turned on but not prescaled
}


void delay_1s(void)
{
    unsigned char i;
    for (i=0;i<25;i++)
            __delay_ms(40);  //max value is 40 since this depends on the _delay() function which has a max number of cycles
}
