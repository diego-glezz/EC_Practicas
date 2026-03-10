#include <msp430.h>

int main(void) {

    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;
    
    PM5CTL0 &= ~LOCKLPM5;
    // Boton
    P1SEL0 &= ~BIT1;
    P1SEL1 &= ~BIT1;

    P1REN |= BIT1;
    P1DIR &= ~BIT1;
    P1OUT |= BIT1;

    // LED
    P1SEL0 &= ~BIT0;
    P1SEL1 &= ~BIT0;

    P1DIR |= BIT0;
    P1OUT &= ~BIT0;

    while(1) // Con polling
    {
        if ((P1IN & BIT1) == 0) {
            P1OUT |= BIT0;
        } else {
            P1OUT &= ~BIT0;
        }
    }
}
