#include <msp430.h>
#include <stdlib.h>
#include <string.h>

#define DIT 10000
#define DOT_TIME_LIMIT (2* DIT)
#define LETTER_END_TIME (3 * DIT)
#define SPACE_ADD_TIME (7 * DIT)
#define CHAR_COUNT 36
#define LETTER_COUNT 26
#define DIGIT_COUNT 10

#define INSTRUCTIONS_SIZE (27 + CHAR_COUNT * 10 + CHAR_COUNT / 4)

#define pos1 9   /* Digit A1 begins at S18 */
#define pos2 5   /* Digit A2 begins at S10 */
#define pos3 3   /* Digit A3 begins at S6  */
#define pos4 18  /* Digit A4 begins at S36 */
#define pos5 14  /* Digit A5 begins at S28 */
#define pos6 7   /* Digit A6 begins at S14 */

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

const char toChar[CHAR_COUNT] =
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

volatile int buffer[6] = {0, 1, 2, 3, 4, 5};

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

void init_button_config();
void init_LCD();
void init_leds();
void config_ACLK_to_32KHz_crystal();
void config_reloj_8MHz(void);
void config_UART_9600(void);
void init_timer0();
void init_timer1(int limit);

void add_letter();
void add_char(char** str, char c);
void delete_char(char** str);
void append_char(char* str, char c);
void clean_string(char** str);

void update_LCD();
void show_buffer(volatile int buffer[]);
//void shift_buffer(volatile int buffer[], int nueva_letra);
//void displayScrollText(char *msg);
//void showChar(char c, int position);

int get_char_index(char c);
void UART_print_char(char c);
void UART_print_string(char* str);
char* console_instructions();


char* curr_string = NULL;
char curr_morse[6] = "\0";

volatile int state = 0; //0 -> Waiting to start | 1-> Waiting for letter | 2 -> Mid letter

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;
    PM5CTL0 &= ~LOCKLPM5;

    init_button_config();
    
    init_LCD();
    init_leds();

    config_reloj_8MHz();
    //config_UART_9600();
    //config_ACLK_to_32KHz_crystal();

    show_buffer(buffer);

    curr_string = (char*)malloc(sizeof(char));
    (curr_string)[0] = '\0';

    //UART_print_string(console_instructions());

    __bis_SR_register(LPM0_bits |GIE);
    while (1) {}
}
/*
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
*/
// SIN ESPACIOS INFINITOS

#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void) {
    switch (state) {
        case 1: //No letter in a while so a space is added
            add_letter();
            TA1CTL &= ~MC_3;
            break;
        case 2: //No input in a while so the letter ended
            add_letter();
            state = 1;  //Waiting to start letter
            init_timer1(SPACE_ADD_TIME);
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
            if (strlen(curr_morse) < 5) {
                add_char((char**)&curr_morse, toAdd);
            }
            
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

    UCA1IE &= ~(UCRXIE | UCTXIE);
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

    char c = '\0';
    
    if (strlen(curr_morse) == 0) c = ' ';
    
    int i;

    
    for (i = LETTER_COUNT - 1; i >= 0 && c == '\0'; i--) {
        if (strcmp((char*)curr_morse, morse_letters[i]) == 0) {
            c = letterToChar[i];
            break;
        }
    }
    for (i = DIGIT_COUNT - 1; i >= 0 && c == '\0'; i--) {
        if (strcmp((char*)curr_morse, morse_digits[i]) == 0) {
            c = digitToChar[i];
            
            break;
        }
    }

    if (c == '\0') {
        P1OUT |= BIT0;
    }
    else {
        append_char((char**)&curr_string, c);
        P1OUT &= ~BIT0;
    }
    clean_string((char**)&curr_morse);
    update_LCD();
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
    unsigned int len = strlen(curr_morse);
    
    // Si hay menos de 5 caracteres, añadimos el nuevo y cerramos con nulo
    if (len < 5) {
        curr_morse[len] = c;
        curr_morse[len + 1] = '\0';
        update_LCD();
    }
}



