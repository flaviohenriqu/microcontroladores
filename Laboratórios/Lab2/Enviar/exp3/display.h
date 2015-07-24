#define FCY 16000000UL
#include <libpic30.h>
#include <p30f4011.h>


int x, y, w, z;
int mask (int num){	
	switch (num){
		case -1:return 0x40;
		case 0:return 0x3F; 	
		case 1:return 0x06;
		case 2:return 0x5B;
		case 3:return 0x4F;
		case 4:return 0x66;
		case 5:return 0x6D;
		case 6:return 0x7D;
		case 7:return 0x07;
		case 8:return 0x7F;
		case 9:return 0x6F;
		case 'd':return 0x5E;
		case 'S':return 0x6D;
		case 'P':return 0x73;
		case 'I':return 0x06;
		case 'C':return 0x39;
		case 'F':return 0x71;
		case 'E':return 0x79;
		case 'r':return 0x50;
		case 'O':return 0x3F;
		case 'H':return 0x76;
		case 'A':return 0x77;
		case 'n':return 0x54;
		case 'x': return 0x00;
		case 'L':return 0x38;
		default:return 0x3F;
	}
}
void __display(int x, int y, int w, int z)
{

	_LATD1 = 1;
	LATB = mask(x);		
	__delay_ms(1);
	_LATD1 = 0;

	_LATD2 = 1;
	LATB = mask(y);		
	__delay_ms(1);
	_LATD2 = 0;

	_LATD3 = 1;
	LATB = mask(w);
	__delay_ms(1);
	_LATD3 = 0;

	_LATD0 = 1;
	LATB = mask(z);
	__delay_ms(1);
	_LATD0 = 0;	
}
