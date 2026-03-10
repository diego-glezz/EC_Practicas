#include <msp430.h>

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;

    PM5CTL0 &= ~LOCKLPM5;

    P1SEL0 &= ~BIT0;
    P1SEL1 &= ~BIT0;
    
    P9SEL0 &= ~BIT7;
    P9SEL1 &= ~BIT7;

    P1SEL0 &= ~BIT1;
    P1SEL1 &= ~BIT1;
    P1REN |= BIT1;
    P1DIR &= ~BIT1;
    P1OUT |= BIT1;

    int rojo = 1;

    P1DIR |= BIT0;
    P9DIR |= BIT7;

    P1OUT |= BIT0;
    P9OUT |= BIT7;

    while(1) {
        if ((P1IN & BIT1) == 0) rojo = (rojo == 1) ? 0 : 1;
        if (rojo) P1OUT ^= BIT0;
        P9OUT ^= BIT7;

        __delay_cycles(300000);
    }

    return 0;
}