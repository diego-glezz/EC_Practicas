#include <msp430.h>

volatile int buffer[6] = {65, 65, 65, 65, 65, 65}; 
volatile char letra = 'A';
volatile int stateLED = 1;

const char alphabetBig[26][2] = {
    {0xEF, 0x00}, /* "A" */  {0xF1, 0x50}, /* "B" */  {0x9C, 0x00}, /* "C" */
    {0xF0, 0x50}, /* "D" */  {0x9F, 0x00}, /* "E" */  {0x8F, 0x00}, /* "F" */
    {0xBD, 0x00}, /* "G" */  {0x6F, 0x00}, /* "H" */  {0x90, 0x50}, /* "I" */
    {0x78, 0x00}, /* "J" */  {0x0E, 0x22}, /* "K" */  {0x1C, 0x00}, /* "L" */
    {0x6C, 0xA0}, /* "M" */  {0x6C, 0x82}, /* "N" */  {0xFC, 0x00}, /* "O" */
    {0xCF, 0x00}, /* "P" */  {0xFC, 0x02}, /* "Q" */  {0xCF, 0x02}, /* "R" */
    {0x87, 0x00}, /* "S" */  {0x80, 0x50}, /* "T" */  {0x7C, 0x00}, /* "U" */
    {0x00, 0x28}, /* "V" */  {0x6C, 0x0A}, /* "W" */  {0x00, 0xAA}, /* "X" */
    {0x00, 0xB0}, /* "Y" */  {0x90, 0x28}  /* "Z" */
};

void config_reloj_8MHz(void);
void config_UART_9600(void);
void config_TimerA(void);

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;
    PM5CTL0 &= ~LOCKLPM5;

    // Configuraciones iniciales
    config_reloj_8MHz();
    config_UART_9600();
    config_TimerA();
    P1SEL0 &= ~BIT0;
    P1SEL1 &= ~BIT0;
    
    P1DIR |= BIT0;

    P1OUT |= BIT0;


    // Ahora habilitamos la interrupción de RECEPCIÓN (UCRXIE)
    UCA1IE |= UCRXIE; 

    // Dormimos la CPU y activamos las interrupciones globales
    __bis_SR_register(LPM0_bits | GIE); 

    return 0;
}


void config_reloj_8MHz(void) {
    CSCTL0_H = CSKEY >> 8;                    
    CSCTL1 = DCOFSEL_3 | DCORSEL;             
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK; 
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;     
    CSCTL0_H = 0;                             
}

void config_UART_9600(void) {
    P3SEL0 |= (BIT4 | BIT5);                  
    P3SEL1 &= ~(BIT4 | BIT5);                 

    UCA1CTLW0 = UCSWRST;                      
    UCA1CTLW0 |= UCSSEL__SMCLK;               
    
    UCA1BR0 = 52;                             
    UCA1BR1 = 0x00;                           
    UCA1MCTLW |= UCOS16 | UCBRF_1 | 0x4900;   

    UCA1CTLW0 &= ~UCSWRST;                    
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void) {
    // Despertamos a la UART para que envíe un carácter
    if (stateLED == 1) P1OUT ^= BIT0;
}


#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void) {

    char letra_recibida;
    
    // Comprobamos qué evento ha disparado la interrupción
    switch(__even_in_range(UCA1IV, 0x08)) {
        
        case 0x00: break;
        
        // Se ejecuta cuando ha llegado un carácter completo desde el PC
        case 0x02: 
            
            // 1. Leemos el dato del buzón. (Al leerlo, el flag se limpia solo) 
            letra_recibida = UCA1RXBUF; 
            
            // 2. Filtramos para asegurarnos de que solo aceptamos letras MAYÚSCULAS
            if (letra_recibida == 'A') {
                P1OUT &= ~BIT0;
                stateLED = 0;
            }
            else if (letra_recibida == 'E') {
                stateLED = 1;
            }

            
            break;
            
        case 0x04: break; 
        case 0x06: break; // Start bit
        case 0x08: break; // Transmisión completa
        default: break;
    }
}

void config_TimerA(void) {
    TA0CCR0 = 15000; 
    TA0CCTL0 = CCIE; // Habilitar interrupción de este temporizador
    TA0CTL = TASSEL__ACLK | MC__UP | TACLR; // Fuente ACLK, Modo UP, Limpiar contador
}
