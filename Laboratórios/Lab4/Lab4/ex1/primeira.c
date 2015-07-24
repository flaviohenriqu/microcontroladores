#include "..\libraries\lcd.h"
#include "..\libraries\uart2.h"

unsigned char m;
int atualiza = 0;
int duty_buzzer = 0;
int perifericos = 0;
int i = 0;
int *ADC16Ptr;
int valor = 0;
int p1;
int menu = 0;
int letra = 0;
int letra2 = 0;
int can_a,can_b,can_c,can_d,can_e,can_f,can_sete,can_oito;
int timer = 0;
int can[2]={0,0};	// canais 7 e 8.
int mostra = 0;
int timer2 = 0;
int cont = 0;
float duty, val_med7, val_med8;

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

int pwm()
{
	p1 = (p1+1)%101;
	if(p1<=duty_buzzer)
		return 1;
	else
	   	return 0;		
}

void __attribute__ ((interrupt, no_auto_psv)) _T2Interrupt(void)
{
	IFS0bits.T2IF = 0;
	if(perifericos == 1)
	{

		_LATE3 = pwm();

		
		atualiza = 0;
	}	
}	

void __attribute__((interrupt, no_auto_psv)) _U2RXInterrupt(void)
{
	_U2RXIF = 0; 		// Limpa o flag de interrupção de TX
	m = U2RXREG; 				// Le o dado do buffer e o pasa para a variável m
	if((96<m)&(m<124))m=m-32;	// transforma a letra em maiúscula (caso seja minúscula)
	
	atualiza=1; 				// flag para saber que aconteceu a interrupção.
	
	if(m == 49)					// se apertou 1, entra no menu do tempo de aquisição
		menu = 1;
	if(m == 50)					// se apertou 2, entra no menu de seleção manual
		menu = 2;
	if(m == 27)					// se apertou esc, volta ao menu principal
		menu = 0;
	if((m>=65) & (m<=69)) 	// vai para o menu de ações
	{
		menu = 3;
		atualiza = 0;
		mostra = 1;
		TMR1 = 0;
		timer = 0;
	}	 
	if((m>=54)&(m<=56))
	{
		menu = 4;
		atualiza = 0;
		mostra = 1;
		TMR1 = 0;
		timer = 0;
	}
	if(m == 80){
		perifericos = 1;
		m = letra;
	}	
}		

void __attribute__ ((interrupt, no_auto_psv)) _T1Interrupt(void)
{
	IFS0bits.T1IF = 0;
	
	timer++;
	timer2 = (timer2+1)%120;
	
	if(timer==1 && letra == 65)	// a cada 0,5 segundos
	{
		can_a = 1;
		timer = 0;
	}	
	else
		can_a = 0;
	
	if(timer==2 && letra == 66)// a cada 1 segundo
	{
		can_b = 1;
		timer = 0;
	}	
	else
		can_b = 0;
	
	if(timer==20 && letra == 67)// a cada 10 segundos
	{
		can_c = 1;
		timer = 0;
	}	
	else
		can_c = 0;
			
	if(timer==120 && letra == 68)// a cada 1 minuto 
	{
		can_d = 1;
		timer = 0;
	}	
	else
		can_d = 0;
			
	if(timer==7200 && letra == 69)// a cada 1 hora
	{
		can_e = 1;
		timer = 0;
	}	
	else
		can_e = 0;
	
}  

int converte(int i){	
	valor = 0;
	ADC16Ptr = &ADCBUF0;	
	IFS0bits.ADIF = 0;
	ADCON1bits.ASAM = 1;	
	while(IFS0bits.ADIF == 0);
	ADCON1bits.ASAM = 0;	
	valor = *(ADC16Ptr+i);
	delayUS(81);
	return (valor);
}


