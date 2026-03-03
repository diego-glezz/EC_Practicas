#include <msp430.h>

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;

    PM5CTL0 &= ~LOCKLPM5;

    P1SEL0 &= ~BIT0;
    P1SEL1 &= ~BIT0;
    
    P9SEL0 &= ~BIT7;
    P9SEL1 &= ~BIT7;

    P1DIR |= BIT0;
    P9DIR |= BIT7;

    P1OUT |= BIT0;
    P9OUT |= BIT7;
    
    while(1) {
        P1OUT ^= BIT0;
        P9OUT ^= BIT7;

        __delay_cycles(3000000); 
    }

    return 0;
}