#include <msp430.h>

int main(void) {

    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD; 

    // Desbloquear los pines
    PM5CTL0 &= ~LOCKLPM5;

    // LED
    P1SEL0 &= ~BIT0;
    P1SEL1 &= ~BIT0;
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;
    
    // TA0CCR0 es el registro que guarda el "tope" de la cuenta. 
    // Le decimos que cuente hasta 40.000 como pide el ejercicio.
    TA0CCR0 = 40000; 

    // TA0CCTL0 es el registro de control de Captura/Comparación.
    // Habilitamos la interrupción (CCIE = Capture/Compare Interrupt Enable).
    TA0CCTL0 |= CCIE; 

    // TA0CTL es el registro de control general del Timer0.
    // TASSEL_2: Selecciona el reloj SMCLK (aprox 1 MHz por defecto).
    // MC_1: Selecciona el modo "Up" (cuenta hasta TA0CCR0 y se reinicia a 0).
    // TACLR: Limpia el contador actual para asegurarnos de que empieza desde 0.
    TA0CTL = TASSEL_2 | MC_1 | TACLR; 

    // Activa el GIE para dar paso a la interrupcion
    __bis_SR_register(LPM0_bits | GIE);
}

#pragma vector=TIMER0_A0_VECTOR 
__interrupt void Timer0_A0_ISR(void) {
    
    // Alternamos el estado del LED (parpadeo)
    P1OUT ^= BIT0;
}