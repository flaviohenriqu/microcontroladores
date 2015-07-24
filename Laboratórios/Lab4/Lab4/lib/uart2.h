#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//=============================================================== //*********** Função INIT_UART2(valor_baud) **********************
 /*Funciona com uma freqüência de 8 MHz, e ativando as interrupções  de Tx e Rx  valor_baud = INT[( (FCY/Desired Baud Rate)/16) – 1]   onde INT = inteiro. */
void INIT_UART2 (valor_baud) { 
	U2BRG = valor_baud; 
	/*Configuramos a UART, 8 bits de dados, 1 bit de parada,  sem paridade */
	U2MODE = 0x0000; //Ver tabela para saber as outras configurações 
	U2STA = 0x0000; 
	IPC6 = 0x0044; //A faixa de prioridade média, não é urgente. 
	_U2TXIF = 0; //Zerar o flag de interrupção de Tx. 
	_U2TXIE = 0; //Habilita interrupção de Tx. 
	_U2RXIF = 0; //Zerar o flag de interrupção de Rx. 
	_U2RXIE = 1; //Habilita interrupção de Rx. 
	U2MODEbits.USIDL = 1; //Idle
	U2MODEbits.UARTEN = 1; //E liga a UART 
	U2STAbits.UTXEN = 1; 
}

void OUTCHR_UART2(unsigned char c) { 
	while ( U2STAbits.UTXBF); // espera enquanto o buffer de Tx está cheio.  
	U2TXREG = c; // escreve caractere. 
}

void PUTS_UART2(unsigned char *s)
{
	while(*s){
		OUTCHR_UART2(*s); // envia o caractere e pronto para o próximo
		s++;
	}
}

//************** Função INCHR_UART2(char c) ********************
unsigned char INCHR_UART2()
{
	unsigned char c;
	while (!U2STAbits.URXDA); // espera enquanto o buffer de Rx está VAZIO.
	c = U2RXREG ; // recebe caractere.
	return c;
}

//************** Função PRINTF_UART2(const char *str, ...) *******
void PRINTF_UART2(const char *str, ...) 
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

            PUTS_UART2(c);
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
            PUTS_UART2(c);
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
            PUTS_UART2(c);
         }
         else
            OUTCHR_UART2(*str);
         break;
         default:
         OUTCHR_UART2(*str);
         break;
      }
      str++;
   }
   va_end(ap);
} // PRINTF_UART2
