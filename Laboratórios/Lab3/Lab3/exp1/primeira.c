#define FCY	16000000UL
#include <libpic30.h>
#include <p30f4011.h>
#include "lcd.h"
#define _buzzer _LATE3

long int T = 0;
unsigned int voltas = 0;
int cont = 0, duty = 2, est_ini = 0, prox_est = 0;
int rpm;


void __attribute__ ((interrupt, no_auto_psv)) _T1Interrupt(void)
{
	IFS0bits.T1IF = 0; //Zera o flag de interrupção
	if (_RE1 == 1) //Verifica o estado da saída da ventoinha
	{
		_LATE1 = 0; //Desabilita a ventoinha
		PR1 = T*(10-duty)*0.1; //Ajusta o PR1
	}
	else
	{
		_LATE1 = 1; //Habilita a ventoinha
		PR1 = T*(duty)*0.1; //Ajusta o PR1
	}

}

void __attribute__ ((interrupt, no_auto_psv)) _IC1Interrupt(void)
{
	_IC1IF = 0; //zera o flag de interrupção
	voltas++; //Incrementa o número de voltas da ventoinha
}

void __attribute__ ((interrupt, no_auto_psv)) _T2Interrupt(void)
{
	_T2IF = 0; //Zera o flag de interrupção
	_RB6 = ~_RB6; //Altera valor RB6
	_RD0 = _RB6; //Valor Input Capture 1
}

void __attribute__ ((interrupt, no_auto_psv)) _T3Interrupt(void)
{
	_T3IF = 0; //Zera o flag de interrupção	
}

void __attribute__ ((interrupt, no_auto_psv)) _T4Interrupt(void)
{
	_T4IF = 0; //Zera o flag de interrupção	
	rpm = voltas; //numero de voltas 
   	voltas = 0; //Zera o numero de voltas
}

void __attribute__ ((interrupt, no_auto_psv)) _INT0Interrupt(void)
{
	//__delay_ms(50); //debounce
	IFS0bits.INT0IF = 0; //Zera o flag de interrupção
	est_ini = prox_est; //Proximo estado
	T1CON = T1CON | 0x8000; //ativamos o timer1
	T3CON = T3CON | 0x8000; //ativamos o timer3
	T4CON = T4CON | 0x8030; //ativamos o timer4
	TMR1 = 0;					//zera os timers
	TMR3 = 0;
	TMR4 = 0;
}

void __attribute__ ((interrupt, no_auto_psv)) _INT2Interrupt(void)
{
	__delay_ms(50); //debounce
	_INT2IF = 0; //Zera o flag de interrupção
	
}
	
void __criatividade1(); //Assinatura da criatividade

/************* Programa Principal *************************/
int main()
{
	ADPCFG = 0xFFFF; //configura a porta B (PORTB) como entradas/saidas digitais
	TRISB=0; //a PORTB como saída
	_TRISC13=1; //PORTC<13>=1; pino T2CK (infravermelho)de entrada
	TRISD=0x0001; //Saída Latch
	_TRISD1=1;
	_TRISE1=0; //Saida para ventoinha
	_TRISE3=0; //Saida do buzzer
	IFS0=0; //Flag de interrupção do timer1
	IEC0 = IEC0 | 0x00CB; // bit 3 do registrador IEC0 habilita a interrupção do timer1 (IEC0bits.T1IE=1), Habilita interrupção do Input Capture 1 
	IFS1 = 0; //Interrupção externa 1
	IEC1 = IEC1 | 0x0060; //Habilitando a interrupção externa 1 e timer 4
	_INT0IP = 2; //Prioridade 2 para a INT0

	IC1CON = 0; 
	IC1CONbits.ICTMR = 0; // Seleciona o Timer 3
	IC1CONbits.ICM = 1; // Captura em toda borda de subida e/ou descida 

	T1CON=0x0000; //Prescaler fica em 1.
	T2CON=0x8002; //habilita o timer 2 com TCS
	T3CON=0x0000; //timer 3 input capture
	T4CON=0x0030;//timer 4 prescaler 256

	PR1 = 3200; //O registrador de periodo PR1 é igual a 640 20% duty cycle
	PR2 = 7; //total para completar um volta da ventoinha
	PR3 = 0xFFFF; //input capture
	PR4 = 62500; //1s timer 4 prescaler 1:256
	T = 3200; //periodo para 5000Hz
	TMR1 = 0; //Zerar os timers
	TMR3 = 0;
	TMR4 = 0;
	_LATE0 = 0;

	init_lcd();

	while(1)//laço infinito
	{
		//FMS
		switch(est_ini)
		{
			case 0 : prox_est = 1; //Próximo estado
				     	lcd_gotoxy(1,1);
				     	lcd_puts("MODO: PARADA   \n");
				     	lcd_printf("%d    ",0);
				     	lcd_gotoxy(5,2);
					 	lcd_puts("        rpm");
					 	__delay_ms(150);
					 
					break;
			case 1 : prox_est = 2; //Próximo estado
					 duty = 8; //Duty Cycle 80%
	  		         if(_RD1 == 1){
					 	lcd_gotoxy(1,1);
				     	lcd_puts("MODO: VELOZ 80%\n");
			         	lcd_printf("%d    ",rpm*60);
			         	lcd_gotoxy(5,2);
				     	lcd_puts("        rpm");
				     	__delay_ms(150);}
					 else
					 	__criatividade1();		

					if(rpm >= 45)
						_buzzer = 1;
					else
						_buzzer = 0;
					 
					 break;
			case 2 : prox_est = 3; //Próximo estado
					 duty = 5; //Duty Cycle 50%
				     if(_RD1 == 1){
					 	lcd_gotoxy(1,1);
				     	lcd_puts("MODO: MEDIA 50%\n");
			         	lcd_printf("%d    ",rpm*60);
			         	lcd_gotoxy(5,2);
				     	lcd_puts("        rpm");
				     	__delay_ms(150);}		 
   					 
					 else
					 	__criatividade1();

					if(rpm >= 45)
						_buzzer = 1;
					else
						_buzzer = 0;

					 break;
			case 3 : prox_est = 1;
					 duty = 3; //Duty Cycle 30%
					 if(_RD1 == 1){
			         	lcd_gotoxy(1,1);
				     	lcd_puts("MODO: LENTA 30%\n");
			         	lcd_printf("%d    ",rpm*60);
			         	lcd_gotoxy(5,2);
				     	lcd_puts("        rpm");
				     	__delay_ms(150);}		
					 else
					 	__criatividade1();

					if(rpm >= 45)
						_buzzer = 1;
					else
						_buzzer = 0;

					 break;
		}

	}
}

void __criatividade1()
{
	if(rpm < 18) {
		lcd_gotoxy(1,1);
		lcd_printf("||||||          ");
		lcd_putc('\n');
		lcd_printf("||||||          ");}	
	if(rpm <27 && rpm >= 18) {
		lcd_gotoxy(1,1);
		lcd_printf("||||||||||      ");
		lcd_putc('\n');
		lcd_printf("||||||||||      ");}			
	if(rpm<40 && rpm >= 28) {
		lcd_gotoxy(1,1);
		lcd_printf("||||||||||||||||");
		lcd_putc('\n');
		lcd_printf("||||||||||||||||");}	
}
