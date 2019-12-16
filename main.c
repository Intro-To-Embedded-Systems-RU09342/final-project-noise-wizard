#include <msp430.h> 
#include <stdint.h>

#include <driverlib.h> // Required for the LCD
#include "myGpio.h" // Required for the LCD
#include "myClocks.h" // Required for the LCD
#include "myLcd.h" // Required for the LCD
/*
 By: Nicholas Riggins
 Date Started: 12/5/2019
 Date Finished: 12/16/2019

 Final Project LCD Display
 Goal: Show the volume, bass, treble and mid change in digipots outputs to the audio amplifier on an LCD Screen
       Signal is Transmitted through USB
       After 4 seconds of inactivity the volume level is shown as default
       Bass, treble and mid is at a range -4 to 4
       Volume range is 0 to 8


*/

const unsigned char lcd_num[10] = {
    0xFC,        // 0
    0x60,        // 1
    0xDB,        // 2
    0xF3,        // 3
    0x67,        // 4
    0xB7,        // 5
    0xBF,        // 6
    0xE4,        // 7
    0xFF,        // 8
    0xF7,        // 9
};

    int packager = 0;    // Used to identify if there is a package being sent
    char package [2]=""; // Identify what is being changed in the first item of the array and the second item is the number
    volatile int i = 1;  //location of current byte

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	PJSEL0 = BIT4 | BIT5;       // For LFXT


	// Initialize LCD segments 0 - 21; 26 - 43
	LCDCPCTL0 = 0xFFFF;
	LCDCPCTL1 = 0xFC3F;
	LCDCPCTL2 = 0x0FFF;

	// Disable the GPIO power-on default high-impedance mode
	// to activate previously configured port settings
	PM5CTL0 &= ~LOCKLPM5;

	// Configure LFXT 32kHz crystal
	CSCTL0_H = CSKEY >> 8;                  // Unlock CS registers
	CSCTL4 &= ~LFXTOFF;                     // Enable LFXT
	do
	{
	  CSCTL5 &= ~LFXTOFFG;                  // Clear LFXT fault flag
	  SFRIFG1 &= ~OFIFG;
	}
	while (SFRIFG1 & OFIFG);              // Test oscillator fault flag
	  CSCTL0_H = 0;                           // Lock CS registers

    int vol = 0;    // default Volume
    int bass = 0;   // default Bass
    int mid = 0;    // default Mid
    int treb =0;    // default Treble
    char Vo = 'V';     // Volume
    char B = 'B';     // Bass
    char M = 'M';     // Mid
    char T = 'T';     // Treble

    // Initialize LCD_C
    // ACLK, Divider = 1, Pre-divider = 16; 4-pin MUX
    LCDCCTL0 = LCDDIV__1 | LCDPRE__16 | LCD4MUX | LCDLP;

    // VLCD generated internally,
    // V2-V4 generated internally, v5 to ground
    // Set VLCD voltage to 2.60v
    // Enable charge pump and select internal reference for it
    LCDCVCTL = VLCD_1 | VLCDREF_0 | LCDCPEN;

    LCDCCPCTL = LCDCPCLKSYNC;               // Clock synchronization enabled

    LCDCMEMCTL = LCDCLRM;                   // Clear LCD memory



    initGPIO(); // Initializes General Purpose
    // Inputs and Outputs for LCD
    initClocks(); // Initialize clocks for LCD

    myLCD_init(); // Prepares LCD to receive commands


    UCA0CTL1 |= UCSSEL_2;                     // SMCLK
    UCA0BR0 = 104;                            // 1MHz 9600
    UCA0BR1 = 0;                              // 1MHz 9600
   // UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1
    UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
    UCA0IE = UCRXIE;                          //Enable interrupt

    P1SEL0 |= BIT1 + BIT2;
    P1SEL1 |= BIT1 + BIT2;

    __bis_SR_register(GIE);       // Enter LPM0, interrupts enabled

