#include <msp430fr6989.h>

#define LED1    BIT0    // Defines the LED at P1.0
#define BUTTON  BIT1    // Defines the BUTTON at P1.1

/**
 * Nathan Sulzer
 * Into to Embedded Systems
 * main.c
 */

unsigned int CurrentValueY = 0;
unsigned int CurrentValueX = 0;

/*unsigned int ADCRead2(void)
{
    ADC12CTL0 |= ADC12ENC | ADC12SC;        //Start sampling/conversion
    while(!(ADC12IFGR2 & ADC12IFG2));
    return ADC12MEM2;
}

unsigned int ADCRead1(void)
{
    ADC12CTL0 &= ~(ADC12ENC + ADC12SC);
    ADC12CTL0 |= ADC12ENC + ADC12SC;        //Start sampling/conversion
    while(!(ADC12IFGR0 & ADC12IFG1));
    return ADC12MEM1;
}

unsigned int ADCRead0(void)
{
    ADC12CTL0 &= ~(ADC12ENC + ADC12SC);
    ADC12CTL0 |= ADC12ENC + ADC12SC;        //Start sampling/conversion
    while(!(ADC12IFGR0 & ADC12IFG0));
    return ADC12MEM0;
}*/
int UARTContrl = 1;
unsigned int control = 0x00;
void SteeringSet(void)
{
  switch(control)
  {
    case 0x0000 : //stop
        {
            P9OUT &= ~BIT4;
            P4OUT &= ~BIT7;
            break;
        }
    case 0x0001 : //Foward
        {
            P9OUT |= BIT4;
            P4OUT |= BIT7;
            P2OUT = 0x10;
            P3OUT = 0x01;
        break;
        }
    case 0x0002 : //Backwards
        {
            P9OUT |= BIT4;
            P4OUT |= BIT7;
            P2OUT = 0x08;
            P3OUT = 0x02;
        break;
        }
    case 0x0003 : //Right
        {
            P9OUT |= BIT4;
            P4OUT |= BIT7;
            P2OUT = 0x10;
            P3OUT = 0x02;
            break;
        }
    case 0x0004 : //Left
        {
            P9OUT |= BIT4;
            P4OUT |= BIT7;
            P2OUT = 0x08;
            P3OUT = 0x01;
            break;
        }
    case 0x0005 : //UartON
        {
         UARTContrl = 0x00;
         break;
        }
    case 0x0006 : //UartOff
        {
           UARTContrl = 0x01;
           break;
        }

  }
}

void SteeringSetAnalogXY(void)
{
    if(CurrentValueY >= 0x0F00)
    {
     P9OUT |= BIT4;
     P4OUT |= BIT7;
     P2OUT = 0x10;
     P3OUT = 0x01;

    } else if(CurrentValueY <= 0x0010)
    {
    P9OUT |= BIT4;
    P4OUT |= BIT7;
    P2OUT = 0x08;
    P3OUT = 0x02;

    } else if(CurrentValueX <= 0x0010)
    {
        P9OUT |= BIT4;
                    P4OUT |= BIT7;
                    P2OUT = 0x10;
                    P3OUT = 0x02;

    } else if(CurrentValueX >= 0x0F00)
    {
        P9OUT |= BIT4;
                    P4OUT |= BIT7;
                    P2OUT = 0x08;
                    P3OUT = 0x01;

    } else
    {
    P9OUT &= ~BIT4;
    P4OUT &= ~BIT7;
    }
}

