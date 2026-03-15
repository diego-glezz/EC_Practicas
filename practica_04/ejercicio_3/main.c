
#include <msp430.h>
const char alphabetBig[26][2] =
{
{0xEF, 0x00}, /* "A" LCD segments a+b+c+e+f+g+m */
{0xF1, 0x50}, /* "B" */
{0x9C, 0x00}, /* "C" */
{0xF0, 0x50}, /* "D" */
{0x9F, 0x00}, /* "E" */
{0x8F, 0x00}, /* "F" */
{0xBD, 0x00}, /* "G" */
{0x6F, 0x00}, /* "H" */
{0x90, 0x50}, /* "I" */
{0x78, 0x00}, /* "J" */
{0x0E, 0x22}, /* "K" */
{0x1C, 0x00}, /* "L" */
{0x6C, 0xA0}, /* "M" */
{0x6C, 0x82}, /* "N" */
{0xFC, 0x00}, /* "O" */
{0xCF, 0x00}, /* "P" */
{0xFC, 0x02}, /* "Q" */
{0xCF, 0x02}, /* "R" */
{0xB7, 0x00}, /* "S" */
{0x80, 0x50}, /* "T" */
{0x7C, 0x00}, /* "U" */
{0x0C, 0x28}, /* "V" */
{0x6C, 0x0A}, /* "W" */
{0x00, 0xAA}, /* "X" */
{0x00, 0xB0}, /* "Y" */
{0x90, 0x28} /* "Z" */
};

void ShowBuffer(volatile unsigned int buffer[]) {
LCDMEM[9] = alphabetBig[(buffer[5])-65][0];
LCDMEM[10] = alphabetBig[(buffer[5])-65][1];
LCDMEM[5] = alphabetBig[(buffer[4])-65][0];
LCDMEM[6] = alphabetBig[(buffer[4])-65][1];
LCDMEM[3] = alphabetBig[(buffer[3])-65][0];
LCDMEM[4] = alphabetBig[(buffer[3])-65][1];
LCDMEM[18] = alphabetBig[(buffer[2])-65][0];
LCDMEM[19] = alphabetBig[(buffer[2])-65][1];
LCDMEM[14] = alphabetBig[(buffer[1])-65][0];
LCDMEM[15] = alphabetBig[(buffer[1])-65][1];
LCDMEM[7] = alphabetBig[(buffer[0])-65][0];
LCDMEM[8] = alphabetBig[(buffer[0])-65][1];
}
void Initialize_LCD() {
   PJSEL0 = BIT4 | BIT5;
   LCDCPCTL0 = 0xFFFF;
   LCDCPCTL1 = 0xFC3F;
   LCDCPCTL2 = 0x0FFF;

   CSCTL0_H = CSKEY >> 8;
   CSCTL4 &= ~LFXTOFF;
   do {
       CSCTL5 &= ~LFXTOFFG;
       SFRIFG1 &= ~OFIFG;
   } while (SFRIFG1 & OFIFG);
   CSCTL0_H = 0;

   LCDCCTL0 = LCDDIV__1 | LCDPRE__16 | LCD4MUX | LCDLP;
   LCDCVCTL = VLCD_1 | VLCDREF_0 | LCDCPEN;
   LCDCCPCTL = LCDCPCLKSYNC;
   LCDCMEMCTL = LCDCLRM;

   LCDCCTL0 |= LCDON;
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

volatile char letter = 'A';
volatile unsigned char tx_open = 0;
#define BUFFER_SIZE 6  // Tamaño del buffer, según ShowBuffer()
volatile unsigned int rxBuffer[BUFFER_SIZE];  // Buffer para almacenar los datos convertidos
volatile  int index = BUFFER_SIZE-1;     // Índice del buffer
int main(void)
{
   WDTCTL = WDTPW | WDTHOLD;    // Stop watchdog timer

   // 1. DESBLOQUEAR PINES (Crucial en MSP430FR)
   PM5CTL0 &= ~LOCKLPM5;

   // Configuración de Reloj (8MHz)
   CSCTL0_H = CSKEY >> 8;
   CSCTL1 = DCOFSEL_3 | DCORSEL;
   CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
   CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;
   CSCTL0_H = 0;

   // Configurar pines UART (P3.4 = TX, P3.5 = RX)
   P3SEL0 |= BIT4 | BIT5;
   P3SEL1 &= ~(BIT4 | BIT5);

   //Configurar pantalla
      config_ACLK_to_32KHz_crystal();
      Initialize_LCD();

   // Configure USCI_A1 para UART
   UCA1CTLW0 = UCSWRST;                    // Reset
   UCA1CTLW0 |= UCSSEL__SMCLK;             // SMCLK
   UCA1BR0 = 52;                           // 9600 Baudios
   UCA1BR1 = 0x00;
   UCA1MCTLW |= UCOS16 | UCBRF_1 | 0x4900;
   UCA1CTLW0 &= ~UCSWRST;                  // Liberar Reset

   // Solo habilitamos RXIE. El TX lo manejaremos por flag o en la ISR
   UCA1IE |= UCRXIE | UCTXIE;

   // Configurar Timer A0
   TA0CCR0 = 50000;
   TA0CCTL0 = CCIE;
   TA0CTL = TASSEL_2 | MC_1 | ID_0;        // SMCLK, Up mode

   __bis_SR_register(LPM0_bits | GIE);     // Dormir y habilitar interrupciones
   __no_operation();
}

// Interrupción UART
#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void) {
   if (UCA1IFG & UCRXIFG) {  // Si hay un dato recibido
          char received = UCA1RXBUF; // Leer dato
          if(received >= 'A' & received <= 'Z'){
              rxBuffer[index] = received;
              ShowBuffer(rxBuffer);
              index--;
              if (index < 0) {  // Si el buffer está lleno
                              index = BUFFER_SIZE-1; // Reiniciar índice
            }
          }
      }

      if (UCA1IFG & UCTXIFG) {  // Si el buffer de transmisión está listo
         tx_open = 1;
      }
}

// ISR del Timer
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A0_ISR(void) {
   if(tx_open == 1){
       UCA1TXBUF = letter;            // Esto limpia automáticamente UCTXIFG
       if(letter >= 'Z') letter = 'A';
       else letter++;
       tx_open = 0;
   }
}