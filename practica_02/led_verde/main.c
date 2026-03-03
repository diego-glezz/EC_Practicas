#include <msp430.h>

int main(void) {
    
    WDTCTL = WDTPW | WDTHOLD;

    P9SEL0 &= ~BIT7;
    P9SEL1 &= ~BIT7;

    P9DIR |= BIT7;

    PM5CTL0 &= ~LOCKLPM5;

    while(1) {
        P9OUT ^= BIT7;
        __delay_cycles(30000); 
    }

    return 0;
}