int main(void)
{
	ADPCFG = 0xFE7F;
	TRISB = 0x0000;	
	TRISBbits.TRISB8 = 1;	
	TRISBbits.TRISB7 = 1;

	_TRISE1 = 0;

	_LATE1 = 0;


	IFS0 = 0;
	IFS1 = 0;

	IEC0bits.T1IE = 1;
	IEC0bits.T2IE = 1;
	
	T1CON = 0x8030;
	T2CON = 0x8000;

	PR1 = 31250;
	PR2 = 3200;

	ADCON1=0x00E0;			// conversão automática começa depois da amostragem
	
	ADCON2bits.SMPI = 15;	// interrupção ocorre a cada 16 amostras			
	
	ADCON2bits.CSCNA = 1;	// varre múltiplas entradas para o canal 0

	ADCON3=0x101F; 			//Tad= 16*Tcy = 16*62,5 ns = 1 us > 153,85 ns (quando Vdd = 4,5 a 5,5V)
	ADCON3bits.SAMC = 32; 	// tempo de amostragem de 7 TAD = 7 us; 

	ADCHS=0x0000;
	
	ADCSSL = 0x0180;	 // Varrendo RB8 e RB7
	ADCON1bits.ADON = 1; //Ativa o ADC
	
	INIT_UART2(51);		//bauda rate de 19200 KHz para 16 Mips
	
	init_lcd();
	while(1)
  	{ 
		if(menu == 0)
		{
			PUTS_UART2("\f");
			PUTS_UART2("1)Selecione o tempo de aquisicao\n\r");
			PUTS_UART2("2)Selecione o canal manualmente\n\r");
			delayMS(2000);

		}
		
		if(atualiza == 1)
		{
			if(menu == 1)
			{
				PUTS_UART2("\f");	
				PUTS_UART2("a)Aquisicao a cada 0,5 segundos\n\r");
				PUTS_UART2("b)Aquisicao a cada 1,0 segundo\n\r");
				PUTS_UART2("c)Aquisicao a cada 10,0 segundos\n\r");
				PUTS_UART2("d)Aquisicao a cada 1 minuto\n\r");
				PUTS_UART2("e)Aquisicao a cada 1 hora\n\r");
			}
			if(menu == 2)
			{
				PUTS_UART2("\f");
				PUTS_UART2("7)Selecione o canal 7\n\r");
				PUTS_UART2("8)Selecione o canal 8\n\r");
			}

			atualiza = 0;			
		}		
		
		if(menu == 3)
		{
		 	letra = m;				//letra recebe a opção selecionada
			
			switch(letra)
			{
				case(65): //A
				{
					if(can_a==1){
						for(i=0;i<2;i++){
							can[i]=converte(i);
						}	
					}	
						
					can_a = 0;
					break;
				}
				case(66): //B
				{
					if(can_b==1){
						for(i=0;i<2;i++){
							can[i]=converte(i);
						}	
					}	
					can_b = 0;
					break;
				}
				case(67): //C
				{
					if(can_c==1){
						for(i=0;i<2;i++){
							can[i]=converte(i);
						}	
					}	
					can_c = 0;
					break;
				}
				case(68): //D
				{
					if(can_d==1){
						for(i=0;i<2;i++){
							can[i]=converte(i);
						}
					}	
					can_d = 0;
					break;
				}
				case(69): //E
				{
					if(can_e==1){
						for(i=0;i<2;i++){
							can[i]=converte(i);
						}
					}	
					can_e = 0;
					break;
				}
			}
			

			if(mostra == 1){
				if(can_sete == 1)	
					PRINTF_UART2("\fCanal 7\r\n");
				if(can_oito == 1)	
					PRINTF_UART2("\fCanal 8\r\n");	
				
				if(can_oito == 0 && can_sete == 0)
					PRINTF_UART2("\fCanal 7 \t | \t Canal 8\r\n");
			
				mostra = 0;	
			}
		
		 	if(can_sete == 1){
				val_med7 = (float)can[0];
				val_med7 *= 5;
				val_med7 = val_med7/1023;

				PRINTF_UART2("   %f     \r", val_med7);
				lcd_gotoxy(1,1);
		 		lcd_printf("Canal 7         ");
		 		lcd_gotoxy(1,2);
		 		lcd_printf("%f             ", val_med7);
				duty_buzzer = can[0];
			}
			if(can_oito == 1){
				val_med8 = (float)can[1];
				val_med8 *= 5;
				val_med8 = val_med8/1023;

				PRINTF_UART2("   %f     \r",val_med8);	
				lcd_gotoxy(1,1);
		 		lcd_printf("Canal 8         ");
		 		lcd_gotoxy(1,2);
		 		lcd_printf("%f              ", val_med8);
			}	

			if(can_sete == 0 && can_oito == 0)			
			{
				val_med7 = (float)can[0];
				val_med7 *= 5;
				val_med7 = val_med7/1023;

				val_med8 = (float)can[1];
				val_med8 *= 5;
				val_med8 = val_med8/1023;

				PRINTF_UART2("   %f    \t | \t   %f      \r",val_med7,val_med8);
			}

		}

		if(menu == 4)
		{
			letra2 = m;
			switch(letra2){
				case(55): //7
				{
					can_sete = 1;
					can_oito = 0;
					break;
				}
				case(56): //8
				{
					can_sete = 0;
					can_oito = 1;
					break;
				}
			}

			menu = 3;
			m = letra;
		}
		 
 	}
 	return 0;	
}
