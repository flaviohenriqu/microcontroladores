#include "..\libraries\lcd.h"
#include "..\libraries\uart2.h"

unsigned char m;
int atualiza = 0;
int i = 0;
int *ADC16Ptr; 				//ponteiro que armazenará as 16 amostras
int valor = 0;
float val_med = 0;
int conversao = 0;			// indicará se a conversão foi ativa
int parada = 1;
int ledsB[7] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40};
float leds = 0;
int j = 0;
int escala = 0;
float _valor = 0.0;
float _valorTotal = 0.0;

void __attribute__((interrupt, no_auto_psv)) _U2RXInterrupt(void)
{
	_U2RXIF = 0; 		// Limpa o flag de interrupção de TX
	m = U2RXREG; 				// Le o dado do buffer e o pasa para a variável m
	if((96<m)&(m<124))m=m-32;	// transforma a letra em maiúscula (caso seja minúscula)
	atualiza=1; 				// flag para saber que aconteceu a interrupção.

	if(m == 49)
		escala = 0;
	if(m == 50)
		escala = 1;
	if(m == 51)
		escala = 2;
}


// ******* Função de atraso de tempo. Gera um atraso de 'tempo' us
void delayUS(int i){
        while(i>0){
                asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
                asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
                i--;
        }        
}

void delayMS(int i){
        while(i>0){
                delayUS(1000);        
                i--;
        }        
}

int main(void)
{
	ADPCFG = 0xFEFF; // Pino RB8 como entrada analógica (0 = analógica, 1=digital)
	TRISB = 0x0000; // Porta B como saída
	TRISBbits.TRISB8 = 1;	//AN8

	TRISF = 0x0000;
	TRISE = 0x0000;
	
	//Configuração do SFR ADCON1:Adc desligado, Formato de saída: inteiro, Amostragem/conversão manual
	ADCON1=0x0000;			// conversão automática começa depois da amostragem
	
	//Configuração do SRF ADCON2: tensão de referência: AVDD e AVSS, sem varredura, conversão pelo canal 0, interrupção após uma
	//amostra, buffer como palavra de 16 bits
//	ADCON2bits.SMPI= 15;	// interrupção ocorre a cada 16 amostras			
	//Configuração do SRF ADCON3: clock do ADC = 500kHz, tempo de amostragem = 16 TAD, fonte do clock: ciclo da máquina
	ADCON3=0x101F; //Tad= 16 * Tcy = 16*62,5 ns = 1us > 153,85 ns (quando Vdd = 4,5 a 5,5V)
//	ADCON3bits.SAMC =	9; 	// tempo de amostragem de 9 TAD = 9 us; 
	//tempo total = 166,66 ms -> 6 KHz
	//Tad = 1us
	//conversao = 12Tad = 12us
	//Tempo de amostragem = 9us * 16 amostras = 144 us
	// Tempo total = 144+12 = 156 us -> 6,4 KHz

	//Configuração do SRF ADCHS: seleciona o canal CH0, configura entrada analógica AN8 (RB8), entrada de referência negativa do
	//CHO igual a Vref-
	ADCHS=0x0000;
	ADCHSbits.CH0SA = 8; // seleciona o canal 8

	ADCSSL=0;
	ADCON1bits.ADON = 1; //Ativa o ADC
	INIT_UART2(51);		//bauda rate de 19200 KHz para 16 Mips
	
	init_lcd();
	
 while(1)
 { 
	if (atualiza == 1){
		if(m == 65)		// se pressionou A, realiza amostragem
		{
			PUTS_UART2("A conversao iniciou, confira no LCD, meu caro...\n\r"); 
			conversao = 1;
			parada = 1;
			ADCON1bits.ADON = 1;			
		}
		
		if(m == 80)		// se pressionou P,	
		{
			conversao = 0;
			ADCON1bits.ADON = 0;	//desativa ADC
			PUTS_UART2("\r\fA conversao parou, pressionar A para comecar de novo\n\r"); 
			parada = 0;
			delayMS(1000);
		}	
		
		atualiza = 0;
	}// fim do atualiza == 1		
	
	if(conversao == 1)
	{
		
		valor = 0;
		parada = 1;
	
		for(i=0; i<16; i++)
		{					// pegando 16 amostras
			
			IFS0bits.ADIF = 0;			
			ADCON1bits.SAMP = 1;
			delayUS(50);
			ADCON1bits.SAMP = 0;		
			while(!ADCON1bits.DONE);	
			
			valor = valor + ADCBUF0;	//coletando as 16 amostras
			_valorTotal = (valor * 0.0049);
			_valor = (ADCBUF0 * 0.0049);
			PRINTF_UART2("Valor %d %f \t%f \r\n",i,_valorTotal,_valor);
			lcd_gotoxy(1,1);
			lcd_printf("Valor amostrado:");
			lcd_gotoxy(1,2);
			lcd_printf("%d            ",ADCBUF0);
			delayUS(104);
				
			if(m == 80)		// se pressionou P,	
			{
				conversao = 0;
				ADCON1bits.ADON = 0;	//desativa ADC
				PUTS_UART2("\r\fA conversao parou, pressionar A para comecar de novo"); 
				delayMS(1000);
				parada = 0;
				break;
			}	
			
			
		}

		if(parada!=0)
		{	
			switch(escala)
			{
				case 0: //pressiona 1 - escala 5v
						val_med = (float)(valor);
						val_med = val_med*5;
						val_med = val_med/(16*1023);
						break;
				case 1: //pressiona 2 - escala de 12v
						val_med = (float)(valor);
						val_med = val_med*12;
						val_med = val_med/(16*1023);
						break;
				case 2: //pressiona 3 - escala de 0 - 1023
						val_med = (float)(valor);
						break;

			}

			PRINTF_UART2("\nValor medio %f\r\n\n",val_med);
			lcd_gotoxy(1,1);
			lcd_printf("\fValor medio:\n");
			lcd_gotoxy(1,2);
			lcd_printf("%f",val_med);
			delayMS(1000);
		}
	}	
	
	
 }
 return 0;	
}
