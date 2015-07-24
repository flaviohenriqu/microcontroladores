#define FCY	16000000UL
#include <libpic30.h>
#include <p30f4011.h>
#include "display.h"


//============================================
_FOSC( CSW_FSCM_OFF & XT_PLL8); //A frequencia do XTAL é de 8 MHZ no kit, utilizamos PLL8.
_FWDT(WDT_OFF);
_FBORPOR(PBOR_ON & PWRT_OFF & MCLR_EN);

long int T = 0;			      // período do sinal em número de instruções
int duty = 3, est_ini = 0, prox_est = 0, j = 0; //duty cycle, estados, contador

//============================================
void __attribute__ ((interrupt, no_auto_psv)) _T1Interrupt(void)
{
	IFS0bits.T1IF = 0; //Zerar o bit de interrupção
	if (_RE0 == 1) //Verifica qual o estado atual da saída da ventoinha
	{
		_LATE0 = 0; //Zera o estado
		PR1 = T*(10-duty)*0.1; //Ajusta o valor do PR1 para o duty selecionado
	}
	else
	{
		_LATE0 = 1; //Ativa a saída da ventoinha
		PR1 = T*(duty)*0.1; //Ajusta o valor do PR1
	}

}

void __attribute__ ((interrupt, no_auto_psv)) _INT0Interrupt(void)
{
	__delay_ms(50); //debounce
	IFS0bits.INT0IF = 0; //Zerar o bit de interrupção
	est_ini = prox_est; //Mudança de estado
}
	

/************* Programa Principal *************************/
int main()
{
	ADPCFG = 0xFFFF; //configura a porta B (PORTB) como entradas/saidas digitais
	TRISB=0; //a PORTB como saída
	TRISD = 0x0001;				//a PORTD como saída
	TRISC=0; //Saída para o Latch
	_TRISE0 = 0;		//a PORTE como saída da ventoinha
	IFS0=0; //Flag de interrupção do timer1
	INTCON2bits.INT0EP = 0; //Borda de subida int0
	IEC0 = IEC0 | 0x0009; // bit 3 do registrador IEC0 habilita a interrupção do timer1 (IEC0bits.T1IE=1)
	IPC0 = 0x1002; //Prioridade 1 para o timer 1 e 2 para a int0
	PR1 = 24000; //O registrador de periodo PR1 é igual a 24000 equivalente a 30% do período total
	T = 80000; //Período total
	T1CON=0x8000; //ativamos o timer1 e o Prescaler fica em 1.
	
	int temp; //variavel temporaria
	while(1)//laço infinito
	{
	 	int frase[8] = {'S','P','E','E','d','x',est_ini+1,'x'}; //Frase a ser exibida na criatividade
     	while (!_RD0) //Botão pressionado
		{
			for (j = 0;j<120;j++) //Loop para exibir o valor no display
				 __display(frase[0],frase[1],frase[2],frase[3]);
						
			//deslocamento da frase no display
			temp = frase[0];
			for (j = 0; j<7;j++)
				 frase[j] = frase[j+1];
			frase[7] = temp;		
		}
				 
     	__display('x','x',duty,0);//valor do duty cycle a ser exibido no display

		//FMS
		switch(est_ini)
		{
			case 0 : prox_est = 1; //Proximo estado
					 duty = 3; //duty cycle de 30%
					 break;
			case 1 : prox_est = 2; //Proximo estado
					 duty = 8; //duty cycle de 80%
					 break;
			case 2 : prox_est = 0; //Proximo estado
					 duty = 5; //duty cycle de 50%
					 break;
		}
	}
}
