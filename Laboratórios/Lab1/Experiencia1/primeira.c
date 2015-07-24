//Laborat�rio 1 - Experi�ncia 1.
#include <p30f4011.h>
#define FCY	16000000UL
#include <libpic30.h>
//============================================
_FOSC( CSW_FSCM_OFF & XT_PLL8);//Estamos usando o oscilador externo de 8 MHz e multiplicado por 8 (PLL8)
_FWDT(WDT_OFF);	//O watchdog est� desativado
_FBORPOR(PBOR_ON & PWRT_OFF & MCLR_EN); // Este registrador programa o Power On Reset - POR e o Brown Out Reset - BOR e o Master Clear (Reset) - MCLR.
//============================================

/*Devido a que a frequencia do oscilador externo � 8 MHz e o PLL8, teremos que
  o valor de 8 MHz ser� multiplicado por 8 o que d� 64 MHz, mas o que se utiliza
  � o Fcy = (Fosc* PLL8)/4 = (8MHz * 8)/4 = 16 MHz (ciclos de instru��o por segundo).
  A macro __delay32(x) nos diz que x � o numero de ciclos de instru��o, portanto
  se queremos obter um atraso de 1 segundo o x = 16000000, se precisamos de
  0,5 segundos ent�o x = 8000000.  */
  
/* Trabalhando com outra macro (__delay_ms(x))deve-se definir a FCY antes da 
   biblioteca <libpic30.h>, assim na biblioteca se calcular� o numero de ciclos
   e se obter� o valor certo em ms. Temos outra macro para micro segundos (us)
   (__delay_us(x))onde o funcionamento � parecido � macro __delay_ms(x). */
   
//************* Programa Principal *************************
#define Led	LATB
#define buzzer _LATD3

/**
* Fun��o para ativar o buzzer
*
* Par�metro: int t tempo de ativa��o do buzzer
*/
void __buzzer(int t)
{
	buzzer = 1; //ativa
	__delay_ms(t); //delay de t ms
	buzzer = 0; //desativa
}


/**
* Fun��o Contador para a entrada dos n�meros de 0..9
*
* Par�metros: int i posi��o do n�mero
* 			  unsigned int num[10] vetor contendo valores correspondente aos n�meros que ser�o exibidos nos displays
* 			  unsigned int seq[2] vetor adicional com as representa��es dos displays desligados e o tra�o
*
*/
int __cont(int* i, unsigned int num[10], unsigned int seq[2])
{
	int flag = 0,_return; //flag utilizada para verificar se o bot�o foi pressionado

	while(PORTDbits.RD1 == 0) //enquanto o bot�o for pressionado fica nesse loop
		flag = 1;
	
	if(flag) //se o bot�o foi pressionado
	{
		flag = 0; //false na flag
		*i = *i < 9 ? (*i + 1) : 0; // incrementa a variavel i se for menor que nove
	}
	
	_return = *i == -1 ? seq[1] : num[*i]; //verifica o valor a ser retornado
	return _return;

}

/**
* Fun��o que faz a multiplexa��o dos display de 7 segmentos
*
* Par�metros: int x valor a ser exibido no primeiro display;
*			  int y valor a ser exibido no segundo display;
*			  int w valor a ser exibido no terceiro display;
*			  int z valor a ser exibido no quarto display;
*/
void __display(int x, int y, int w, int z, int t)
{
	int b = t; //Repeti��es necess�ria para visualizarmos o display com o referente valor
	while(b != 0)
	{
		_LATD0 = 1;
		Led = x;
		__delay_ms(5);
		_LATD0 = 0;
	
		_LATD2 = 1;
		Led = y;
		__delay_ms(5);
		_LATD2 = 0;
	
		_LATC14 = 1;
		Led = w;
		__delay_ms(5);
		_LATC14 = 0;
	
		_LATC13 = 1;
		Led = z;
		__delay_ms(5);
		_LATC13 = 0;
		
		b--;
	}	
}

