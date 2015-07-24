//Laboratório 1 - Experiência 2.
#include <p30f4011.h>
#define FCY	16000000UL
#include <libpic30.h>
//============================================
_FOSC( CSW_FSCM_OFF & XT_PLL8);//Estamos usando o oscilador externo de 8 MHz e multiplicado por 8 (PLL8)
_FWDT(WDT_OFF);	//O watchdog está desativado
_FBORPOR(PBOR_ON & PWRT_OFF & MCLR_EN); // Este registrador programa o Power On Reset - POR e o Brown Out Reset - BOR e o Master Clear (Reset) - MCLR.
//============================================

/*Devido a que a frequencia do oscilador externo é 8 MHz e o PLL8, teremos que
  o valor de 8 MHz será multiplicado por 8 o que dá 64 MHz, mas o que se utiliza
  é o Fcy = (Fosc* PLL8)/4 = (8MHz * 8)/4 = 16 MHz (ciclos de instrução por segundo).
  A macro __delay32(x) nos diz que x é o numero de ciclos de instrução, portanto
  se queremos obter um atraso de 1 segundo o x = 16000000, se precisamos de
  0,5 segundos então x = 8000000.  */
  
/* Trabalhando com outra macro (__delay_ms(x))deve-se definir a FCY antes da 
   biblioteca <libpic30.h>, assim na biblioteca se calculará o numero de ciclos
   e se obterá o valor certo em ms. Temos outra macro para micro segundos (us)
   (__delay_us(x))onde o funcionamento é parecido à macro __delay_ms(x). */
   
//************* Programa Principal *************************
#define Led	LATB
#define buzzer _LATD3

void __ligarDisplay(int seq[], unsigned int num0_6[7], int _buzzer, int* ctn)
{
	int b = 20; //repetição para que o valor possa ser visto nos displays
	while(b != 0) //condição para o loop
	{
		_LATC14 = 1; //ativar o primeiro display
		Led = num0_6[seq[0]]; //atribuir o valor a ser exibido
		__delay_ms(5); //delay 5ms
		_LATC14 = 0; //desligar display

		_LATC13 = 1; //ativar o segundo display
		Led = num0_6[seq[1]]; //atribuir o valor a ser exibido
		__delay_ms(5); //delay 5ms
		_LATC13 = 0; //desligar display

		if((_buzzer) && (*ctn < 10)) //condição para ativar o buzzer por 2s
			_PTEN = 1;
		else
			_PTEN = 0;

		b--; //decrementar contador
	}
	*ctn = _buzzer ? *ctn + 1 : 0; //incrementar controle se buzzer estiver ativo
}

void pause(unsigned short i){
	unsigned short j;
	for (j = 0; j < i; j++){
		__delay_ms(10);
	}
}

void Sound_Play(int _freq, int t)
{
	
	PTPER = _freq;
	PDC1 = _freq;
	__delay_ms(t);
}

void __criatividade()
{
	// Happy birthday notes
	/*                        Hap py  Birth Day  to  you,  Hap py  birth day  to
	                         C4   C4   D4   C4   F4   E4   C4   C4   D4   C4   G4 */
	unsigned int notes[] = {954, 954, 850, 954, 716, 757, 954, 954, 850, 954, 637,
	
	/*                       you, Hap py  Birth Day  dear  xxxx      Hap  py   birth
	                         F4   C4   C4   C5   A4   F4   E4   D4   B4b  B4b  A4 */
	                        716, 954, 954, 478, 568, 716, 757, 850, 536, 536, 568,
	
	/*                       day  to  you
	                         F4   G4   F4   */
	                        716, 637, 716
	                        };
	
	unsigned short interval[] = {4, 4, 8, 8, 8, 10, 4, 4, 8, 8, 8, 10, 4, 4, 8, 8, 8,
	                             8, 8, 4, 4, 8, 8, 8, 12};
	
	unsigned short k;
	
	while(PORTFbits.RF0 == 0)
	{
		_PTEN = 1;
		for(k = 0; k<25; k++){
			
			Sound_Play(notes[k], 100*interval[k]);
			pause(6);
		}
		pause(100);
		_PTEN = 0;
	}
}

int main () //função principal
{
	unsigned int num0_6[7] = {63,6,91,79,102,109,125}; //Números de 0 a 6
	ADPCFG = 0xFFFF;	//todos os pinos da porta B como digitais
	TRISB=0;	//Todos os pinos da porta B como saídas.
	_TRISC13=0; //Saída Latch
	_TRISC14=0; //Saída Latch
	_TRISD1=1; //Entrada botão 1
	_TRISF6=1; //Entrada botão 2
	_TRISD3=0; //Saída buzzer

	_TRISF0=1; //Entrada da Criatividade
    // Configure PWM for free running mode
    //
    //   PWM period = Tcy * prescale * PTPER = 0.33ns * 64 * PTPER
    //   PWM pulse width = (Tcy/2) * prescale * PDCx
    //
    PWMCON1 = 0x00FF;     // Enable all PWM pairs in complementary mode
    PTCONbits.PTCKPS = 3; // prescale=1:64 (0=1:1, 1=1:4, 2=1:16, 3=1:64)
    PTPER = 5000;         // 20ms PWM period (15-bit period value)
    PTMR = 0;             // Clear 15-bit PWM timer counter
	PDC1 = PTPER;
    PTCONbits.PTEN = 0;   // Enable PWM time base


	int ctn0 = 0, ctn1 = 0, ctn2; //controle para ligar o buzzer durante 2s

	while (1) //loop infinito
	{
		if(PORTDbits.RD1 == 0 && PORTFbits.RF6 == 0){ //Os dois botões não presionados
			int X[] = {0,0}; //valores a serem exibidos nos displays
			ctn1 = 0; //zerar controle da segunda condição
			PDC1 = PTPER = 5000;
			__ligarDisplay(X,num0_6,1,&ctn0); //ligar display e o buzzer ativado
		}
		else if(PORTDbits.RD1 == 0 && PORTFbits.RF6 == 1) //Botão na porta RF6 pressionado
		{
			int y[] = {1,2}; //valores a serem exibidos nos displays
			ctn0 = 0; //zerar controle da primeira condição 
			PDC1 = PTPER = 5000;
			__ligarDisplay(y,num0_6,1,&ctn1); //ligar display e o buzzer ativado			
		}
		else if(PORTDbits.RD1 == 1 && PORTFbits.RF6 == 0) //Botão na porta RD1 pressionado
		{
			int z[] = {3,4}; //valores a serem exibidos nos displays
			ctn0 = ctn1 = 0; //zerar controle das condições iniciais 
			__ligarDisplay(z,num0_6,0,&ctn2); //ligar display e desativar o buzzer
		}
		else if(PORTDbits.RD1 == 1 && PORTFbits.RF6 == 1) //Os dois botões pressionados
		{
			int w[] = {5,6}; //valores a serem exibidos nos displays
			ctn0 = ctn1 = 0; //zerar controle das condições iniciais 
			__ligarDisplay(w,num0_6,0,&ctn2); //ligar display e desativar o buzzer
			__criatividade();
		}
	}
}
