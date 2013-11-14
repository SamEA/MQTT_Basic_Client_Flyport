/* **************************************************************************
 *  FileName:        millis.c
 *  Module:          FlyPort WI-FI
 *
 *  Author               Rev.    Date              Comment
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  SamEA                1.0    08/01/2013		      needs to be tested
 *  
 *  					  
 *
 *  					                                 
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
    
#include "HWlib.h"
#include "p24FJ256GA106.h"
long int millisCount;

//millis uses Timer4 
void millisInit(){
millisCount=0;
	IEC1bits.T4IE = 0; //Disable timer4 interrupt
	T4CONbits.TON = 0; //Disable timer
	T4CONbits.TSIDL = 0; //Continue operation in idle mode
	T4CONbits.TGATE = 0; //Timer gate accumulation disabled
	T4CONbits.TCKPS = 0b00; //Timer prescale 1:1, 1:8, 1:64, 1:256
	T4CONbits.T32 = 0; //32 bit timer disabled
	T4CONbits.TCS = 0; //Internal clock source
	
	//Resets after 0.001s  1ms
	PR4 = 125; //Period register
	
	TMR4=0;
	T4CONbits.TON = 1; //Enable timer4
	IEC1bits.T4IE = 1; //Enable timer4 interrupt
}

void __attribute__((__interrupt__, auto_psv)) _T4Interrupt( void )
{
	 IFS1bits.T4IF = 0; //Clear interrupt flag
    IEC1bits.T4IE = 0; //Disable timer4 interrupt
    millisCount++;
     IFS1bits.T4IF = 0; //Reset Timer1 interrupt flag and Return from ISR
     IEC1bits.T4IE = 1; //Enable timer1 interrupt
     //IFS1bits.INT2IF = 1;
}

INT32 millis()
{
  return millisCount;
}
