#include <msp430.h>
#include <stdlib.h>
#include <string.h>

#define DOT_TIME_LIMIT 9830
#define LETTER_END_TIME (2 * DOT_TIME_LIMIT)
#define SPACE_ADD_TIME (3 * DOT_TIME_LIMIT)
#define CHAR_COUNT 37

const char* morse[CHAR_COUNT] = 
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
    "--.--",  // Ñ
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
    "--..",   // Z
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

const char toChar[CHAR_COUNT] = {
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
    'Ñ',
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
    'Z',
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

volatile int buffer[6] = {37, 37, 37, 37, 37, 37};

const char alphabetBig[40][2] =
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
    {0x6C, 0xC2}, /* 26: "Ñ" */
    {0xFC, 0x28}, /* 27: "0" */
    {0x60, 0x20}, /* 28: "1" */
    {0xDB, 0x00}, /* 29: "2" */
    {0xF3, 0x00}, /* 30: "3" */
    {0x67, 0x00}, /* 31: "4" */
    {0xB7, 0x00}, /* 32: "5" */
    {0xBF, 0x00}, /* 33: "6" */
    {0xE4, 0x00}, /* 34: "7" */
    {0xFF, 0x00}, /* 35: "8" */
    {0xF7, 0x00}, /* 36: "9" */
    {0x00, 0x00}, /* 37: " " (Espacio) */
    {0x00, 0x50}, /* 38: "." (Punto - segmento DP) */
    {0x02, 0x00}  /* 39: "-" (Raya - segmento G) */
};

void init_button_config(void);
void init_LCD();
void init_timer0(void);
void init_timer1(int limit);

void add_letter(void);
void add_char(char** str, char c);
void delete_char(char** str);
void append_char(char** str, char c);
void clean_string(char** str);

void update_LCD(void);
void show_buffer(volatile int buffer[]);
void shift_buffer(volatile int buffer[], int nueva_letra);

int get_char_index(char c);


char* curr_string = NULL;
char* curr_morse = NULL;

volatile int state = 0; //0 -> Waiting to start | 1-> Waiting for letter | 2 -> Mid letter

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;
    PM5CTL0 &= ~LOCKLPM5;

    UCA1IE |= UCRXIE;

    init_button_config();
    __bis_SR_register(LPM0_bits |GIE);
    init_LCD();

    while (1) {}
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void) {
    switch (state) {
        case 1: //No letter in a while so a space is added
            add_char((char**)&curr_string, ' ');
            break;
        case 2: //No input in a while so the letter ended
            add_letter();
            state = 1;  //Waiting to start letter
            break;
    }
    init_timer1(SPACE_ADD_TIME);
}
/* SIN ESPACIOS INFINITOS

#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void) {
    switch (state) {
        case 1: //No letter in a while so a space is added
            add_char((char**)&curr_string, ' ');
            TA1CTL &= ~MC_3;
            break;
        case 2: //No input in a while so the letter ended
            add_letter();
            state = 1;  //Waiting to start letter
            init_timer1(SPACE_ADD_TIME);
            break;
    }
}


*/

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
        P1IFG &= ~BIT1; // Limpia la bandera física del Botón 1
    }

    if (P1IFG & BIT2) { //Boton 2 -> delete o fin de texto
        delete_char((char**)&curr_string);

        init_timer1(SPACE_ADD_TIME);

        P1IFG &= ~BIT2; //Limpia la bandera física del Botón 2
    }
}

void init_button_config() {
    P1SEL0 &= ~(BIT1 | BIT2);
    P1SEL1 &= ~(BIT1 | BIT2);
    P1DIR &= ~(BIT1 | BIT2);
    P1REN |= (BIT1 | BIT2);
    P1OUT |= (BIT1 | BIT2); // Pull up
    P1IES |= (BIT1 | BIT2);
    P1IE |= (BIT1 | BIT2);  // Habilita la interrupcion de botones
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

void add_letter() {
    if (curr_morse == NULL) return;

    char letter = '\0';
    int i;
    for (i = CHAR_COUNT - 1; i >= 0; i--) {
        if (strcmp((char*)curr_morse, morse[i]) == 0) {
            letter = toChar[i];
            append_char((char**)&curr_string, letter);
            break;
        }
    }
    
    clean_string((char**)&curr_morse);
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

    if (curr_morse != NULL) len_morse = strlen((char*)curr_morse);
    if (curr_string != NULL) len_str = strlen((char*)curr_string);

    for (i = 0; i < 6; i++) {
        buffer[i] = 37;
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

void shift_buffer(volatile int buffer[], int nueva_letra) {
    buffer[5] = buffer[4];
    buffer[4] = buffer[3];
    buffer[3] = buffer[2];
    buffer[2] = buffer[1];
    buffer[1] = buffer[0];
    buffer[0] = nueva_letra;
}

int get_char_index(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= '0' && c <= '9') return c - '0' + 27;
    if (c == 'Ñ') return 26;
    if (c == '.') return 38;
    if (c == '-') return 39;
    return 37; // Espacio por defecto
}