/**
* Fun��o respons�vel pela sele��o da senha
*
*
*
*/
void __estados(int* i, unsigned int num[10], unsigned int seq[2], int estado, int* _x, int* _y, int* _w, int* _z, int* flag, int* ctr)
{
	int x,y,w,z;
	switch(estado)
	{
		case 0:
				x = y = w = seq[1];
				z = __cont(&*i,num,seq);
				if(z != 64) *_z = z;
				break;
		case 1:
				x = y = seq[1];
				if(*flag && !*ctr)
				{
					w = *_w = *_z;
					__buzzer(100);
					*flag = 0;
					*ctr = 1;
				}
				z = __cont(&*i,num,seq);
				if(z != 64) *_z = z;
				break;
		case 2:
				x = seq[1];
				if(*flag && !*ctr)
				{
					y = *_y = *_w;
					w = *_w = *_z;
					__buzzer(100);
					*flag = 0;
					*ctr = 1;
				}
				z = __cont(&*i,num,seq);
				if(z != 64) *_z = z;
				break;
		case 3:
				if(*flag && !*ctr)
				{
					x = *_x = *_y;
					y = *_y = *_w;
					w = *_w = *_z;
					__buzzer(100);
					*flag = 0;
					*ctr = 1;
				}
				z = __cont(&*i,num,seq);
				if(z != 64) *_z = z;
				break;
	}

	__display(x,y,w,z,20);
}

/**
* Fun��o que verifica se a senha est� correta
*
* Par�metros: int x,y,w,z valores informados pelo usu�rio
*
*/
int __check(int x, int y, int w, int z)
{
	if(x == 6 && y == 102 && w == 109 && z == 63) //Senha 1450
		return 1;

	return 0;
}


/**
* Fun��o Criatividade Deslocamento
*
*
*
*/
void __criatividade(int* prox_est)
{
	int initial = *prox_est;

	switch(initial)
	{
		case 0: *prox_est = 1;
				__display(6,102,109,63,50);
				break;
		case 1: *prox_est = 2;
				__display(2,3,67,98,50);
				break;
		case 2: *prox_est = 3;
				__display(4,28,88,84,50);
				break;
		case 3: *prox_est = 4;
				__display(6,102,109,63,50);
				break;
		case 4: *prox_est = 5;
				__display(102,109,63,94,20);
				break;
		case 5: *prox_est = 6;
				__display(109,63,94,109,20);
				break;
		case 6: *prox_est = 7;
				__display(63,94,109,115,20);
				break;
		case 7: *prox_est = 8;
				__display(94,109,115,48,50);
				break;
		case 8: *prox_est = 9;
				__display(99,67,33,32,50);
				break;
		case 9: *prox_est = 10;
				__display(12,88,92,16,50);
				break;
		case 10: *prox_est = 11;
				__display(94,109,115,48,50);
				break;
		case 11: *prox_est = 12;
				__display(109,115,48,57,20);
				break;
		case 12: *prox_est = 13;
				__display(115,48,57,79,20);
				break;
		case 13: *prox_est = 14;
				__display(48,57,79,63,20);
				break;
		case 14: *prox_est = 15;
				__display(57,79,63,113,50);
				break;
		case 15: *prox_est = 16;
				__display(96,67,98,33,50);
				break;
		case 16: *prox_est = 17;
				__display(80,76,84,88,50);
				break;
		case 17: *prox_est = 18;
				__display(57,79,63,113,50);
				break;
		case 18: *prox_est = 19;
				__display(79,63,113,102,20);
				break;
		case 19: *prox_est = 20;
				__display(63,113,102,63,20);
				break;
		case 20: *prox_est = 21;
				__display(113,102,63,6,20);
				break;
		case 21: *prox_est = 22;
				__display(102,63,6,6,50);
				break;
		case 22: *prox_est = 23;
				__display(3,98,2,2,50);
				break;
		case 23: *prox_est = 24;
				__display(28,84,4,4,50);
				break;
		case 24: *prox_est = 25;
				__display(102,63,6,6,50);
				break;
		case 25: *prox_est = 26;
				__display(63,6,6,0,20);
				break;
		case 26: *prox_est = 27;
				__display(6,6,0,0,20);
				break;
		case 27: *prox_est = 28;
				__display(6,0,0,0,20);
				break;
		case 28: *prox_est = 29;
				__display(0,0,0,0,20);
				break;
		case 29: *prox_est = 30;
				__display(0,0,0,6,20);
				break;
		case 30: *prox_est = 31;
				__display(0,0,6,102,20);
				break;
		case 31: *prox_est = 0;
				__display(0,6,102,109,20);
				break;
	}
}

