#include <msp430.h>
#include <stdlib.h>
#include <string.h>

#define DIT 5000
#define DOT_TIME_LIMIT (2* DIT)
#define LETTER_END_TIME (3 * DIT)
#define SPACE_ADD_TIME (7 * DIT)
#define LETTER_COUNT 26
#define DIGIT_COUNT 10
#define SCROLL_TEXT_TIME 10000

#define INSTRUCTIONS_SIZE 600   

#define pos1 9   /* Digit A1 begins at S18 */
#define pos2 5   /* Digit A2 begins at S10 */
#define pos3 3   /* Digit A3 begins at S6  */
#define pos4 18  /* Digit A4 begins at S36 */
#define pos5 14  /* Digit A5 begins at S28 */
#define pos6 7   /* Digit A6 begins at S14 */

const char* morse_letters[LETTER_COUNT] =
{
    ".-",     // A
    "-...",   // B
    "-.-.",   // C
    "-..",    // D
    ".",      // E
    "..-.",   // F
    "--.",    // G
    "....",   // H
    "..",     // I
    ".---",   // J
    "-.-",    // K
    ".-..",   // L
    "--",     // M
    "-.",     // N
    "---",    // O
    ".--.",   // P
    "--.-",   // Q
    ".-.",    // R
    "...",    // S
    "-",      // T
    "..-",    // U
    "...-",   // V
    ".--",    // W
    "-..-",   // X
    "-.--",   // Y
    "--.."   // Z
};

const char* morse_digits[DIGIT_COUNT] =
{
    "-----",  // 0
    ".----",  // 1
    "..---",  // 2
    "...--",  // 3
    "....-",  // 4
    ".....",  // 5
    "-....",  // 6
    "--...",  // 7
    "---..",  // 8
    "----."   // 9
};

const char letterToChar[LETTER_COUNT] =
{
    'A',
    'B',
    'C',
    'D',
    'E',
    'F',
    'G',
    'H',
    'I',
    'J',
    'K',
    'L',
    'M',
    'N',
    'O',
    'P',
    'Q',
    'R',
    'S',
    'T',
    'U',
    'V',
    'W',
    'X',
    'Y',
    'Z'
};

const char digitToChar[DIGIT_COUNT] =
{
    '0',
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9'
};

volatile int buffer[6] = {36, 36, 36, 36, 36, 36};

const char alphabetBig[39][2] =
{
    {0xEF, 0x00}, /* 0: "A" */
    {0xF1, 0x50}, /* 1: "B" */
    {0x9C, 0x00}, /* 2: "C" */
    {0xF0, 0x50}, /* 3: "D" */
    {0x9F, 0x00}, /* 4: "E" */
    {0x8F, 0x00}, /* 5: "F" */
    {0xBD, 0x00}, /* 6: "G" */
    {0x6F, 0x00}, /* 7: "H" */
    {0x90, 0x50}, /* 8: "I" */
    {0x78, 0x00}, /* 9: "J" */
    {0x0E, 0x22}, /* 10: "K" */
    {0x1C, 0x00}, /* 11: "L" */
    {0x6C, 0xA0}, /* 12: "M" */
    {0x6C, 0x82}, /* 13: "N" */
    {0xFC, 0x00}, /* 14: "O" */
    {0xCF, 0x00}, /* 15: "P" */
    {0xFC, 0x02}, /* 16: "Q" */
    {0xCF, 0x02}, /* 17: "R" */
    {0xB7, 0x00}, /* 18: "S" */
    {0x80, 0x50}, /* 19: "T" */
    {0x7C, 0x00}, /* 20: "U" */
    {0x0C, 0x28}, /* 21: "V" */
    {0x6C, 0x0A}, /* 22: "W" */
    {0x00, 0xAA}, /* 23: "X" */
    {0x00, 0xB0}, /* 24: "Y" */
    {0x90, 0x28}, /* 25: "Z" */
    {0xFC, 0x28}, /* 26: "0" */
    {0x60, 0x20}, /* 27: "1" */
    {0xDB, 0x00}, /* 28: "2" */
    {0xF3, 0x00}, /* 29: "3" */
    {0x67, 0x00}, /* 30: "4" */
    {0xB7, 0x00}, /* 31: "5" */
    {0xBF, 0x00}, /* 32: "6" */
    {0xE4, 0x00}, /* 33: "7" */
    {0xFF, 0x00}, /* 34: "8" */
    {0xF7, 0x00}, /* 35: "9" */
    {0x00, 0x00}, /* 36: " " (Espacio) */
    {0x00, 0x50}, /* 37: "|" (Punto) */
    {0x02, 0x00}  /* 38: "-" (Raya) */
};

