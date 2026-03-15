#include <msp430.h>

volatile char letra = 'A'; // Variable global para rastrear la letra actual a enviar

void config_reloj_8MHz(void);
void config_UART_9600(void);

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;
    PM5CTL0 &= ~LOCKLPM5;

    config_reloj_8MHz();      // Configurar el reloj principal a 8 MHz
    config_UART_9600();       // Configurar la UART a 9600 baudios

    // Habilitar la interrupción de transmisión de la UART
    UCA1IE |= UCTXIE; 

    // Entrar en modo de bajo consumo (LPM0) y habilitar interrupciones globales
    __bis_SR_register(LPM0_bits | GIE);
    
}

void config_reloj_8MHz(void) {
    CSCTL0_H = CSKEY >> 8;                    // Desbloquear registros de reloj
    CSCTL1 = DCOFSEL_3 | DCORSEL;             // Configurar DCO a 8MHz
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK; // Seleccionar fuentes
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;     // Establecer divisores a 1
    CSCTL0_H = 0;                             // Bloquear registros de reloj
}

void config_UART_9600(void) {
    // 1. Configurar los pines P3.4 (TX) y P3.5 (RX) para la función UART 
    P3SEL0 |= BIT4 | BIT5;
    P3SEL1 &= ~(BIT4 | BIT5);

    // 2. Mantener la UART en estado de reset por software para configurarla
    UCA1CTLW0 = UCSWRST; 
    
    // 3. Seleccionar SMCLK (8 MHz) como fuente de reloj
    // Nota: Por defecto (0 en los campos correspondientes) ya tiene 8 bits de datos, 1 bit stop y sin paridad 
    UCA1CTLW0 |= UCSSEL__SMCLK; 

    // 4. Configurar la tasa de baudios a 9600
    UCA1BR0 = 52; 
    UCA1BR1 = 0x00; 
    
    // 5. Configurar la modulación del canal UART
    UCA1MCTLW |= UCOS16 | UCBRF_1 | 0x4900; 
    
    // 6. Liberar el reset por software para que la UART comience a funcionar
    UCA1CTLW0 &= ~UCSWRST; 
}

#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void) {
    // Leer el vector de interrupción para determinar la causa
    switch(__even_in_range(UCA1IV, 0x08)) {
        case 0x00: break; // Sin interrupciones pendientes
        
        case 0x02: break; // Interrupción de recepción (UCRXIFG) - No utilizada aquí
        
        case 0x04: // Interrupción de transmisión (UCTXIFG)
            // El buffer de transmisión está vacío y listo para enviar un nuevo carácter
            UCA1TXBUF = letra; // Enviar la letra actual al buffer de transmisión
            
            letra++; // Incrementar al siguiente carácter ASCII
            
            // Si pasamos de la 'Z', reiniciar a la 'A' e insertar saltos de línea para el terminal
            if (letra > 'Z') {
                letra = 'A';
            }
            break;
            
        case 0x06: break; // Bit de inicio (UCSTTIFG)
        case 0x08: break; // Transmisión completa (UCTXCPTIFG)
        default: break;
    }
}