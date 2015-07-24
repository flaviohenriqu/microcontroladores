#include <p30f4011.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <stdarg.h>

/* Set APENAS UMA das definições abaixo como 1 */

#define XTx4PLL    0
#define XTx8PLL    1
#define XTx16PLL   0

#if  XTx4PLL
_FOSC(CSW_FSCM_OFF & XT_PLL4);
#define MDelay 2
#elif   XTx8PLL
_FOSC(CSW_FSCM_OFF & XT_PLL8);
#define MDelay 4
#elif   XTx16PLL
_FOSC(CSW_FSCM_OFF & XT_PLL16);
#define MDelay 8
#else
_FOSC(CSW_FSCM_OFF & XT);
#define MDelay 1
#endif

//============================================
_FWDT(WDT_OFF);
_FBORPOR(PBOR_ON & PWRT_OFF & MCLR_EN);
_FGS(CODE_PROT_OFF);
//============================================



#if !defined (TRUE)
  #define TRUE 1
#endif
#if !defined (FALSE)
  #define FALSE 0
#endif
#define NULL_CHAR '\0'

//#define min(x, y) ((x < y) ? x : y)
//#define max(x, y) ((x > y) ? x : y)

#define to_string(s) #s

typedef unsigned char int8;
typedef unsigned int int16;
typedef unsigned long int32;
typedef unsigned long long int64;

//******************* Variáveis Complexas ********************

#define _complex __complex__ // _complex float c;
#define real(x) __real__ x
#define imag(x) __imag__ x
#define complex_conj(x) ~x

//************************** Delay ***************************

void delay_cycles(int16 cycles){
   while(cycles != 0) {Nop(); cycles--;}
}

void delay_us(int16 micro){
   while(micro != 0) {delay_cycles(2*MDelay); micro--;}
}

void delay_ms(int16 mili){
   while(mili != 0) {delay_us(1000); mili--;}
}

//*********************** Display 7seg ***********************

void init_disp(void){
   TRISB = 0;
   TRISD = 0;
}

void select_disp(int8 d){
   int8 disp[5] = {0, 1, 2, 4, 8};
   LATD = disp[d];
}

void disp_putc(char c){
   switch (c){
      case '0': LATB = 0x3F; break;
      case '1': LATB = 0x06; break;
      case '2': LATB = 0x5B; break;
      case '3': LATB = 0x4F; break;
      case '4': LATB = 0x66; break;
      case '5': LATB = 0x6D; break;
      case '6': LATB = 0x7D; break;
      case '7': LATB = 0x07; break;
      case '8': LATB = 0x7F; break;
      case '9': LATB = 0x67; break;
      case '.': LATB += 0x80; break;
      default : LATB = 0x00; break;
   }
}

//************************ Keypad 4x3 ************************

void init_keypad(void){
   TRISF = 0xFFF8;
}

#define COL_0 0xF6
#define COL_1 0xF5
#define COL_2 0xF3

typedef struct keypad_row{
   unsigned      : 3;
   unsigned ROW_0   : 1;
   unsigned ROW_1   : 1;
   unsigned ROW_2   : 1;
   unsigned ROW_3   : 1;
   unsigned       : 9;
}row;

row *key = (row*)&PORTF;

#define kpad !(*key)

// LATF = 0xFF; TRISF = 0xFFFF;

char keypad_getc(void){

   LATF = COL_0;
   if(kpad.ROW_0) return '1';
   else if(kpad.ROW_1) return '4';
   else if(kpad.ROW_2) return '7';
   else if(kpad.ROW_3) return '*';

   LATF = COL_1;
   if(kpad.ROW_0) return '2';
   else if(kpad.ROW_1) return '5';
   else if(kpad.ROW_2) return '8';
   else if(kpad.ROW_3) return '0';

   LATF = COL_2;
   if(kpad.ROW_0) return '3';
   else if(kpad.ROW_1) return '6';
   else if(kpad.ROW_2) return '9';
   else if(kpad.ROW_3) return '#';

   return NULL_CHAR;
}

//*************************** LCD ****************************

#define CHAR_INF 0xF3
#define CHAR_SUM 0xF6
#define CHAR_PI 0xF7
#define CHAR_ALFA 0xE0
#define CHAR_BETA 0xE2
#define CHAR_EPSILON 0xE3
#define CHAR_MU 0xE4
#define CHAR_SIGMA 0xE5
#define CHAR_THETA 0xF2
#define CHAR_OHM 0xF4
#define CHAR_RHO 0xE6
#define CHAR_DIV 0xFD
#define CHAR_INV 0xE9
#define CHAR_SETA_D 0x7E
#define CHAR_SETA_E 0x7F
#define CHAR_PONTO 0xA5
#define CHAR_SQTR 0xE8
#define CHAR_MED 0xF8

typedef struct lcd_pin_map_data{
   unsigned data   : 4;
   unsigned       : 12;
}dados;

dados *d = (dados*)&LATB;

#define lcd_rs LATEbits.LATE4
#define lcd_en LATEbits.LATE5
#define lcd_data (*d).data

#define line_2 0x40

int8 const INIT[4] = {0x28, 0x0C, 0x01, 0x06};

void lcd_send_nibble(int8 data){
   lcd_data = data;
   delay_cycles(1);
   lcd_en = 1;
   delay_us(2);
   lcd_en = 0;
}

void lcd_send_byte(int8 add, int8 data){
   delay_ms(2);
   lcd_rs = add;
   delay_cycles(1);
   lcd_en = 0;
   lcd_send_nibble(data >> 4);
   lcd_send_nibble(data & 0x0F);
}