/*
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
*/

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
/*
void shift_buffer(volatile int buffer[], int nueva_letra) {
    buffer[5] = buffer[4];
    buffer[4] = buffer[3];
    buffer[3] = buffer[2];
    buffer[2] = buffer[1];
    buffer[1] = buffer[0];
    buffer[0] = nueva_letra;
}

volatile unsigned char mode;

void displayScrollText(char *msg)
{
    int length = strlen(msg);
    int oldmode = mode;
    int i;
    int s = 5;
    char buffer[6] = "      ";
    for (i=0; i<length+7; i++)
    {
        if (mode != oldmode)
            break;
        int t;
        for (t=0; t<6; t++)
            buffer[t] = ' ';
        int j;
        for (j=0; j<length; j++)
        {
            if (((s+j) >= 0) && ((s+j) < 6))
                buffer[s+j] = msg[j];
        }
        s--;

        showChar(buffer[0], pos1);
        showChar(buffer[1], pos2);
        showChar(buffer[2], pos3);
        showChar(buffer[3], pos4);
        showChar(buffer[4], pos5);
        showChar(buffer[5], pos6);

        __delay_cycles(200000);
    }
}

void showChar(char c, int position)
{
    if (c == ' ')
    {
        // Display space
        LCDMEM[position] = 0;
        LCDMEM[position+1] = 0;
    }
    else if (c >= '0' && c <= '9')
    {
        // Display digit
        LCDMEM[position] = digit[c-48][0];
        LCDMEM[position+1] = digit[c-48][1];
    }
    else if (c >= 'A' && c <= 'Z')
    {
        // Display alphabet
        LCDMEM[position] = alphabetBig[c-65][0];
        LCDMEM[position+1] = alphabetBig[c-65][1];
    }
    else
    {
        // Turn all segments on if character is not a space, digit, or uppercase letter
        LCDMEM[position] = 0xFF;
        LCDMEM[position+1] = 0xFF;
    }
}
*/
int get_char_index(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= '0' && c <= '9') return c - '0' + 26;
    if (c == '.') return 37;
    if (c == '-') return 38;
    return 36; // Espacio por defecto
}

void UART_print_char(char c) {
    while (!(UCA1IFG & UCTXIFG)); 
    
    UCA1TXBUF = c;
}

void UART_print_string(char* str) {
    int i;
    for (i = 0; str[i] != '\0'; i++) {
        UART_print_char(str[i]);
    }
}

char* console_instructions() {
    static char instructions[INSTRUCTIONS_SIZE];
    
    strcpy(instructions, "--- DICCIONARIO MORSE ---\r\n");
    
    unsigned int i;

    for (i = 0; i < LETTER_COUNT; i++) {
        char morseStr[11];
        morseStr[10] = '\0';
        morseStr[0] = letterToChar[i];
        morseStr[1] = ':';
        morseStr[2] = ' ';
        morseStr[3] = '\0';

        strcat(morseStr, morse_letters[i]);
        while (strlen(morseStr) < 10) strcat(morseStr, " ");

        strcat(instructions, morseStr);
        if (i % 5 == 4 || toChar[i] == 'Z') strcat(instructions, "\r\n");
    }
    if (LETTER_COUNT % 5 != 0) {
        strcat(instructions, "\r\n");
    }
    
    for (i = 0; i < DIGIT_COUNT; i++) {
        char morseStr[11];
        morseStr[10] = '\0';
        morseStr[0] = digitToChar[i];
        morseStr[1] = ':';
        morseStr[2] = ' ';
        morseStr[3] = '\0';

        strcat(morseStr, morse_digits[i]);
        while (strlen(morseStr) < 10) strcat(morseStr, " ");

        strcat(instructions, morseStr);
        //if (i % 5 == 4 || toChar[i] == 'Z') strcat(instructions, "\r\n");
    }
    if (DIGIT_COUNT % 5 != 0) {
        strcat(instructions, "\r\n");
    }

    return instructions;
}
