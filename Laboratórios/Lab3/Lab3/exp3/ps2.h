//********* Variáveis Globais *******************************
#define PS2DAT PORTCbits.RC13 // entrada no pino RC13 dos dados do teclado PS/2
#define PS2CLK PORTCbits.RC14 // entrada no pino RC14 do clock do teclado PS/2

// definição da máquina de estado do teclado PS/2
#define PS2START 0
#define PS2BIT 1
#define PS2PARITY 2
#define PS2STOP 3

// máquina de estado do KBD PS2 e buffer
int PS2State;
unsigned char KBDBuf;
int KCount, KParity;
// flags de mensagem (mailbox)
volatile int KBDReady;
volatile unsigned char KBDCode;

void (*KBDfunc)(void);

void initKBD( void)
{	
	KBDReady = 0;  // zeramos o flag
	CNEN1 = 0x0001;  // habilitamos o Change Notification 0.
	_CNIF = 0; // zerar o flag de interrupção do CN
	_CNIE = 1; // habilitar a interrupção CN0
} // void initKBD

void setKBDFunc(void (*funcP)(void)){
	KBDfunc = funcP;
}

//************ Interrupção do Change Notification ***********************
void __attribute__ ((interrupt, no_auto_psv)) _CNInterrupt( void)
{ // Nesta subrotina devemos ter certeza que a borda do sinal é de descida
	if ( PS2CLK == 0)
	{
		// máquina de estado da recepção do PS/2
		switch( PS2State)
			{
				default:
				case PS2START:
					if ( ! PS2DAT)
						{
							KCount = 8; // inicia o contador de bits
							KParity = 0; // inicia a checagem de paridade
							PS2State = PS2BIT;
						}
				break;
				case PS2BIT:
					KBDBuf >>=1; // desloca para dentro o bit de dado
					if ( PS2DAT)
						KBDBuf += 0x80;
					KParity ^= KBDBuf; // atualiza paridade
					if ( --KCount == 0) // se todos os bits foram lidos, se move
						PS2State = PS2PARITY;
				break;
				case PS2PARITY:
					if ( PS2DAT)
						KParity ^= 0x80;
					if ( KParity & 0x80) // se paridade é impar, continua
						PS2State = PS2STOP;
					else
						PS2State = PS2START;
				break;
				case PS2STOP:
					if ( PS2DAT) // verificao bit de parada
					{
						KBDCode = KBDBuf; // salva o codigo da tecla
						KBDReady = 1; // seta o flag, o codigo da tecla está disponível
						(*KBDfunc)();
					}
					PS2State = PS2START;	//volta para o inicio
				break;
			} // comuta máquina de estado
	} // se(if) borda de descida
	// zera o flag de interrupção
	_CNIF = 0;
} //  Interrupção CN
