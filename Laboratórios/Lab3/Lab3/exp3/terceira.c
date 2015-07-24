#include "lcd.h"
#include "ps2.h"

// Variáveis Globais
int dist=0;
float larguramicro=0;
float largura=0; // Armazena o valor da largura do sinal
float cm = 0, auxcm = 0;;
float Tosc = 1/(16000000); // Ciclo de máquina do dsPIC
float auxiliar2 = 50;
float y = 0;
unsigned long int auxiliar; // Variável que captura o valor da contagem do Timer 2
unsigned long int contador = 0; // Variável que indica o número de overflows do Timer 2
unsigned int criatividade = 0;

void __attribute__ ((interrupt,no_auto_psv))_IC1Interrupt(void)
{
	IFS0bits.IC1IF = 0; //Limpa flag de interrupção
	T2CONbits.TON = 0; //Desliga timer
	auxiliar = (contador*65536) + IC1BUF; //Armazena o valor da contagem;
	if(_LATD0==0)
		largura = 0.0000625*auxiliar;
	TMR2 = 0; //Zera Timer
	contador = 0;
	
	T2CONbits.TON = 1; //Ativa Timer
}

void __attribute__((interrupt,no_auto_psv))_T2Interrupt(void)
{
	IFS0bits.T2IF = 0; // Limpa flag de interrupção
	contador++; //Número de overflows
}

void __attribute__ ((interrupt, no_auto_psv)) _T1Interrupt(void)
{
	IFS0bits.T1IF = 0;
	
	switch(dist){
		case 1:
		   _LATE3 = ~_LATE3;
		   PR1 = 0xF424;
		   break; 
		case 2:
		   _LATE3 = ~_LATE3;
		   PR1 = 0xB71B ;
		   break; 
		case 3:
		   	_LATE3 = ~_LATE3;
		   PR1 = 0x7A12 ;
		   break; 
		case 4:
		   	_LATE3 = ~_LATE3;
		   PR1 = 0x3D09;
		   break;
		case 5:
		   	_LATE3 = 0;		   
		   break;	
		case 6:
		   _LATE3 = 1;
		   break; 
		default:
			_LATE3 = 1;
			break;
	}
	
}
	
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

float __convert();
void montar_grafico();
void setLimite();
void trataKBD(void);	

int main()
{

ADPCFG = 0xFFFF; // I/O digitais  
TRISE = 0x0100; //
                
// Configuração do Input Capture 
IC1CON = 0; 
IC1CONbits.ICTMR = 1; // Seleciona o Timer 2 
IC1CONbits.ICM = 1; // Captura na 16ª borda de subida
T1CON = 0x8030;

//Configuração dos Timers
PR2 = 0xFFFF; //Perído máximo do Timer 2 
PR1 = 0x0001;
T2CON = 0; // Timer 2 desligado
TMR2 = 0; //Zera contagem do Timer 2
IFS0 = 0; //
IEC0bits.IC1IE = 1; // Habilita interrupção do Input Capture 1 
IEC0bits.T2IE = 1; // Habilita interrupção do Timer 2
IEC0bits.T1IE = 1; // Habilita interrupção do Timer 1

init_lcd(); //inicializar LCD
initKBD(); //inicializar Teclado

setKBDFunc(&trataKBD);

while(1)
{
	auxcm = cm;
	cm = __convert();
	
	if(cm>500) cm = auxcm; //Elimna valores absurdos
	
	if (cm>150) dist=6; 
	if (cm<=150&&cm>=120) dist=1; 
	if (cm<120&&cm>=90) dist=2; 
	if (cm<90&&cm>=70) dist=3; 
	if (cm<70&&cm>=50) dist=4; 
	if (cm<50) dist=5; 
	
	switch(criatividade)
	{
		case 0:
				lcd_gotoxy(1,1);
				lcd_printf("%f,%f cm              ", largura,cm);
				lcd_putc('\n');
				break;
		case 1:
				montar_grafico();
				break;
		case 2: 
				setLimite(); 
				break;		
	}
	
	delayMS(50);
 }
  
}


/**
* Função para a conversao da largura de pulso em centímetros:
* O datasheet do sensor informa que para cada polegada (2,54 centímetros)
* há uma variação de 147 microsegundos na largura do pulso do sensor.
*
*
**/
float __convert()
{   
	larguramicro = largura*1000; //Conversao da largura para microsegundos:
	larguramicro= (larguramicro/147)*2.54; //Conversão de microsegundos para centimetros
	return larguramicro;
}