/**
* Fun��o que exibi a mensagem de Sucesso ou de Erro da senha
*
* 
*
*/
void __mensagem(unsigned int suc[20], unsigned int err[4], int _x, int _y, int _w, int _z, int chk, int* cria)
{
	int x,y,w,z,i,b;
	int erro[12] = {_x,_y,_w,_z,err[0],err[1],err[2],err[3],0,0,0,0};

	switch(chk)
	{
		case 0: //mensagem de Erro
				for(i = 0; i < 12;i++)
				{
					x = erro[i];
					y = i > 10 ? erro[i-11] : erro[i+1];
					w = i > 9 ? erro[i-10] : erro[i+2];
					z = i > 8 ? erro[i-9] : erro[i+3];
					__display(x,y,w,z,20);
					b = 10;
					while(b != 0 && PORTFbits.RF6 == 1){ //Delay de 500 ms
						__delay_ms(50);
						b--;
					}
				}
				break;
		case 1: //mensagem de sucesso
				if(PORTDbits.RD1)
				{
					for(i = 0;i < 20;i++)
					{
						x = suc[i];
						y = i > 18 ? suc[i-19] : suc[i+1];
						w = i > 17 ? suc[i-18] : suc[i+2];
						z = i > 16 ? suc[i-17] : suc[i+3];
						__display(x,y,w,z,20);
					}
				}
				else __criatividade(&*cria); //chamada da criatividade
				break;
	}
}


int main ()
{
	unsigned int num0_9[10] = {63,6,91,79,102,109,125,7,127,111}; //N�meros de 0 a 9
	unsigned int erro[4] = {121,80,80,63}; //ErrO
	unsigned int sucesso[20] = {6,102,109,63,94,109,115,48,57,79,63,113,102,63,6,6,0,0,0,0}; //1450dSPIC30F4011
	unsigned int seq[2] = {0,64}; //desligado e -
	ADPCFG = 0xFFFF;	//todos os pinos da porta B como digitais
	TRISB=0;	//Todos os pinos da porta B como sa�das.
	_TRISC13=0; //Sa�da Latch
	_TRISC14=0; //Sa�da Latch
	_TRISD0=0; //Sa�da Latch
	_TRISD2=0; //Sa�da Latch
	_TRISD1=1; //Entrada bot�o 1
	_TRISF6=1; //Entrada bot�o 2
	_TRISD3=0; //Sa�da buzzer

	int est_ini = 0, est_prox = 0; //Estado inicial e Proximo estado
	int i = -1, flag = 0, cntErro = 0, ctr1 = 0, ctr2 = 0, ctr3 = 0; // Variaveis de controle
	int x,y,w,z; //Entradas da SENHA
	int criat_prox = 0; //variavel da craitividade

	while (1) //loop infinito
	{
		while(PORTFbits.RF6 == 0){ //loop para mudan�a de estado
			est_ini = est_prox;
			i = -1;
			flag = 1;
		}
			
		switch(est_ini)
		{
			case 0: est_prox = (PORTFbits.RF6 && i != -1) ? 1 : 0; //proximo estado
					__estados(&i,num0_9,seq,0,&x,&y,&w,&z,&flag,0); //valor exibido no display
					break;
			case 1: est_prox = (PORTFbits.RF6 && i != -1) ? 2 : 1; //proximo estado
					__estados(&i,num0_9,seq,1,&x,&y,&w,&z,&flag,&ctr1); //valor exibido no display
					break;
			case 2: est_prox = (PORTFbits.RF6 && i != -1) ? 3 : 2; //proximo estado
					__estados(&i,num0_9,seq,2,&x,&y,&w,&z,&flag,&ctr2); //valor exibido no display
					break;
			case 3: est_prox = (PORTFbits.RF6 && i != -1) ? 4 : 3; //proximo estado
					__estados(&i,num0_9,seq,3,&x,&y,&w,&z,&flag,&ctr3); //valor exibido no display
					break;
			case 4: est_prox = PORTFbits.RF6 ? 0 : 4; //proximo estado
					if(!__check(x,y,w,z) && flag) //verifica se a senha esta correta para conta o n�mero de erros
					{
					 	cntErro++;
						flag = 0;
					}
					if(cntErro == 3) //Se errar 3 vezes buzzer de 5s e volta para o estado inicial
					{
						cntErro = 0;
						__buzzer(5000);
						est_ini = 0;
					}
					__mensagem(sucesso,erro,x,y,w,z,__check(x,y,w,z),&criat_prox); //mensagem de erro ou sucesso da senha
					ctr1 = ctr2 = ctr3 = 0; //reiniciar controle dos estados
					break;
		}

	}
}
