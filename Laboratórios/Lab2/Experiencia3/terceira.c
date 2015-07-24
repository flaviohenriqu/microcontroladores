#define FCY 16000000UL
#include <libpic30.h>
#include <p30f4011.h>
#include <math.h>
#include "display.h"

//============================================
_FOSC( CSW_FSCM_OFF & XT_PLL8); //A frequencia do XTAL é de 8 MHZ no kit, utilizamos PLL8.
_FWDT(WDT_OFF);
_FBORPOR(PBOR_OFF & PWRT_OFF & MCLR_EN);
_FGS(CODE_PROT_OFF);
//============================================
unsigned int est_ini=0, prox_est=0; //Estados da FSM

long int freq = 0;				//freq é um inteiro de 32 bits que receberá o TMR2/3

int x, y, w, z; //Variaveis para exibir o valor nos displays
long int k = 1; //Controle do frequencimetro Hz,kHz e MHz
int i = 0; //variavel para a criatividade

void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void)
{
	
	//Atribuir o valor do timer de 32 bits a variavel freq
	freq = TMR3;
	freq = (freq << 16) + TMR2;
	
	TMR2 = 0; //Zerar os timers
	TMR3 = 0;

	IFS0bits.T1IF = 0; //Zerar o flag da interrupção
}

void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void)
{			
	IFS0bits.T3IF = 0; //Zerar o flag da interrupção
}

void __attribute__ ((interrupt, no_auto_psv)) _INT0Interrupt(void)	//caso se pressione o botao de interrupção
{
	__delay_ms(50); //debounce
	
	est_ini = prox_est; //Proximo estado
	IFS0bits.INT0IF = 0; //Zerar o flag da interrupção
	TMR2 = 0; //zerar os timers
	TMR3 = 0;
}

void __criatividade(); //Assinatura da criatividade

//************* Programa Principal *************************
int main (void)
{
	ADPCFG = 0xFFFF; 		//configura a porta B (PORTB) como entradas/saidas digitais
	TRISB=0; 				//a PORTB como saída
	TRISD = 0;				//a PORTD como saída
	TRISC = 0x2000; 		// PORTC<13>=1; pino T2CK de entrada
	IFS0=0; //Flag de interrupção do timer1
	IEC0 = IEC0 | 0x0089; // bit 7 do registrador IEC0 habilita a interrupção do timer3, timer 1 e INT0
	INTCON2bits.INT0EP = 0; //borda de subida da INT0
	T1CON = 0x8030; //habilitar o timer 1 com prescaler 256
	T2CON =0x800A; //habilitar o timer de 32 bits
	
	PR1 = 62500; //1s				
	PR2 = 0xFFFF; //Máximo 
	PR3 = 0xFFFF;
	
	TMR3 = 0; //Zerar os timers
	TMR2 = 0;
	
	while(1) //loop infinito
    {  
   
	    z = (freq/k)%10; //unidade
  		w = ((freq/k)/10)%10; //dezena
 		y = ((freq/k)/100)%10; //centena
 		x = ((freq/k)/1000)%10; //milhar
	    __display(x,y,w,z); //exibir valor no display

		__criatividade(); //criatividade em notação ex.: 1000 Hz = 1E3 Hz
		//FMS
		 switch (est_ini){
			case 0: prox_est = 1; //Próximo estado
					k = 1; //Hz
					break;
			case 1: prox_est = 2; //Próximo estado
					k = 1000; //kHz
					break;
			case 2: prox_est = 0; //Próximo estado
					k = 1000000; //MHz
					break;
		 }
  
  }
}

void __criatividade()
{
	long int m = 0;
	
	while(!_RF0)
	{
		if(freq/k > 999)
		{
			x = 'x';
			w = 'E';
			m = freq/k;
			while(m>=10)
			{
				m = m/10;
				i++;
			}
			y = m;
			z = i;
		}
		__display(x,y,w,z);
		i = 0;
	}
}