void SteeringSetAnalogX(void)
{
    if(CurrentValueX >= 0x0F00)
    {
        P9OUT |= BIT4;
                    P4OUT |= BIT7;
                    P2OUT = 0x10;
                    P3OUT = 0x02;

    } else if(CurrentValueX <= 0x0010)
    {
        P9OUT |= BIT4;
                    P4OUT |= BIT7;
                    P2OUT = 0x08;
                    P3OUT = 0x01;

    } else
    {
    P9OUT &= ~BIT4;
    P4OUT &= ~BIT7;
    }
}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;             // stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;                 // Disable GPIO default high-impedance mode
    // GPIO Setup
    P8SEL1 |= BIT6;                         // Configure P8.6 for ADC
    P8SEL0 |= BIT6;
    P8SEL1 |= BIT5;                         // Configure P8.5 for ADC
    P8SEL0 |= BIT5;
    //P8SEL1 |= BIT4;                         // Configure P8.4 for ADC
    //P8SEL0 |= BIT4;
    P9DIR = BIT4;
    P9REN = BIT4;
    P9OUT = BIT4;
    P4DIR = BIT7;
    P4REN = BIT7;
    P4OUT = BIT7;
    P3DIR = BIT0 + BIT1;
    P3REN = BIT0 + BIT1;
    P2DIR = BIT3 + BIT4;                        // Set P1.0 LED to output
    P3REN = BIT0 + BIT1;                        // Enables resistor for button P1.1
    P3OUT = BIT0;                               // Makes resistor P1.1 a pull up
    P2REN = BIT3 + BIT4;
    P2OUT = BIT4;
    P2SEL0 |= BIT0 | BIT1;                    // Configure UART pins
    P2SEL1 &= ~(BIT0 | BIT1);
    // Configure ADC12
    ADC12CTL0 = ADC12SHT0_3 + ADC12ON;      // Sampling time, S&H=16, ADC12 on
    ADC12CTL1 = ADC12SHP + ADC12CONSEQ_3;                   // Use sampling timer
    ADC12CTL2 |= ADC12RES_2;                // 12-bit conversion results
    ADC12MCTL0 |= ADC12INCH_5;              // A5 ADC input select; Vref=AVCC
    ADC12MCTL1 |= ADC12INCH_6;              // A6 ADC input select; Vref=AVCC
    ADC12IER0 |= ADC12IE0 + ADC12IE1;
    //ADC12MCTL2 |= ADC12INCH_7;              // A7 ADC input select; Vref=AVCC
    // Startup clock system with max DCO setting ~8MHz
    CSCTL0_H = CSKEY >> 8;                    // Unlock clock registers
    CSCTL1 = DCOFSEL_3 | DCORSEL;             // Set DCO to 8MHz
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;     // Set all dividers
    CSCTL0_H = 0;                             // Lock CS registers
    // Configure USCI_A0 for UART mode
    UCA0CTLW0 = UCSWRST;                      // Put eUSCI in reset
    UCA0CTLW0 |= UCSSEL__SMCLK;               // CLK = SMCLK
    // Baud Rate calculation
    // 8000000/(16*9600) = 52.083
    // Fractional portion = 0.083
    // User's Guide Table 21-4: UCBRSx = 0x04
    // UCBRFx = int ( (52.083-52)*16) = 1
    UCA0BR0 = 52;                             // 8000000/16/9600
    UCA0BR1 = 0x00;
    UCA0MCTLW |= UCOS16 | UCBRF_1 | 0x4900;
    UCA0CTLW0 &= ~UCSWRST;                    // Initialize eUSCI
    UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
    //resistor enabled
P1REN |= BUTTON;

        //Sets Resistor to pullup, 1.0 is low, has to use = to initialize
P1OUT = BUTTON;

      //Enables port 1.0 as output, has to use = to initialize
P1IE |=  BUTTON;                           //enable interrupt on port 1.1
P1IES |= BUTTON;                            //sets as falling edge
P1IFG &=~(BUTTON);                     //clear interrupt flag
//SteeringSet();
    //enter LPM4 mode and enable global interrupt
_BIS_SR(GIE);
while(1){
while( UARTContrl == 1 ){
    __delay_cycles(5000);
    ADC12CTL0 |= ADC12ENC |ADC12SC;
    //CurrentValueX = ADCRead1();
    UCA0IE |= UCRXIE;
    SteeringSet();
    _BIS_SR( GIE);
    SteeringSetAnalogXY();
}
UCA0IE |= UCRXIE;
_BIS_SR( GIE);
}
}

               //Port 1 ISR
#pragma vector = PORT1_VECTOR
__interrupt void PORT_1(void)                        //double __
{
P9OUT ^= BIT4;
P4OUT ^= BIT7;
P1IFG &= ~BUTTON;                           // P1.1 IFG cleared
}


#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
    switch(__even_in_range(UCA0IV, USCI_UART_UCTXCPTIFG))
              {
                case USCI_NONE: break;
                case USCI_UART_UCRXIFG:
                  control = UCA0RXBUF;
                  SteeringSet();
                  break;
                case USCI_UART_UCTXIFG: break;
                case USCI_UART_UCSTTIFG: break;
                case USCI_UART_UCTXCPTIFG: break;
              }
}

