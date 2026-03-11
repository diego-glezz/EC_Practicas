#include <msp430.h>

volatile unsigned int contador = 0; 
volatile int corriendo = 1;
volatile int estado_led = 1;

const unsigned int LCD_Num[10] = {
    0xFC,
    0x60,
    0xDB,
    0xF3,
    0x67,
    0xB7,
    0xBF,
    0xE4,
    0xFF,
    0xF7
};

void Initialize_LCD(void);
void display_num_lcd(unsigned int n);
void config_ACLK_to_32KHz_crystal(void);

int main(void) {

    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD; 

    // Desbloquear los pines
    PM5CTL0 &= ~LOCKLPM5;

    // Inicializar LCD
    config_ACLK_to_32KHz_crystal();
    Initialize_LCD();
    display_num_lcd(contador);

    // Configurar los pines asociados a los botones
    P1SEL0 &= ~(BIT1 | BIT2);
    P1SEL1 &= ~(BIT1 | BIT2);
    P1DIR &= ~(BIT1 | BIT2);
    P1REN |= (BIT1 | BIT2);
    P1OUT |= (BIT1 | BIT2);

    // Configurar interrupciones de los pines
    P1IES |= (BIT1 | BIT2);
    P1IFG &= ~(BIT1 | BIT2);
    P1IE |= (BIT1 | BIT2);

    TA0CCR0 = 40000;
    TA0CCTL0 |= CCIE;
    
    TA0CTL = TASSEL_1 | MC_1 | TACLR; 

    P1SEL0 &= ~BIT0;
    P1SEL1 &= ~BIT0;
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;
        
    TA1CCR0 = 10000; // lo ponemos al tiempo que dura encendido

    TA1CCTL0 |= CCIE; 

    TA1CTL = TASSEL_1 | MC_1 | TACLR; 
    
    __bis_SR_register(LPM4_bits | GIE);
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_A0_ISR(void) {
    contador++;
    display_num_lcd(contador);
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void Timer1_A0_ISR(void) {
    if (estado_led == 1) {
        // Si estaba encendido, lo apagamos y asignamos el tiempo largo
        P1OUT &= ~BIT0;
        TA1CCR0 = 30000;
        estado_led = 0;
    } else {
        // Si estaba apagado, lo encendemos y asignamos el tiempo corto
        P1OUT |= BIT0;
        TA1CCR0 = 10000;
        estado_led = 1;
    }
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1_ISR(void) {
    
    // Si la interrupción viene del Botón 1
    if (P1IFG & BIT1) {
        if (corriendo == 1) {
            TA0CTL &= ~(MC_1 | MC_2 | MC_3);
            corriendo = 0;
        } else {
            TA0CTL |= MC_1;
            corriendo = 1;
        }
        P1IFG &= ~BIT1;
    }
    
    if (P1IFG & BIT2) {
        contador = 0;
        display_num_lcd(contador);
        
        TA0CTL |= TACLR; 
        
        P1IFG &= ~BIT2;
    }
}

//**********************************************************
// Initializes the LCD_C module
// *** Source: Function obtained from MSP430FR6989’s Sample Code ***
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

//***************function that displays any 16-bit unsigned integer************
//***************function that displays any 16-bit unsigned integer************
void display_num_lcd(unsigned int n) {
    LCDM8 = LCD_Num[n % 10]; 
    n = n / 10;

    if (n > 0) {
        LCDM15 = LCD_Num[n % 10];
        n = n / 10;
    } else {
        LCDM15 = 0;
    }

    if (n > 0) {
        LCDM19 = LCD_Num[n % 10];
        n = n / 10;
    } else {
        LCDM19 = 0;
    }

    if (n > 0) {
        LCDM4 = LCD_Num[n % 10];
        n = n / 10;
    } else {
        LCDM4 = 0;
    }

    if (n > 0) {
        LCDM6 = LCD_Num[n % 10];
        n = n / 10;
    } else {
        LCDM6 = 0;
    }
}

//**********************************
// Configures ACLK to 32 KHz crystal
void config_ACLK_to_32KHz_crystal() {

    // By default, ACLK runs on LFMODCLK at 5MHz/128 = 39 KHz
    // Reroute pins to LFXIN/LFXOUT functionality
    PJSEL1 &= ~BIT4;
    PJSEL0 |= BIT4;
    // Wait until the oscillator fault flags remain cleared
    CSCTL0 = CSKEY; // Unlock CS registers

    do {
        CSCTL5 &= ~LFXTOFFG; // Local fault flag
        SFRIFG1 &= ~OFIFG; // Global fault flag
    } while ((CSCTL5 & LFXTOFFG) != 0);

    CSCTL0_H = 0; // Lock CS registers

    return;
}
