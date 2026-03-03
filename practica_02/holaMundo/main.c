#include <msp430.h>
#include <stdio.h>

int main(void) {
    WDTCTL = WDTPW | WDTHOLD; 

    PM5CTL0 &= ~LOCKLPM5;

    printf("Hola Mundo\n"); 

    while(1) {
    }
}