void init_button_config(void);
void init_LCD();
void init_leds();
void config_ACLK_to_32KHz_crystal();
void init_timer0(void);
void init_timer1(int limit);

int add_letter(void);
void add_char(char** str, char c);
void delete_char(char** str);
void append_char(char** str, char c);
void clean_string(char** str);

void update_LCD(void);
void show_buffer(volatile int buffer[]);

int get_lcd_char(char c);

void scrollText(char *msg);

void config_reloj_8MHz(void);
void config_UART_9600(void);
void uart_print(char *str);
char* console_instructions();


char* curr_string = NULL;
char* curr_morse = NULL;

volatile int state = 0; //0 -> Waiting to start | 1-> Waiting for letter | 2 -> Mid letter

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;
    PM5CTL0 &= ~LOCKLPM5;

    init_button_config();
    
    config_reloj_8MHz();
    config_UART_9600();

    init_LCD();
    init_leds();
    //config_ACLK_to_32KHz_crystal();

    show_buffer(buffer);

    curr_string = (char*)malloc(sizeof(char));
    (curr_string)[0] = '\0';

    curr_morse = (char*)malloc(sizeof(char));
    (curr_morse)[0] = '\0';


    uart_print(console_instructions());

    __bis_SR_register(LPM0_bits |GIE);
    while (1) {}
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void) {
    switch (state) {
        case 1: // No hay letra en un rato asi que se añade un espacio
            add_char((char**)&curr_string, ' ');
            TA1CTL &= ~MC_3;
            break;
        case 2: // No hay input(. o -) en un rato asi que la letra ha terminado
            if (add_letter() == 1) {
                init_timer1(SPACE_ADD_TIME);
                state = 1;
            }
            else state = 0;
            
            break;
    }
}


#pragma vector=PORT1_VECTOR
__interrupt void ISR_Puerto1(void) {
    
    if (P1IFG & BIT1) { //Boton 1 -> letra
        if (P1IES & BIT1) { // Boton pulsado
            state = 2;
            P1IES &= ~BIT1; // Cambiar a flanco de subida(interrupcion al soltar el boton)
            TA1CTL &= ~MC_3;
            init_timer0();
        }
        else {              // Boton soltado
            TA0CTL &= ~MC_3;

            unsigned int time = TA0R;

            char toAdd = (time <= DOT_TIME_LIMIT) ? '.' : '-';

            add_char((char**)&curr_morse, toAdd);

            P1IES |= BIT1; // Cambia a flanco de bajada

            init_timer1(LETTER_END_TIME);
        }
        
        P1IFG &= ~BIT1;
    }

    if (P1IFG & BIT2) { //Boton 2 -> delete o fin de texto
        if (P1IES & BIT2) { // Boton 2 pulsado
            P1IES &= ~BIT2; // Cambiar a flanco de subida
            TB0CTL = TBSSEL__ACLK | TBCLR | MC__CONTINUOUS;
        }
        else { // Boton 2 soltado
            TB0CTL &= ~MC_3;
            int time = TA0R;

            if (time < SCROLL_TEXT_TIME) { // Borrar letra
                delete_char((char**)&curr_string);
                state = 0;
                TA1CTL &= ~MC_3;
            } else {// Scroll del texto
                scroll_text((char*)curr_string);
            }

            P1IES |= BIT2;
        }
        P1IFG &= ~BIT2;
    }
}

void init_button_config() {
    P1SEL0 &= ~(BIT1 | BIT2);
    P1SEL1 &= ~(BIT1 | BIT2);
    P1DIR &= ~(BIT1 | BIT2);
    P1REN |= (BIT1 | BIT2);
    P1OUT |= (BIT1 | BIT2); 
    
    P1IES |= (BIT1 | BIT2); // 1. Configuras el flanco (esto genera el flag fantasma)
    
    P1IFG &= ~(BIT1 | BIT2); // 2. LIMPIAS el flag (apagas el timbre antes de empezar)
    
    P1IE |= (BIT1 | BIT2);   // 3. Habilitamos la interrupción
}

