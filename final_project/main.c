#include <msp430.h>
//need delay header file

unsigned int RxCount = 0;                   // Keeps track of how many bytes have been received
unsigned int nBytes = 0;                    // Stores number of bytes in current packet

P8.1
#define LEFTWHEEL BIT5                     //WAS RED
#define RIGHTWHEEL BIT6                     //WAS GREEN
#define TILTFORWARD BIT7                    //WAS BLUE
#define BUTTON BIT5
#define RELAY BIT8                         //relay for reverse functions

float hex2duty(int hex)
{
    float duty;
    duty = 1000-(hex/0xFF)*1000;
    return (int)duty;
}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;               // Stop Watchdog

    P3SEL0 |= LEFTWHEEL + RIGHTWHEEL + TILTFORWARD;
    P3DIR |= LEFTWHEEL + RIGHTWHEEL + TILTFORWARD;
    P3OUT &= ~(LEFTWHEEL + RIGHTWHEEL + TILTFORWARD);

    TB0CTL |=   TBSSEL_2 |ID_2 | MC_1;

    TB0CCR0 = 255;              // Set clock period ORIGINAL 255

    TB0CCR4 = 255;                      //left wheel
    TB0CCTL4 |= OUTMOD_7;       // Set/Reset mode

    TB0CCR5 = 255;                     //right wheel
    TB0CCTL5 |= OUTMOD_7;       // Set/Reset mode

    TB0CCR6 = 255;                      //tilt
    TB0CCTL6 |= OUTMOD_7;       // Set/Reset mode

    TB0CCR7 = 255;         //REVERSE FUNCTION START   relay
    TB0CCTL7 |= OUTMOD_7;



    // Configure GPIO
    P2SEL0 &= ~(BIT0 | BIT1);
    P2SEL1 |= BIT0 | BIT1;                  // USCI_A0 UART operation

    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configuLEFTWHEEL port settings
    PM5CTL0 &= ~LOCKLPM5;

    // Startup clock system with max DCO setting ~8MHz
    CSCTL0_H = CSKEY_H;                     // Unlock CS registers
    CSCTL1 = DCOFSEL_3 | DCORSEL;           // Set DCO to 8MHz
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;   // Set all dividers
    CSCTL0_H = 0;                           // Lock CS registers

    // Configure USCI_A0 for UART mode
    UCA0CTLW0 = UCSWRST;                    // Put eUSCI in reset
    UCA0CTLW0 |= UCSSEL__SMCLK;             // CLK = SMCLK
    // Baud Rate calculation
    // 8000000/(16*9600) = 52.083
    // Fractional portion = 0.083
    // User's Guide Table 21-4: UCBRSx = 0x04
    // UCBRFx = int ( (52.083-52)*16) = 1
    UCA0BRW = 52;                           // 8000000/16/9600
    UCA0MCTLW |= UCOS16 | UCBRF_1 | 0x4900;
    UCA0CTLW0 &= ~UCSWRST;                  // Initialize eUSCI
    UCA0IE |= UCRXIE;                       // Enable USCI_A0 RX interrupt

    __bis_SR_register(LPM3_bits | GIE);     // Enter LPM3, interrupts enabled
    __no_operation();                       // For debugger
}


#pragma vector=EUSCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
    //RxCount = 0;

    switch(__even_in_range(UCA0IV, USCI_UART_UCTXCPTIFG))
    {
        case USCI_NONE: break;
        case USCI_UART_UCRXIFG:
            while(!(UCA0IFG & UCTXIFG));
                if (RxCount == 0 ) {                 //start at bit segment 0 (0x08) and subtract 3 this is how many are being sent to the next processor
                    UCA0TXBUF = UCA0RXBUF - 4;   //subtract 4 bits left,right,tilt,relay
                    nBytes = UCA0RXBUF;

                    RxCount ++;                    //move on to the next segment
                }
                else if (RxCount == 1) {    // LEFTWHEEL info

                    TB0CCR4 = 255 - UCA0RXBUF;      //ff=255 =strength so ff is off and 00 is full duty cycle
                    RxCount ++;                   //go to RIGHTWHEEL info segment
                }

                else if (RxCount == 2) {    // RIGHTWHEEL info

                    TB0CCR5 = 255 - UCA0RXBUF;     //same as LEFTWHEEL 00 - full duty cycle
                    RxCount ++;                 //go to TILTFORWARD segment
                }
                delay(2000);                 //delay 2 seconds before tilt

                else if (RxCount == 3) {    // TILTFORWARD info

                    TB0CCR6 = 255 - UCA0RXBUF;      //same as LEFTWHEEL
                    RxCount ++;                    //go to next segment
                    delay (1000);

                    else if (RxCount == 4){         //TB6612FNG CHIP ACTIVATES SWITCHING DIRECTION OF MOTORS
                        TB0CCR7 = 255;
                        RxCount --;
                        delay(1000);          //ONE SECOND BEFORE REVERSE FUNCTION
                    }
                    else if (RxCount ==2){              //right wheel reverse function
                               TBOCCR5= 255 -UCA0RXBUF ;
                    RxCount --;
                }
                    else if (RxCount ==1){                 //left wheel reverse function
                        TB0CCR4 = 255 - UCA0RXBUF;
                        Rxcount ++;
                        Rxcount ++;
                        delay (1000);         //wait one second same as original movement time
                    }
                    else if (RxCount ==1){
                        TB0CCR4 = 0;        //stop wheel LEFT
                        RxCount ++;
                    }
                    else if (RxCount ==2){
                        TB0CCR5 =0;          //STOP WHEEL RIGHT
                        RxCount ++;
                    }
                    else if (RxCount ==3){
                        TB0CCR6 =0;         //release tilt
                        delay (10000);

                    }
                    else if (RxCount ==3){
                        TBOCCR6 == 0;


                }
                else if (RxCount > 3 & RxCount <= nBytes -1) {     // Pass these along after the first MOVEMENT segments have been seen
                if (!(RxCount == (nBytes-1))){                     //if segments is more than half of total segments then dont change anything
                    UCA0TXBUF = UCA0RXBUF;
                    RxCount ++;
                   //if (RxCount == nBytes)  // EOM   bad code we kept just in case
                   //     RxCount = 0;
                }
                else {
                    UCA0TXBUF = 0x0D;           //kill the segment
                    RxCount =0;
                    _delay_cycles(100000);      //wait

                }
                break;
                }
            __no_operation();
            break;
        case USCI_UART_UCTXIFG: break;
        case USCI_UART_UCSTTIFG: break;
        case USCI_UART_UCTXCPTIFG: break;
        default: break;
    }
}
