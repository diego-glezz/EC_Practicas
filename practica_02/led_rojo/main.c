#include <msp430.h>

int main(void) {
    volatile int i;

    WDTCTL = WDTPW | WDTHOLD;

    P1SEL0 &= ~BIT0;
    P1SEL1 &= ~BIT0;

    P1DIR |= BIT0;

    PM5CTL0 &= ~LOCKLPM5;

    while(1) {
        P1OUT ^= BIT0;
        __delay_cycles(30000); 
    }

    return 0;
}
