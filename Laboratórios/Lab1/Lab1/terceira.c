//Laboratório 1 - Experiencia 3.
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

/**
* Função Deslocar os Leds de 0..7 (Direita)
*
*/
void __deslocaDireita(int* cnt)
{
	Led = *cnt; //Atribuir o valor na porta de saída
	*cnt = *cnt >> 1; //Desloca o bit para direita
	*cnt = *cnt < 1 ? 128 : *cnt; //Se for menor que 1 reinicia em 128
}

/**
* Função Deslocar os Leds de 7..0 (Esquerda)
*
*/
void __deslocaEsquerda(int* cnt)
{
	Led = *cnt; //Atribuir o valor na porta de saída
	*cnt = *cnt << 1; //Desloca o bit para esquerda
	*cnt = *cnt > 128 ? 1 : *cnt; //Se for maior que 128 reinicia em 1
}

/**
* Função para ligar a combinação de LEDs
*
*/
void __sequencia(int* i, unsigned int seq[4])
{
	Led = seq[*i]; //Atribuir o valor na porta de saída
	*i = *i < 3 ? *i + 1 : 0; //Incrementa até 3 a variavel de controle
}

/**
* Criatividade Funcionalidade de PWM
*
*/
void __criatividade()
{
	while(!PORTFbits.RF6)
	{
		Led = 255;
        PDC1 = 450;  // channel 1 pulse width = 0.9ms
		__delay_ms(1000); // 1 second delay
 		
		Led = 255;
        PDC1 = 750;  // channel 1 pulse width = 1.5ms
		__delay_ms(1000); // 1 second delay

		Led = 255;
        PDC1 = 1050;  // channel 1 pulse width = 2.1ms
		__delay_ms(1000); // 1 second delay

		Led = 255;
        PDC1 = 1350;  // channel 1 pulse width = 2.7ms
		__delay_ms(1000); // 1 second delay

		Led = 255;
        PDC1 = PTPER;  // channel 1 pulse width = 10.0ms
		__delay_ms(1000); // 1 second delay

	}
}

/**
* Função Principal
*
*/
int main ()
{
	unsigned int seq[4] = {129,66,36,24}; //sequência (0,7), (1,6), (2,5) e (3,4)
	int ini_estado = 0,prox_estado = 0; //Estado inicial e proximo estado da FMS
	ADPCFG = 0xFFFF;	//todos os pinos da porta B como digitais
	TRISB=0;	//Todos os pinos da porta B como saídas.
	_TRISD1=1; //Botão de Entrada
	_TRISF6=1; //Botão de Entrada
	Led = 0; //Saída dos LEDs

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
    PTCONbits.PTEN = 1;   // Enable PWM time base

	int cnt = 1, i = 0, b;	//Variaveis de controle
	while (1) //loop infinito
	{
		while(PORTDbits.RD1 == 0){ //Loop quando o botão está pressionado
			ini_estado = prox_estado; //Adiciona o proximo estado a ser ativo
		}

		//Maquina de Estado Finito (FMS)
		switch(ini_estado)
		{
			case 0:
					prox_estado = PORTDbits.RD1 ? 1 : 0; 
					Led = 0; //Estado inicial LEDs desligados
					break;
			case 1:
					prox_estado = PORTDbits.RD1 ? 2 : 1;
					if(PORTFbits.RF6) Led = 255; //Ligar todos os LEDs
					
					//Criatividade PWM
					__criatividade();
					break;
			case 2:
					prox_estado = PORTDbits.RD1 ? 3 : 2;
					__deslocaEsquerda(&cnt); //Deslocar para Direita
					break;
			case 3:
					prox_estado = PORTDbits.RD1 ? 4 : 3;
					__deslocaDireita(&cnt); //Deslocar para Esquerda
					break;
			case 4:
					prox_estado = PORTDbits.RD1 ? 1 : 4;
					__sequencia(&i,seq); //Sequencia para ligar LEDs
					break;
			default: prox_estado = 0;
					Led = 0;
					break;
		}

		//Quando selecionar o botão a qualquer momento mudar de estado
		b = 10;
		while(b != 0 && PORTDbits.RD1 == 1){ //Delay de 500 ms
			__delay_ms(50);
			b--;
		}
	}
}
