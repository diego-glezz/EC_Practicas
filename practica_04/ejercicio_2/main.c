#include <msp430.h>

volatile int buffer[6] = {65, 65, 65, 65, 65, 65}; 

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
void config_UART(void);
void Initialize_LCD(void);
void config_ACLK_to_32KHz_crystal();
void ShowBuffer(volatile int buffer[]);
void ShiftBuffer(volatile int buffer[], int nueva_letra);

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;
    PM5CTL0 &= ~LOCKLPM5;

    // Configuraciones iniciales
    config_reloj_8MHz();
    config_UART();
    config_ACLK_to_32KHz_crystal();
    Initialize_LCD();
    
    // Mostramos el estado inicial del buffer (AAAAAA)
    ShowBuffer(buffer);

    // Ahora habilitamos la interrupción de RECEPCIÓN (UCRXIE) [cite: 68-70, 457]
    UCA1IE |= UCRXIE; 

    // Dormimos la CPU y activamos las interrupciones globales
    __bis_SR_register(LPM0_bits | GIE); 

    return 0;
}

void ShiftBuffer(volatile int buffer[], int nueva_letra) {
    buffer[5] = buffer[4];
    buffer[4] = buffer[3];
    buffer[3] = buffer[2];
    buffer[2] = buffer[1];
    buffer[1] = buffer[0];
    buffer[0] = nueva_letra;
}

void ShowBuffer(volatile int buffer[]) {
    LCDMEM[9]  = alphabetBig[(buffer[0])-65][0];
    LCDMEM[10] = alphabetBig[(buffer[0])-65][1];
    
    LCDMEM[5]  = alphabetBig[(buffer[1])-65][0];
    LCDMEM[6]  = alphabetBig[(buffer[1])-65][1];
    
    LCDMEM[3]  = alphabetBig[(buffer[2])-65][0];
    LCDMEM[4]  = alphabetBig[(buffer[2])-65][1];
    
    LCDMEM[18] = alphabetBig[(buffer[3])-65][0];
    LCDMEM[19] = alphabetBig[(buffer[3])-65][1];
    
    LCDMEM[14] = alphabetBig[(buffer[4])-65][0];
    LCDMEM[15] = alphabetBig[(buffer[4])-65][1];
    
    LCDMEM[7]  = alphabetBig[(buffer[5])-65][0];
    LCDMEM[8]  = alphabetBig[(buffer[5])-65][1];
}

void config_reloj_8MHz(void) {
    CSCTL0_H = CSKEY >> 8;                    
    CSCTL1 = DCOFSEL_3 | DCORSEL;             
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK; 
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;     
    CSCTL0_H = 0;                             
}

void config_UART(void) {
    P3SEL0 |= (BIT4 | BIT5);                  
    P3SEL1 &= ~(BIT4 | BIT5);                 

    UCA1CTLW0 = UCSWRST;                      
    UCA1CTLW0 |= UCSSEL__SMCLK;               
    
    UCA1BR0 = 52;                             
    UCA1BR1 = 0x00;                           
    UCA1MCTLW |= UCOS16 | UCBRF_1 | 0x4900;   

    UCA1CTLW0 &= ~UCSWRST;                    
}

#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void) {

    char letra_recibida;
    
    // Comprobamos qué evento ha disparado la interrupción [cite: 698-716]
    switch(__even_in_range(UCA1IV, 0x08)) {
        
        case 0x00: break;
        
        // --- CASO 0x02: INTERRUPCIÓN DE RECEPCIÓN (UCRXIFG) ---
        // Se ejecuta cuando ha llegado un carácter completo desde el PC [cite: 74, 485, 702-704]
        case 0x02: 
            
            // 1. Leemos el dato del buzón. (Al leerlo, el flag se limpia solo) 
            letra_recibida = UCA1RXBUF; 
            
            // 2. Filtramos para asegurarnos de que solo aceptamos letras MAYÚSCULAS [cite: 109]
            if (letra_recibida >= 'A' && letra_recibida <= 'Z') {
                
                // 3. Desplazamos las letras viejas y metemos la nueva
                ShiftBuffer(buffer, letra_recibida);
                
                // 4. Actualizamos la pantalla LCD
                ShowBuffer(buffer);
            }
            break;
            
        case 0x04: break; // Transmisión (No la usamos aquí)
        case 0x06: break; // Start bit
        case 0x08: break; // Transmisión completa
        default: break;
    }
}

void Initialize_LCD() {
    PJSEL0 = BIT4 | BIT5; // For LFXT
    // Initialize LCD segments 0 - 21; 26 - 43
    LCDCPCTL0 = 0xFFFF;
    LCDCPCTL1 = 0xFC3F;
    LCDCPCTL2 = 0x0FFF;
    // Configure LFXT 32kHz crystal
    CSCTL0_H = CSKEY >> 8; // Unlock CS registers
    CSCTL4 &= ~LFXTOFF; // Enable LFXT
    
    do {
        CSCTL5 &= ~LFXTOFFG; // Clear LFXT fault flag
        SFRIFG1 &= ~OFIFG;
    } while (SFRIFG1 & OFIFG); // Test oscillator fault flag
    
    CSCTL0_H = 0; // Lock CS registers
    // Initialize LCD_C
    // ACLK, Divider = 1, Pre-divider = 16; 4-pin MUX
    LCDCCTL0 = LCDDIV__1 | LCDPRE__16 | LCD4MUX | LCDLP;
    // VLCD generated internally,
    // V2-V4 generated internally, v5 to ground
    // Set VLCD voltage to 2.60v
    // Enable charge pump and select internal reference for it
    LCDCVCTL = VLCD_1 | VLCDREF_0 | LCDCPEN;
    LCDCCPCTL = LCDCPCLKSYNC; // Clock synchronization enabled
    LCDCMEMCTL = LCDCLRM; // Clear LCD memory
    //Turn LCD on
    LCDCCTL0 |= LCDON;

    return;
}

void config_ACLK_to_32KHz_crystal() {
   PJSEL1 &= ~BIT4;
   PJSEL0 |= BIT4;
   CSCTL0 = CSKEY;
   do {
       CSCTL5 &= ~LFXTOFFG;
       SFRIFG1 &= ~OFIFG;
   } while ((CSCTL5 & LFXTOFFG) != 0);
   CSCTL0_H = 0;
}