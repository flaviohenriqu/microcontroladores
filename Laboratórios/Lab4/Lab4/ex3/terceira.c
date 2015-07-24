#include "..\libraries\lcd.h"
#include "..\libraries\uart2.h"

unsigned char m;
int atualiza = 0;
int i = 0;
int *ADC16Ptr; 				//ponteiro que armazenará as amostras
int valor = 0;
float val_med = 0;
int duty_leds = 0;			
int duty_vent = 0;
int p, p2;
int leds[7]={0x01,0x02,0x04,0x08,0x10,0x20,0x40};
float leds_rms = 0;
float Vled_rms = 0;
int n_leds = 0;
float vent_rms = 0;
float Vvent_rms = 0;
int n_vent = 0;
float tensao_med = 0;

//Função pwm para enviar o sinal PWM à ventoinha
int pwm()
{
	p = (p+1)%101;
	if(p<=duty_vent)
		return 1;
	else
	   	return 0;		
}

// rotina de interrupção do timer2. Nesta rotina, a ventoinha recebe o PWM de um sinal de 500 Hz.
void __attribute__ ((interrupt, no_auto_psv)) _T2Interrupt(void)
{
	IFS0bits.T2IF = 0;
	_LATE1 = pwm();

	vent_rms = duty_vent*0.01;
	Vvent_rms = sqrt(vent_rms);
	Vvent_rms = Vvent_rms*5;
	n_vent = 0;
}  

// esta função pwm2 é idêntica à anterior, entretanto será usada para modificar a intensidade dos leds do DS7
int pwm2()
{
	p2 = (p2+1)%101;
	if(p2<=duty_leds)
		return 127;
	else
	   	return 0;		
}

// rotina de interrupção do timer4, esta rotina enviará o PWM aos leds do DS7 a uma frequência de 10 KHz
void __attribute__ ((interrupt, no_auto_psv)) _T4Interrupt(void)
{
	IFS1bits.T4IF = 0;
	LATB = pwm2();

	leds_rms = duty_leds*0.01;
	Vled_rms = sqrt(leds_rms);
	Vled_rms = Vled_rms*5;
	n_leds = 0;
}    	

void __attribute__((interrupt, no_auto_psv)) _U2RXInterrupt(void)
{
	_U2RXIF = 0; 		// Limpa o flag de interrupção de TX
	m = U2RXREG; 				// Le o dado do buffer e o pasa para a variável m
	if((96<m)&(m<124))m=m-32;	// transforma a letra em maiúscula (caso seja minúscula)
	atualiza=1; 				// flag para saber que aconteceu a interrupção.
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
	ADPCFG = 0xFE7F; 		// Pino RB8 e RB7 como entradas analógicas (0 = analógica, 1=digital)
	TRISB = 0x0000; 		// Porta B como saída
	TRISBbits.TRISB8 = 1;	// AN8 - vai controlar o PWM da ventoinha
	TRISBbits.TRISB7 = 1;	// AN7 - vai controlar o PWM dos leds
	
	_TRISE1 = 0;		// ventoinha

	IFS0 = 0;
	IFS1 = 0;

	IEC0bits.T2IE = 1;	// habilitando a interrupçao do timer2
	IEC1bits.T4IE = 1;	// habilitando a interrupçao do timer4
	
	T2CON = 0x8000;		// ativamos o timer 2
	T4CON = 0x8000;		// ativamos o timer 4

	PR2 = 32000;		// Puslo de 500 Hz
	PR4 = 3200;			// Pulso de 5000 Hz	
	
//Configuração do SFR ADCON1:Adc desligado, Formato de saída: inteiro, Amostragem/conversão manual
	ADCON1=0x00E0;			// conversão automática começa depois da amostragem
	
//Configuração do SRF ADCON2: tensão de referência: AVDD e AVSS, sem varredura, conversão pelo canal 0, interrupção após uma
//amostra, buffer como palavra de 16 bits
	ADCON2bits.SMPI = 15;	// interrupção ocorre a cada 2 amostras			
	
	ADCON2bits.CSCNA = 1;	// varre múltiplas entradas para o canal 0

//Configuração do SRF ADCON3: clock do ADC = 500kHz, tempo de amostragem = 16 TAD, fonte do clock: ciclo da máquina
	ADCON3=0x101F; //Tad= 16*Tcy = 16*62,5 ns = 1us > 153,85 ns (quando Vdd = 4,5 a 5,5V)
	ADCON3bits.SAMC =32; 	// tempo de amostragem de 7 TAD = 7 us; 

//Configuração do SRF ADCHS: seleciona o canal CH0, configura entrada analógica AN8 (RB8), entrada de referência negativa do
//CHO igual a Vref-
	ADCHS=0x0000;
	
//Configuração do SRF ADCSSL: varredura ativada para coletar RB7 e RB8
	ADCSSL = 0x0180;	 // Varrendo RB7 e RB8	
	ADCON1bits.ADON = 1; //Ativa o ADC
// ------------------------ CONFIGURAÇÃO DA SERIAL ---------------------------------------//
	INIT_UART2(51);		//bauda rate de 19200 KHz para 16 Mips
//---------------------------------------------------------------------------------------
	
	init_lcd();
	PRINTF_UART2("    Ventoinha       \t | \t     Leds    \n");
 	 			  
 while(1)
 { 

	valor = 0;
	ADC16Ptr = &ADCBUF0;		
	IFS0bits.ADIF = 0;			
	ADCON1bits.ASAM = 1;		
	while(IFS0bits.ADIF == 0);	
	ADCON1bits.ASAM = 0;
	delayUS(81);		
	
			

	
	val_med = (float)*(ADC16Ptr);				// o primeiro valor armazenado virá de AN7
	tensao_med = val_med*5;
	tensao_med = tensao_med/(1023);
	leds_rms = leds_rms+(tensao_med*tensao_med);
	n_leds++;
	
	val_med = val_med*100;				//val_med pegará o duty medio selecionado -> duty = val_med*100/1023
	val_med = val_med/(1023);			// valor medio das 16 amostras
	duty_leds = (int)val_med;
	
	
	val_med = (float)*(ADC16Ptr+1);				// o primeiro valor armazenado virá de AN7
	tensao_med = val_med*5;
	tensao_med = tensao_med/(1023);
	vent_rms = vent_rms+(tensao_med*tensao_med);
	n_vent++;
	
	val_med = val_med*100;				//val_med pegará o duty medio selecionado -> duty = val_med*100/1023
	val_med = val_med/(1023);
	duty_vent = (int)val_med;			// o segundo valor virá de AN8

	   
	PRINTF_UART2("\r %d%% - %f Vrms\t | \t %d%% - %f  Vrms",duty_vent,Vvent_rms,duty_leds,Vled_rms);
	delayMS(500);

	
}	

 	
 
 	return 0;	
}