//#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = ADC12_VECTOR
__interrupt void ADC12_ISR(void)
//#elif defined(__GNUC__)
//void __attribute__ ((interrupt(ADC12_VECTOR))) ADC12_ISR (void)
//#else
//#error Compiler not supported!
//#endif
{
  switch (__even_in_range(ADC12IV, ADC12IV_ADC12RDYIFG))
  {
    case ADC12IV_NONE:        break;        // Vector  0:  No interrupt
    case ADC12IV_ADC12OVIFG:  break;        // Vector  2:  ADC12MEMx Overflow
    case ADC12IV_ADC12TOVIFG: break;        // Vector  4:  Conversion time overflow
    case ADC12IV_ADC12HIIFG:  break;        // Vector  6:  ADC12BHI
    case ADC12IV_ADC12LOIFG:  break;        // Vector  8:  ADC12BLO
    case ADC12IV_ADC12INIFG:  break;        // Vector 10:  ADC12BIN
    case ADC12IV_ADC12IFG0:                 // Vector 12:  ADC12MEM0 Interrupt
        CurrentValueY = ADC12MEM0;
        SteeringSetAnalogXY();
        break;                                // Clear CPUOFF bit from 0(SR)
    case ADC12IV_ADC12IFG1:
        CurrentValueX = ADC12MEM1;
        SteeringSetAnalogXY();
        break;        // Vector 14:  ADC12MEM1
    case ADC12IV_ADC12IFG2:   break;        // Vector 16:  ADC12MEM2
    case ADC12IV_ADC12IFG3:   break;        // Vector 18:  ADC12MEM3
    case ADC12IV_ADC12IFG4:   break;        // Vector 20:  ADC12MEM4
    case ADC12IV_ADC12IFG5:   break;        // Vector 22:  ADC12MEM5
    case ADC12IV_ADC12IFG6:   break;        // Vector 24:  ADC12MEM6
    case ADC12IV_ADC12IFG7:   break;        // Vector 26:  ADC12MEM7
    case ADC12IV_ADC12IFG8:   break;        // Vector 28:  ADC12MEM8
    case ADC12IV_ADC12IFG9:   break;        // Vector 30:  ADC12MEM9
    case ADC12IV_ADC12IFG10:  break;        // Vector 32:  ADC12MEM10
    case ADC12IV_ADC12IFG11:  break;        // Vector 34:  ADC12MEM11
    case ADC12IV_ADC12IFG12:  break;        // Vector 36:  ADC12MEM12
    case ADC12IV_ADC12IFG13:  break;        // Vector 38:  ADC12MEM13
    case ADC12IV_ADC12IFG14:  break;        // Vector 40:  ADC12MEM14
    case ADC12IV_ADC12IFG15:  break;        // Vector 42:  ADC12MEM15
    case ADC12IV_ADC12IFG16:  break;        // Vector 44:  ADC12MEM16
    case ADC12IV_ADC12IFG17:  break;        // Vector 46:  ADC12MEM17
    case ADC12IV_ADC12IFG18:  break;        // Vector 48:  ADC12MEM18
    case ADC12IV_ADC12IFG19:  break;        // Vector 50:  ADC12MEM19
    case ADC12IV_ADC12IFG20:  break;        // Vector 52:  ADC12MEM20
    case ADC12IV_ADC12IFG21:  break;        // Vector 54:  ADC12MEM21
    case ADC12IV_ADC12IFG22:  break;        // Vector 56:  ADC12MEM22
    case ADC12IV_ADC12IFG23:  break;        // Vector 58:  ADC12MEM23
    case ADC12IV_ADC12IFG24:  break;        // Vector 60:  ADC12MEM24
    case ADC12IV_ADC12IFG25:  break;        // Vector 62:  ADC12MEM25
    case ADC12IV_ADC12IFG26:  break;        // Vector 64:  ADC12MEM26
    case ADC12IV_ADC12IFG27:  break;        // Vector 66:  ADC12MEM27
    case ADC12IV_ADC12IFG28:  break;        // Vector 68:  ADC12MEM28
    case ADC12IV_ADC12IFG29:  break;        // Vector 70:  ADC12MEM29
    case ADC12IV_ADC12IFG30:  break;        // Vector 72:  ADC12MEM30
    case ADC12IV_ADC12IFG31:  break;        // Vector 74:  ADC12MEM31
    case ADC12IV_ADC12RDYIFG: break;        // Vector 76:  ADC12RDY
    default: break;
  }
}