void init_LCD() {
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

void init_leds() {
    P1SEL0 &= ~BIT0;
    P1SEL1 &= ~BIT0;
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;

    P9SEL0 &= ~BIT7;
    P9SEL1 &= ~BIT7;
    P9DIR |= BIT7;
    P9OUT &= ~BIT7;
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

void init_timer0() {
    TA0CTL = TASSEL__ACLK | TACLR;
    TA0CTL |= MC__CONTINUOUS;
}

void init_timer1(int limit) {
    TA1CTL = TASSEL__ACLK | TACLR;
    TA1CCR0 = limit;
    TA1CCTL0 = CCIE;
    TA1CTL |= MC__UP;
}

int add_letter() {
    if (curr_morse == NULL) return 0;

    char letter = '\0';
    int i;
    for (i = LETTER_COUNT - 1; i >= 0 && letter == '\0'; i--) {
        if (strcmp((char*)curr_morse, morse_letters[i]) == 0) {
            letter = letterToChar[i];
            append_char((char**)&curr_string, letter);
            break;
        }
    }
    for (i = DIGIT_COUNT - 1; i >= 0 && letter == '\0'; i--) {
        if (strcmp((char*)curr_morse, morse_digits[i]) == 0) {
            letter = digitToChar[i];
            append_char((char**)&curr_string, letter);
            break;
        }
    }

    if (letter == '\0') {
        P1OUT |= BIT0;
        clean_string((char**)&curr_morse);
        update_LCD();
        return 0;
    }
    else {
        P1OUT &= ~BIT0;
        clean_string((char**)&curr_morse);
        update_LCD();
        return 1;
    }
    
}

void add_char(char** str, char c) {
    append_char(str, c);
}

void delete_char(char** str) {
    if (*str != NULL) {
        unsigned int len = strlen(*str);
        
        if (len > 0) {
            *str = (char*)realloc(*str, len * sizeof(char));
            
            if (*str != NULL) {
                (*str)[len - 1] = '\0';
            }
        }
    }

    update_LCD();
}

void append_char(char** str, char c) {
    if (*str == NULL) {
        *str = (char*)malloc(2 * sizeof(char));
        (*str)[0] = c;
        (*str)[1] = '\0';
        return;
    }

    unsigned int len = strlen(*str);
    
    if (len == 0) {
        *str = (char*)realloc(*str, 2 * sizeof(char));
    }
    else *str = (char*)realloc(*str, (len + 2) * sizeof(char));
    
    (*str)[len] = c;
    (*str)[len + 1] = '\0';

    update_LCD();
}

void clean_string(char** str) {
    if (*str != NULL) {
        *str = (char*)realloc(*str, 1);
        (*str)[0] = '\0';
    }
}

void update_LCD() {
    int i;
    unsigned int len_morse = 0;
    unsigned int len_str = 0;

    if (curr_morse != NULL) len_morse = (int)strlen((char*)curr_morse);
    if (curr_string != NULL) len_str = (int)strlen((char*)curr_string);

    for (i = 0; i < 6; i++) {
        buffer[i] = 36;
    }
    
    int index = 0;
    for (i = len_morse - 1; i >= 0 && index < 6; i--) {
        buffer[index++] = get_char_index(((char*)curr_morse)[i]);
    }

    for (i = len_str - 1; i >= 0 && index < 6; i--) {
        buffer[index++] = get_char_index(((char*)curr_string)[i]);
    }

    show_buffer(buffer);
}

void show_buffer(volatile int buffer[]) {
    LCDMEM[9]  = alphabetBig[buffer[5]][0];
    LCDMEM[10] = alphabetBig[buffer[5]][1];
    LCDMEM[5]  = alphabetBig[buffer[4]][0];
    LCDMEM[6]  = alphabetBig[buffer[4]][1];
    LCDMEM[3]  = alphabetBig[buffer[3]][0];
    LCDMEM[4]  = alphabetBig[buffer[3]][1];
    LCDMEM[18] = alphabetBig[buffer[2]][0];
    LCDMEM[19] = alphabetBig[buffer[2]][1];
    LCDMEM[14] = alphabetBig[buffer[1]][0];
    LCDMEM[15] = alphabetBig[buffer[1]][1];
    LCDMEM[7]  = alphabetBig[buffer[0]][0];
    LCDMEM[8]  = alphabetBig[buffer[0]][1];
}

int get_char_index(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= '0' && c <= '9') return c - '0' + 26;
    if (c == '.') return 37;
    if (c == '-') return 38;
    return 36; // Espacio por defecto
}

void scroll_text(char* str) {
    if (str == NULL || strlen(str) == 0) return;
    
    int len = strlen(str);
    int i, j;

    for (i = 0; i <= len + 5; i++) { 
        
        for (j = 0; j < 6; j++) {
            int str_idx = i - 5 + j;
            
            if (str_idx >= 0 && str_idx < len) {
                buffer[5 - j] = get_char_index(str[str_idx]);
            } else {
                buffer[5 - j] = 36; 
            }
        }
        
        show_buffer(buffer);
        
        __delay_cycles(200000); 
    }
    
    update_LCD(); 
}


void config_reloj_8MHz(void) {
    CSCTL0_H = CSKEY >> 8;                    
    CSCTL1 = DCOFSEL_3 | DCORSEL;             
    CSCTL2 = SELA_2 | SELS__DCOCLK | SELM__DCOCLK; 
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;     
    CSCTL0_H = 0;                             
}

void config_UART_9600(void) {
    // 1. VOLVEMOS A LOS PINES CORRECTOS DE LA LAUNCHPAD (P3.4 y P3.5)
    P3SEL0 |= (BIT4 | BIT5);
    P3SEL1 &= ~(BIT4 | BIT5);

    // 2. CONFIGURAMOS EL MÓDULO UCA1
    UCA1CTLW0 = UCSWRST;                      
    UCA1CTLW0 |= UCSSEL__SMCLK;               // Reloj a 8MHz
    
    // Configuración matemática para 9600 baudios
    UCA1BR0 = 52;                             
    UCA1BR1 = 0x00;                           
    UCA1MCTLW = UCOS16 | UCBRF_1 | 0x4900;    

    UCA1CTLW0 &= ~UCSWRST;
    UCA1IE &= ~(UCRXIE | UCTXIE);             // Interrupciones apagadas
}

void uart_print(char *str) {
    while (*str) {
        while ((UCA1IFG & UCTXIFG) == 0); 
        UCA1TXBUF = *str++;
    }
}


char* console_instructions() {
    static char instructions[INSTRUCTIONS_SIZE];
    
    strcpy(instructions, "\r\n--- DICCIONARIO MORSE ---\r\n");
    
    unsigned int i;

    for (i = 0; i < LETTER_COUNT; i++) {
        char morseStr[12];
        morseStr[10] = '\0';
        morseStr[0] = letterToChar[i];
        morseStr[1] = ':';
        morseStr[2] = ' ';
        morseStr[3] = '\0';

        strcat(morseStr, morse_letters[i]);
        while (strlen(morseStr) < 10) strcat(morseStr, " ");

        strcat(instructions, morseStr);
        if (i % 5 == 4 || letterToChar[i] == 'Z') strcat(instructions, "\r\n");
    }
    if (LETTER_COUNT % 5 != 0) {
        strcat(instructions, "\r\n");
    }
    
    for (i = 0; i < DIGIT_COUNT; i++) {
        char morseStr[12];
        morseStr[10] = '\0';
        morseStr[0] = digitToChar[i];
        morseStr[1] = ':';
        morseStr[2] = ' ';
        morseStr[3] = '\0';

        strcat(morseStr, morse_digits[i]);
        while (strlen(morseStr) < 10) strcat(morseStr, " ");

        strcat(instructions, morseStr);
        if (i % 5 == 4 || digitToChar[i] == '9') strcat(instructions, "\r\n");
    }
    if (DIGIT_COUNT % 5 != 0) {
        strcat(instructions, "\r\n");
    }

    return instructions;
}
