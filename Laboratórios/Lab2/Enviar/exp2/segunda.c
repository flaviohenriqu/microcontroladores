#define FCY	16000000UL
#include <libpic30.h>
#include <p30f4011.h>
#include "display.h"


//============================================
_FOSC( CSW_FSCM_OFF & XT_PLL8); //A frequencia do XTAL é de 8 MHZ no kit, utilizamos PLL8.
_FWDT(WDT_OFF);
_FBORPOR(PBOR_ON & PWRT_OFF & MCLR_EN);

long int T = 0;
unsigned int voltas = 0;
int cont = 0, duty = 2, est_ini = 0, prox_est = 0, cria_ini = 0, cria_prox = 0;
int x,y,w,z;

//============================================
void __attribute__ ((interrupt, no_auto_psv)) _T1Interrupt(void)
{
	IFS0bits.T1IF = 0; //Zera o flag de interrupção
	if (_RE0 == 1) //Verifica o estado da saída da ventoinha
	{
		_LATE0 = 0; //Desabilita a ventoinha
		PR1 = T*(10-duty)*0.1; //Ajusta o PR1
	}
	else
	{
		_LATE0 = 1; //Habilita a ventoinha
		PR1 = T*(duty)*0.1; //Ajusta o PR1
	}

}

void __attribute__ ((interrupt, no_auto_psv)) _T2Interrupt(void)
{
	_T2IF = 0; //Zera o flag de interrupção
	voltas++; //Incrementa o número de voltas da ventoinha
}

void __attribute__ ((interrupt, no_auto_psv)) _T3Interrupt(void)
{
	_T3IF = 0; //Zera o flag de interrupção	
	z = voltas%10; //Unidade
   	w = (voltas/10)%10; //Dezena
   	y = (voltas/100)%10; //Centena
   	x = (voltas/1000)%10; //Milhar
   	voltas = 0; //Zera o numero de voltas
}

void __attribute__ ((interrupt, no_auto_psv)) _INT0Interrupt(void)
{
	__delay_ms(50); //debounce
	IFS0bits.INT0IF = 0; //Zera o flag de interrupção
	est_ini = prox_est; //Proximo estado
	T1CON = T1CON | 0x8000; //ativamos o timer2
	T3CON = T3CON | 0x8030; //ativamos o timer3
	TMR1 = 0;					//zera os timers
	TMR3 = 0;

}

void __attribute__ ((interrupt, no_auto_psv)) _INT1Interrupt(void)
{
	__delay_ms(50); //debounce
	_INT1IF = 0; //Zera o flag de interrupção
	cria_ini = cria_prox; //Próximo estado da criatividade
}
	
void __criatividade(); //Assinatura da criatividade

/************* Programa Principal *************************/
int main()
{
	ADPCFG = 0xFFFF; //configura a porta B (PORTB) como entradas/saidas digitais
	TRISB=0; //a PORTB como saída
	_TRISC13=1; //PORTC<13>=1; pino T2CK (infravermelho)de entrada
	_TRISC14=0; //Saída Latch
	TRISD=0x0001; //Saída Latch
	_TRISE0=0; //Saida para ventoinha
	IFS0=0; //Flag de interrupção do timer1
	IEC0 = IEC0 | 0x00C9; // bit 3 do registrador IEC0 habilita a interrupção do timer1 (IEC0bits.T1IE=1)
	IFS1 = 0; //Interrupção externa 1
	IEC1 = IEC1 | 0x0001; //Habilitando a interrupção externa 1
	IPC0 = 0x1002; //Prioridade 2 para a INT0
	PR1 = 6400; //O registrador de periodo PR1 é igual a 6400 20% duty cycle
	PR2 = 7; //total para completar um volta da ventoinha
	PR3 = 62500; //1s timer 3 prescaler 1:256
	T = 32000; //periodo para 500Hz
	T1CON=0x0000; //Prescaler fica em 1.
	T2CON=0x8002; //habilita o timer 2 com TCS
	T3CON=0x0030; //timer 3 prescaler 256
	TMR1 = 0; //Zerar os timers
	TMR3 = 0;

	while(1)//laço infinito
	{
	    __display(x,y,w,z); //Exibir o número de voltas/segundo
		__criatividade(); //Alterar a frequência para checarmos a diferença no número de volta

		//FMS
		switch(est_ini)
		{
			case 0 : prox_est = 1; //Próximo estado
					 duty = 2; //Duty Cycle 20%
					 break;
			case 1 : prox_est = 2; //Próximo estado
					 duty = 8; //Duty Cycle 80%
					 break;
			case 2 : prox_est = 0; //Próximo estado
					 duty = 5; //Duty Cycle 50%
					 break;
		}

	}
}

void __criatividade()
{
	//FMS criatividade
	switch(cria_ini)
	{
		case 0: cria_prox = 1; //Próximo estado
				T = 32000; //500Hz
				break;
		case 1: cria_prox = 2; //Próximo estado
				T = 16000; //1kHz
				break;
		case 2: cria_prox = 0; //Próximo estado
				T = 64000; //250Hz
				break;
	}
}