void montar_grafico()
{
	if(cm > 170)
	{
		lcd_gotoxy(1,1);
		lcd_printf("                ");
		lcd_putc('\n');
		lcd_printf("                ");
	}
	if(cm<=170&&cm>=162) {
		lcd_gotoxy(1,1);
		lcd_printf("|               ");
		lcd_putc('\n');
		lcd_printf("|               ");}
	if(cm<162&&cm>=155) {
		lcd_gotoxy(1,1);
		lcd_printf("||              ");
		lcd_putc('\n');
		lcd_printf("||              ");}
	if(cm<155&&cm>=147) {
		lcd_gotoxy(1,1);
		lcd_printf("|||             ");
		lcd_putc('\n');
		lcd_printf("|||             ");}
	if(cm<147&&cm>=140) {
		lcd_gotoxy(1,1);
		lcd_printf("||||            ");
		lcd_putc('\n');
		lcd_printf("||||            ");}
	if(cm<140&&cm>=132) {
		lcd_gotoxy(1,1);
		lcd_printf("|||||           ");
		lcd_putc('\n');
		lcd_printf("|||||           ");}
	if(cm<132&&cm>=125) {
		lcd_gotoxy(1,1);
		lcd_printf("||||||          ");
		lcd_putc('\n');
		lcd_printf("||||||          ");}
	if(cm<125&&cm>=118) {
		lcd_gotoxy(1,1);
		lcd_printf("|||||||         ");
		lcd_putc('\n');
		lcd_printf("|||||||         ");}
	if(cm<118&&cm>=111) {
		lcd_gotoxy(1,1);
		lcd_printf("||||||||        ");
		lcd_putc('\n');
		lcd_printf("||||||||        ");}
	if(cm<111&&cm>=104) {
		lcd_gotoxy(1,1);
		lcd_printf("|||||||||       ");
		lcd_putc('\n');
		lcd_printf("|||||||||       ");}	
	if(cm<103&&cm>=95) {
		lcd_gotoxy(1,1);
		lcd_printf("||||||||||      ");
		lcd_putc('\n');
		lcd_printf("||||||||||      ");}		
	if(cm<95&&cm>=87) {
		lcd_gotoxy(1,1);
		lcd_printf("|||||||||||     ");
		lcd_putc('\n');
		lcd_printf("|||||||||||     ");}	
	if(cm<87&&cm>=80) {
		lcd_gotoxy(1,1);
		lcd_printf("||||||||||||    ");
		lcd_putc('\n');
		lcd_printf("||||||||||||    ");}	
	if(cm<80&&cm>=73) {
		lcd_gotoxy(1,1);
		lcd_printf("|||||||||||||   ");
		lcd_putc('\n');
		lcd_printf("|||||||||||||   ");}
	if(cm<73&&cm>=66) {
		lcd_gotoxy(1,1);
		lcd_printf("||||||||||||||  ");
		lcd_putc('\n');
		lcd_printf("||||||||||||||  ");}
	if(cm<66&&cm>=50) {
		lcd_gotoxy(1,1);
		lcd_printf("||||||||||||||| ");
		lcd_putc('\n');
		lcd_printf("||||||||||||||| ");}			
	if(cm<50) {
		lcd_gotoxy(1,1);
		lcd_printf("||||||||||||||||");
		lcd_putc('\n');
		lcd_printf("||||||||||||||||");}	
}


void setLimite()
{
	
	if(auxiliar2>149){auxiliar2=150;}
	if(auxiliar2<51){auxiliar2=50;}
	
	y=(150-auxiliar2)/4;
	 
	if (cm>150) dist=6; 
	if (cm<=150&&cm>=(150-y)) dist=1; 
	if (cm<(150-y)&&cm>=(150-(2*y))) dist=2; 
	if (cm<(150-(2*y))&&cm>=(150-(3*y))) dist=3; 
	if (cm<(150-(3*y))&&cm>=(150-(4*y))) dist=4; 
	if (cm<150-(4*y)) dist=5; 
	
	lcd_gotoxy(1,1);
	lcd_printf("Novo Limite:%f  ",auxiliar2);
	lcd_putc('\n');
	lcd_printf("Distancia:%f",cm);
	
}

void trataKBD(void){
	KBDReady = 0;
	switch(KBDCode){
		case 0x69: // 1n
			 criatividade = 1;
			 break;
		case 0x72: // 2n
			 criatividade = 2;
			 break;
		case 0x7A: // 3n
			 criatividade = 0;
			 break;
		case 0x6B: auxiliar2 = auxiliar2 + 5;
			 break; // "4"
		case 0x73: auxiliar2 = auxiliar2 - 5;
		     break; // "5"
	}	
}	