void init_lcd(){
   int8 i;
//   ADPCFG = 0xFE7F; //RB8 e RB7 analogicas
//   TRISB = 0x0000; 		// Porta B como saída
//   TRISBbits.TRISB8 = 1;	// AN8 - vai controlar o PWM da ventoinha
//   TRISBbits.TRISB7 = 1;	// AN7 - vai controlar o PWM dos leds
   TRISEbits.TRISE4 = 0;
   TRISEbits.TRISE5 = 0;
   lcd_rs = 0;
   delay_cycles(1);
   lcd_en = 0;
   delay_ms(15);
   for(i = 0; i < 3; i++){
      lcd_send_nibble(3);
      delay_ms(15);
   }
   lcd_send_nibble(2);
   for(i = 0; i < 4; i++) lcd_send_byte(0, INIT[i]);
}

void lcd_gotoxy(int8 x, int8 y){
   int8 add;
   if(y != 1) add = line_2;
   else add = 0;
   add += (x - 1);
   lcd_send_byte(0, 0x80|add);

}

// CGRAM

void lcd_CGRAM_write(int8 add, int16 s[]){
   int8 i;
   switch(add){
      case 0: lcd_send_byte(0, 0x40); break;
      case 1: lcd_send_byte(0, 0x48); break;
      case 2: lcd_send_byte(0, 0x50); break;
      case 3: lcd_send_byte(0, 0x58); break;
      case 4: lcd_send_byte(0, 0x60); break;
      case 5: lcd_send_byte(0, 0x68); break;
      case 6: lcd_send_byte(0, 0x70); break;
      case 7: lcd_send_byte(0, 0x78); break;
      default: break;
   }
   for(i = 0; i < 8; i++) lcd_send_byte(1, s[i]);
   lcd_send_byte(0, 0x80);
   //lcd_send_byte(0, 0x01);
}

void lcd_CGRAM_clear(int8 add){
   int16 clear[8] = {0, 0, 0, 0, 0, 0, 0, 0};
   lcd_CGRAM_write(add, clear);
}

void lcd_CGRAM_read(int8 add){
   lcd_send_byte(1, add);
}

void lcd_putc(char c){
   switch (c) {
      case '\f':
      lcd_send_byte(0, 1);
      delay_ms(2);
      break;
      case '\n':
      lcd_gotoxy(1, 2);
      break;
      case '\b':
      lcd_send_byte(0, 0x10);
      break;
      case '°':
      lcd_send_byte(1, 0xDF);
      break;
      default:
      lcd_send_byte(1, c);
      break;
   }
}

void lcd_puts(char *str){
   while(*str){
      lcd_putc(*str);
      str++;
   }
}

#define lcd_OFF 0x08
#define cursor_ON 0x0E
#define cursor_OFF 0x0C
#define cursor_HOME 0x02
#define cursor_PISCA 0x0D
#define cursor_ALT 0x0F
#define desloc_cursor_E 0x10
#define desloc_cursor_D 0x14
#define desloc_cursor_ent_E 0x04
#define desloc_cursor_ent_D 0x06
#define desloc_mensagem_E 0x18
#define desloc_mensagem_D 0x1C
#define desloc_mensagem_ent_E 0x07
#define desloc_mensagem_ent_D 0x05

void lcd_control(int8 mode){
   lcd_send_byte(0, mode);
}

void lcd_printf(const char *str, ...)
{
   va_list ap;
   va_start(ap, str);
   while(*str)
   {
      switch (*str){
         case '%':
         str++;
         if(*str == 'u')
         {
            char c[7] = "", i = 0;
            int16 u = va_arg(ap, unsigned int);

            if(u > 9999) i = 4;
            else if(u > 999) i = 3;
            else if(u > 99) i = 2;
            else if(u > 9) i = 1;
            else i = 0;

            while(u >= 10)
            {
               c[i] = u%10 + '0';
               u /= 10;
               i--;
            }
            c[i] = u + '0';

            lcd_puts(c);
         }
         else if(*str == 'd')
         {
            char c[9] = "", i = 0;
            int d = va_arg(ap, int);

            if(d < 0)
            {
               d = -d;
               c[0] = '-';
               i = 1;
            }

            if(d > 9999) i += 4;
            else if(d > 999) i += 3;
            else if(d > 99) i += 2;
            else if(d > 9) i += 1;

            while(d >= 10)
            {
               c[i] = d%10 + '0';
               d /= 10;
               i--;
            }
            c[i] = d + '0';
            lcd_puts(c);
         }
         else if(*str == 'f')
         {
            char c[20] = "", i = 0, j = 4;
            float fl = va_arg(ap, float);
            float frac, ip;
            unsigned int intpart, fracpart;

            if(fl < 0)
            {
               fl = -fl;
               c[0] = '-';
               i += 1;
            }

            frac = modf(fl, &ip);
            intpart = (unsigned int)ip;

            if(intpart > 9999) i += 4;
            else if(intpart > 999) i += 3;
            else if(intpart > 99) i += 2;
            else if(intpart > 9) i += 1;
            
            frac *= 1000;
            fracpart = (unsigned int)frac;
            
            c[i+1] = '.';
            while(j > 1)
            {
               c[i + j] = fracpart%10 + '0';
               fracpart /= 10;
               j--;
            }

            while(intpart >= 10)
            {
               c[i] = intpart%10 + '0';
               intpart /= 10;
               i--;
            }
            c[i] = intpart + '0';
            lcd_puts(c);
         }
         else
            lcd_putc(*str);
         break;
         default:
         lcd_putc(*str);
         break;
      }
      str++;
   }
   va_end(ap);
}

void lcd_clear(){
	lcd_gotoxy(1,1);
	lcd_printf("                \n                ");
	lcd_gotoxy(1,1);
}
