#include <p30f4011.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//*********** Função INIT_UART1(valor_baud) **********************
/*Funciona com uma freqüência de 8 Mhz de clock, e ativando as interrupções de Tx e Rx
valor_baud = INT[( (FCY/Desired Baud Rate)/16) – 1] onde INT = inteiro. */
void INIT_UART1 (int valor_baud) 
{
	U1BRG = valor_baud;
	/*Configuramos a UART, 8 bits de dados, 1 bit de parada, sem paridade */
	U1MODE = 0x0000; //Ver tabela para saber as outras configurações
	U1STA = 0x0000;
	IPC2 = 0x0440; //A faixa de prioridade média, não é urgente.
	IFS0bits.U1TXIF = 0; //Zerar o flag de interrupção de Tx.
	IEC0bits.U1TXIE = 0; //Habilita interrupção de Tx.
	IFS0bits.U1RXIF = 0; //Zerar o flag de interrupção de Rx.
	IEC0bits.U1RXIE = 1; //Habilita interrupção de Rx.
	U1MODEbits.USIDL = 1; // Idle
	U1MODEbits.UARTEN = 1; //E liga a UART
	U1STAbits.UTXEN = 1;
} // INIT_UART1

//*********** Interrupção para transmição ************************
void __attribute__((interrupt, no_auto_psv)) _U1TXInterrupt(void) 
{
	IFS0bits.U1TXIF = 0;
} //

//*********** Interrupção para recepção de caractere *************
// Variáveis que auxiliam a recepção de caractere
//unsigned char c_in; // caractere recebido
//unsigned char c_in_flag = 0; // flag de notificação
//void __attribute__((interrupt, no_auto_psv)) _U1RXInterrupt(void) 
//{
//	IFS0bits.U1RXIF = 0; // Limpa o flag de interrupção de TX
//	c_in = U1RXREG; // Lê o dado do buffer e o passa para a variável c_in
//	c_in_flag = 1; // flag para saber que aconteceu a interrupção.
//} // recepção
//

//************** Função OUTCHR_UART1(char c) *********************
void OUTCHR_UART1(unsigned char c) 
{
	while ( U1STAbits.UTXBF); // espera enquanto o buffer de Tx está cheio.
	U1TXREG = c; // escreve caractere.
} // OUTCHR_UART1

//************** Função PUTS_UART1(char *str) ********************
void PUTS_UART1(char *str) {
   while(*str){
      OUTCHR_UART1(*str);
      str++;
   }
} // PUTS_UART1

//************** Função PRINTF_UART1(const char *str, ...) *******
void PRINTF_UART1(const char *str, ...) 
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
            unsigned int u = va_arg(ap, unsigned int);

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

            PUTS_UART1(c);
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
            PUTS_UART1(c);
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
            PUTS_UART1(c);
         }
         else
            OUTCHR_UART1(*str);
         break;
         default:
         OUTCHR_UART1(*str);
         break;
      }
      str++;
   }
   va_end(ap);
} // PRINTF_UART1
