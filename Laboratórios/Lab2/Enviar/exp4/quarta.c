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
long int T; //Período total

int y = -1, w = -1, z = -1; //Valor do duty cycle a ser exibido no display
int duty = 0; //duty cycle
int cont = -1; //variavel para o contador
int _size = 9; //limite do contador


void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void)
{
    if (_RE0 == 1) //verifica se a ventoinha esta habilitada
	{
		if (duty == 100) //se o duty for máximo
		{
			PR1 = 1; 
			_LATE0 = 1; //habilita a ventoinha
		}
		else
		{
			_LATE0 = 0; //desabilita a ventoinha
			PR1 = T*(100 - duty)*0.01; //ajusta o valor do PR1
		}
	}
	else
	{
		if (duty == 0) //se o duty for minímo
		{
			_LATE0 = 0; //desabilita a ventoinha
			PR1 = 1;
		}	
		else
		{
			_LATE0 = 1; //habilita a ventoinha
			PR1 = T*(duty)*0.01; //ajusta o valor do PR1
		}	
	}
    IFS0bits.T1IF = 0; //Zera o flag da interrupção

}

void __attribute__ ((interrupt, no_auto_psv)) _INT0Interrupt(void)	//caso se pressione o botao de interrupção
{
	__delay_ms(50); //debounce
	
	est_ini = prox_est; //próximo estado
	cont = -1; //retorna o contador para o inicio
	IFS0bits.INT0IF = 0; //Zera o flag da interrupção
}

void __attribute__ ((interrupt, no_auto_psv)) _INT1Interrupt(void)
{
	__delay_ms(50); //debounce
	_INT1IF = 0; //Zera o flag da interrupção
	cont = (cont < _size) ? cont + 1 : 0; //contador com limite
}

void __estados(int estado,int* flag); //Assinatura do metodo estados
void __criatividade(); //assinatura da criatividade

//************* Programa Principal *************************
int main (void)
{
	ADPCFG = 0xFFFF; 		//configura a porta B (PORTB) como entradas/saidas digitais
	TRISB = 0; 				//a PORTB como saída
	TRISD = 0x0001;				//a PORTD como saída
	_TRISE0 = 0;		//a PORTE como saída da ventoinha
	_TRISF0 = 1;       //Entrada para a criatividade
	TRISC = 0; //saída latch
	IFS0 = 0; //Flag de interrupção do timer1
	IFS1 = 0; //Flag da interrupção externa 1
	IEC1 = IEC1 | 0x0001; //Habilita a interrupção externa 1
	IEC0 = IEC0 | 0x0009; // bit 7 do registrador IEC0 habilita a interrupção do timer3, timer 1 e INT0
	_INT0EP = 0; //borda de subida INT0
	_INT1EP = 0; //borda de subida INT1
	T1CON = 0x8030; //habilita o timer 1 com prescaler 256
	
	PR1 = 6250;	
	T = 6250; //10Hz
	
	int ctr1 = 0, ctr2 = 0; //controle de entrada
	
	while(1) //loop infinito
    {  
		__criatividade(); //Criatividade exibi o valor médio da tensão do duty cycle

		//FMS
		 switch (est_ini){
			case 0: prox_est = 1; //Próximo estado
					break;
			case 1: prox_est = cont != -1 ? 2 : 1; //Próximo estado
					__estados(0,0); //Estado inicial
					break;
			case 2: prox_est =  cont != -1 ? 3 : 2; //Próximo estado
					__estados(1,&ctr1); //Estado 1
					break;
			case 3: prox_est =  cont != -1 ? 4 : 3; //Próximo estado
					__estados(2,&ctr2); //Estado 2
					break;
			case 4: prox_est = 1; //Próximo estado
					duty = ((y*100)+(w*10)+z); //duty cycle da ventoinha
					ctr1 = ctr2 = 0; //zera os controles
					__display('x',y,w,z); //exibi no display
					break;
		 }
  
  }
}

void __estados(int estado, int* flag)
{
	int _z; //variavel de entrada
	//FMS
	switch(estado)
	{
		case 0:	y = w = -1; 
				_size = 1; //size do contador, conta até 1
				_z = cont; //valor de entrada
				if(_z != -1) z = _z; //se valor diferente de -1
				break;
		case 1: y = -1;
				if(!*flag)
				{
					w = z;
					*flag = 1;
					_size = w == 0 ? 9 : 0; //dependendo do valor de w o size será de 0 ou 9			
				}
				_z = cont;
				if(_z != -1) z = _z;	
				break;	
		case 2: if(!*flag)
				{
					y = w;
					w = z;
					*flag = 1;
					_size = y == 0 ? 9 : 0; //dependendo do valor de y o size será de 0 ou 9			
				}
				_z = cont;
				if(_z != -1) z = _z;
				break;
	}
	__display('x',y,w,_z);
}

void __criatividade()
{
	int t1 = 0,t2 = 0,j,temp; //variaveis

	//calcular o valor médio de tensão para o duty cycle selecionado
	float a,b,c; 
	c = (duty*0.05);
	a = floor(c);
	b = c-a;
	b = 10*b;
	
	t1 = a;
	t2 = b;

	int frase[11]={'t','E','n','S','A','O','x',t1,'Q',t2,'x'};//frase a ser exibida e deslocada nos displays
    while (!_RF0)
	{
		for (j = 0;j<120;j++) 
			 __display(frase[0],frase[1],frase[2],frase[3]);
					
		temp = frase[0];
		for (j = 0; j<10;j++)
			 frase[j] = frase[j+1];
		frase[10] = temp;		
	}


}
