#include "lcd.h"
#include "ps2.h"

unsigned char menuAtual = 0;
unsigned char opcaoAtual = 0;

typedef enum _vel{LENTA=0, MEDIA, VELOZ} VEL;
unsigned char cont = LENTA;
unsigned char mensagemPos = 0;
unsigned char contDelay = 0;

unsigned char menu[4][5][25] = 	{
								{
									"1",
									"PERIFERICOS",
									"a)Ventoinha (Q)",
									"b)Buzzer (A)",
									"c)Mensagem (Z)"
								},
								{
									"2",
									"VENTOINHA",
									"1)Dutycycle 100%% (Q)",
									"2)Dutycycle 50%% (A)",
									"3)Dutycycle 30%% (Z)"
								},
								{
									"3",
									"BUZZER",
									"1)Alterna em 2s (Q)",
									"2)Desliga (A)"
								},
								{
									"4",
									"MENSAGEM",
									"Microcontroladores 2013-1"
								}
							};

unsigned char limiteMenu = 4;
unsigned char limiteOpcoes[4] = {3, 3, 2, 1};

void atualizaMenu(){
	lcd_clear();
	lcd_printf(menu[menuAtual][1]);
	lcd_putc('\n');
	lcd_printf(menu[menuAtual][opcaoAtual+2]);
}

void avaliaConfirmacao(){
	switch(menuAtual){
		case 0:
			menuAtual = menuAtual + opcaoAtual + 1;
			opcaoAtual = 0;
			break;
		case 1:
			switch(opcaoAtual){// Selecionar PWM
				case 0:
					cont = VELOZ;
					break;
				case 1:
					cont = MEDIA;
					break;
				case 2:
					cont = LENTA;
					break;
			}
			menuAtual = 0;
			opcaoAtual = 0;
			break;
		case 2:
			switch(opcaoAtual){
				case 0:
						T1CON = 0x8030;
						_LATE3 = 1;
						// Liga Buzzer
						break;
				case 1:
						T1CON = 0x0030;
						_LATE3 = 0;
						// Desliga Buzzer
						break;
				}	
			menuAtual = 0;
			opcaoAtual = 0;
			break;
		case 3:
			mensagemPos = 0;
			contDelay = 0;
	
			menuAtual = 0;
			opcaoAtual = 0;
			break;
	}
}

void trataTecla(void){
	KBDReady = 0; // clear the flag
	
	unsigned char mudouMenu = 0;
	
	switch(KBDCode){
		case 0x75: // Para cima
			opcaoAtual = (opcaoAtual + limiteOpcoes[menuAtual] - 1) % limiteOpcoes[menuAtual];
			mudouMenu = 1;
			break;
		case 0x72: // Para baixo
			opcaoAtual = (opcaoAtual + 1) % limiteOpcoes[menuAtual];
			mudouMenu = 1;
			break;
		case 0x73: // 73 para n5
			avaliaConfirmacao();
			mudouMenu = 1;
			break;
		case 0x66:
			menuAtual = 0;
			opcaoAtual = 0;
			mudouMenu = 1;
			break;
		case 0x15: // 1
			if(limiteOpcoes[menuAtual] < 1){
				break;
			}
			if(opcaoAtual == 0){
				avaliaConfirmacao();
				opcaoAtual = 0;
			} else {
				opcaoAtual = 0;
			}
				mudouMenu = 1;
			break;
		case 0x1C: // 2
			if(limiteOpcoes[menuAtual] < 2){
				break;
			}
			if(opcaoAtual == 1){
				avaliaConfirmacao();
				opcaoAtual = 0;
			} else {
				opcaoAtual = 1;
			}
				mudouMenu = 1;
			break;
		case 0x1A: // 3
			if(limiteOpcoes[menuAtual] < 3){
				break;
			}
			if(opcaoAtual == 2){
				avaliaConfirmacao();
				opcaoAtual = 0;
			} else {
				opcaoAtual = 2;
			}
				mudouMenu = 1;
			break;
	}
	
	if(mudouMenu){
		atualizaMenu();
	}
}

void __attribute__ ((interrupt, no_auto_psv)) _T1Interrupt(void)
{
	IFS0bits.T1IF = 0;
	_LATE3 = ~_LATE3;
}

void __attribute__ ((interrupt, no_auto_psv)) _T2Interrupt(void)
{
	IFS0bits.T2IF = 0;
	
	switch(cont){ // VERIFICAR OS PRs
		case LENTA:
			if(_LATE1){
				_LATE1 = 0;
				PR2 = 0x5780; //Contagem para 80%
			} else {
				_LATE1 = 1;
				PR2 = 0x2580; //Contagem para 20%
			}
			break;
		case MEDIA:
			_LATE1 = ~_LATE1;
			PR2 = 0x3E80; //1.5 segundos e inverte o sinal (período de 3 segundos)
			break;
		case VELOZ:
			_LATE1 = 1;
			PR2 = 0x1900; //Contagem para 20%
			break;
		default:
			_LATE1 = ~_LATE1;
			PR2 = 0x3E80; //1.5 segundos e inverte o sinal (per?odo de 3 segundos)
	}
	
}
	

//************* Programa Principal *************************
int main (void)
{
	init_lcd(); //inicializar LCD
	KBDReady = 0;  // zeramos o flag
	_TRISC14 = 1; // Fazer o pino CN0 = RC14 uma entrada (clock)
	_TRISC13 = 1; // Fazer o pino CN1 = RC13 uma entrada (data)
	TRISE = 0x00;
	
	IFS0=0; //Flag de interrupção do timer3,INT0
	
	IEC0bits.T1IE = 1;
	IEC0 = IEC0 | 0x0040; // bit 7 do registrador IEC0 habilita a interrupção do timer3 (IEC0bits.T3IE=1)
	PR2 = 0x3E80; 
	T2CON = 0x8000; //ativamos o timer2/3

	T1CON = 0x0030;
//	_LATE3 = 1;
	
//	cont = LENTA;
	
	initKBD(); //inicializar teclado
	setKBDFunc(&trataTecla);
	
	atualizaMenu();
	
	while (1)
	{
		if(menuAtual == 3){
			mensagemPos = (mensagemPos+1)%26;
			delay_ms(50);
			contDelay = contDelay+1;
			lcd_gotoxy(1,2);
			lcd_printf(((char*)menu[menuAtual][opcaoAtual+2] + mensagemPos));	
			lcd_putc(' ');
			
			if(contDelay == 50){
				menuAtual = 0;
				opcaoAtual = 0;
				atualizaMenu();
			}	
		}
	}
}