/*
    myLCD_showChar( 'T' , 1);      // Used during debugging
    myLCD_showChar( 'E' , 2);
    myLCD_showChar( 'S' , 3);
    myLCD_showChar( 'T' , 4);
*/
    while(1)
    {
        if(packager==1)
        {
            if(package[0] == Vo )
            {
                vol = package[1];

                myLCD_showChar( 'V', 1 ); // Display "V" in space 1
                myLCD_showChar( 'O', 2 ); // Display "O" in space 2
                myLCD_showChar( 'L', 3 ); // Display "L" in space 3
                                          // Space
                                          // Space
                myLCD_showChar( vol ,6);  // Show Volume Level
            }
            else if (package[0] == B)
            {
                bass = package[1];
                bass = bass - 4;

                myLCD_showChar( 'B', 1 ); // Display "B" in space 1
                myLCD_showChar( 'A', 2 ); // Display "A" in space 2
                myLCD_showChar( 'S', 3 ); // Display "S" in space 3
                                          // Space
                if(bass<0)
                {
                    bass *= -1;
                    myLCD_showChar( '-', 5); // Display "-" in space 4
                    myLCD_showChar( bass,6); // Show Negative Bass level
                }
                else
                {
                                              // Space
                    myLCD_showChar( bass ,6); // Show Bass Level
                }

            }
            else if (package[0] == T)
            {
                treb = package[1];
                treb = treb - 4;

                myLCD_showChar( 'T', 1 ); // Display "T" in space 1
                myLCD_showChar( 'R', 2 ); // Display "R" in space 2
                myLCD_showChar( 'E', 3 ); // Display "E" in space 3
                                          // Space
                if(treb<0)
                {

                    treb *= -1;
                    myLCD_showChar( '-', 5); // Display "-" in space 4
                    myLCD_showChar( treb,6); // Show Negative Treble level
                }
                else
                {
                                              // Space
                    myLCD_showChar( treb ,6); // Show Treble Level
                }
            }
            else if (package[0] == M)
            {
                mid = package[1];
                mid = mid -4;

                myLCD_showChar( 'M', 1 ); // Display "M" in space 1
                myLCD_showChar( 'I', 2 ); // Display "I" in space 2
                myLCD_showChar( 'D', 3 ); // Display "D" in space 3
                                          // Space
                if(mid<0)
                {
                    mid = mid * -1;

                    myLCD_showChar( '-', 5); // Display "-" in space 4
                    myLCD_showChar( mid,6); // Show Negative Middle level
                }
                else
                {
                                              // Space
                    myLCD_showChar( mid ,6); // Show Middle Level
                }
            }
            else
            {
                packager = 0;
                myLCD_showChar( 'E' , 1);
                myLCD_showChar( 'R' , 2);
                myLCD_showChar( 'R' , 3);
                myLCD_showChar( 'O' , 4);
                myLCD_showChar( 'R' , 5);
            }
        }
    }



	return 0;
}


   // void __attribute__ ((interrupt(USCIAB0RX_VECTOR))) USCI0RX_ISR (void)



//#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCIAB0RX_VECTOR
__interrupt void Port_1(void)
//#endif



/*
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector= PORT1_VECTOR
__interrupt void Port_1(void)
#elif defined(__GNUC__)
void__attribute__((interrupt(PORT1_VECTOR))) PORT_1(void)
#else
#error compliler not supported
#endif
*/

/*
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)

#endif
*/

    {

   // while (UCRXIFG2);        //Wait until a byte is ready, is USCI_A0 TX buffer ready?
           switch(4)
                   {
                    case 1:
                        memset(package,0,sizeof(package));        //Clear package
                        break;
                    case 2:
                        package[0] = UCA0RXBUF;                   // Set first bit of array
                                                                  // V = 56
                                                                  // B = 42
                                                                  // T = 54
                      /*                                          // M = 4D
                        if(UCA0RXBUF == 0x56)
                        {
                            package[0] = "V";
                        }

                        else if(UCA0RXBUF == 0x42)
                        {
                            package[0] = "B";
                        }
                        else if(UCA0RXBUF == 0x54)
                        {
                            package[0] = "T";
                        }
                        else if(UCA0RXBUF == 0x4D)
                        {
                            package[0] = "M";
                        }
                        else
                        {
                           package[0]= "E";
                        }
                        */

                        break;
                    case 3:
                        package[1] = UCA0RXBUF;                   //set second bit of array
                        break;
                    case 4:                                       // set packager

                        i=0;
                        if(UCA0RXBUF == 0x0D)                    // ends of bytes if its a valid signal
                        {
                            packager = 1;                        // means there is a signal for while loop in main
                        }
                        else
                        {
                            memset(package,0,sizeof(package));        //Clear package if not valid
                        }
                        break;
                    default:
                            memset(package,0,sizeof(package));        //Clear package
                        break;
                     }
        i++;

    }

