#include <msp430.h>

int main(void) {

    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // Desbloquear los pines
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

    // Configurar interrupcion del pin
    P1IES |= BIT1;
    P1IFG &= ~BIT1;
    P1IE |= BIT1;

    // Activa el GIE para dar paso a la interrupcion
    __bis_SR_register(LPM4_bits | GIE);
}

#pragma vector=PORT1_VECTOR 
__interrupt void Port_1(void) {
    // Comprobar si la interrucpcion corresponde al boton
    if (P1IFG & BIT1) {

        P1OUT ^= BIT0;
        // Desactivar la interrupcion
        P1IFG &= ~BIT1;
    }
}