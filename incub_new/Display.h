/**
*@page Dysplay
* <b>Dysplay realization</b><br>
*<i>
*<ul> 
*<li> Universal.
*<ul> 
*</i>
*/

//!Structure DISPLAY_DATA<br> 
//!unsigned char SymbolData [4] in ASCII codes, +1 char to save trailing zero - end of string
//!unsigned char SymbolNum change from 0 to 2 (3 sym)
//!unsigned char SymbolBrightnessCounter from 0 to 4. At 0 symbol lights, at other values not
//!unsigned char fOverallBrightness 0 or 1, 0 means dark, 1 means bright
//!unsigned int  OverallBrightnessCounter counts cycles to change brightness from bright to dark and back.
typedef struct
{
unsigned char SymbolData [4];			
unsigned char SymbolNum;				
unsigned char SymbolBrightnessCounter;	

unsigned char fOverallBrightness;		
unsigned int  OverallBrightnessCounter; 
} DISPLAY_DATA;

/**@function InitDisplay 
 Инициализация портов для отображения.
 Конфигурирует используемые ножки
*/

#define segA 0x01
#define segB 0x02
#define segC 0x04
#define segD 0x08
#define segE 0x10
#define segF 0x20
#define segG 0x40
#define segPT 0x80
#define segNO 0x00

static const unsigned char ASCIIto7 [0x80] = 
{
	segNO, segNO, segNO, segNO, segNO, segNO, segNO, segNO, 
	segNO, segNO, segNO, segNO, segNO, segNO, segNO, segNO, // 00h - 0Fh
	segNO, segNO, segNO, segNO, segNO, segNO, segNO, segNO, 
	segNO, segNO, segNO, segNO, segNO, segNO, segNO, segNO, // 10h - 1Fh
	segNO, segNO, segNO, segNO, segNO, segNO, segNO, segNO, 
	segNO, segNO, segNO, segNO, segNO, segNO | segG,	//0x2d - "-"
	segNO, segNO, // 20h - 2Fh , space-20

	// 30h -  0  to  39h -  9                                          
	segNO | segA | segB | segC | segD | segE | segF, 		//0
	segNO | segB | segC ,									//1
	segNO | segA | segB | segD | segE | segG ,				//2
	segNO | segA | segB | segC | segD | segG ,				//3
	segNO | segB | segC | segF | segG ,						//4
	segNO | segA | segC | segD | segF | segG ,				//5
	segNO | segA | segC | segD | segE | segF | segG ,		//6
	segNO | segA | segB | segC ,							//7
	segNO | segA | segB | segC | segD | segE | segF | segG, //8
	segNO | segA | segB | segC | segD | segF | segG ,		//9

	segNO, segNO, segNO, segNO, segNO, segNO, 				// 3Ah - 3Fh
	segNO, segNO, segNO, segNO, segNO, segNO, segNO, segNO, 
	segNO, segNO, segNO, segNO, segNO, segNO, segNO, segNO,	// 40h - 4Fh
	segNO, segNO, segNO, segNO, segNO, segNO, segNO, segNO, 
	segNO, segNO, segNO, segNO, segNO, segNO, segNO, segNO, // 50h - 5Fh
	segNO, // 60h

	// 61h - segA to 7Ah - z
	segNO | segA | segB | segC | segE | segF | segG,//a
	segNO | segC | segD | segE | segF | segG ,		//b
	segNO | segA | segD | segE | segF ,				//c
	segNO | segB | segC | segD | segE | segG ,		//d
	segNO | segA | segD | segE | segF | segG ,		//e
	segNO | segA | segE | segF | segG ,				//f
	segNO | segA | segC | segD | segE | segF ,		//g
	segNO | segC | segE | segF | segG ,				//h
	segNO | segE ,									//i
	segNO | segB | segC | segD,						//j
	segNO | segB | segD | segE | segF | segG,		//k
	segNO | segD | segE | segF ,					//l
	segNO | segA | segB | segC | segE | segF ,		//m
	segNO | segC | segE | segG ,					//n
	segNO | segC | segD | segE | segG ,				//o
	segNO | segA | segB | segE | segF | segG ,		//p
	segNO | segA | segB | segC | segF | segG ,		//q
	segNO | segE | segG ,							//r
	segNO | segA | segC | segD | segF | segG,		//s
	segNO | segD | segE | segF | segG ,				//t
	segNO | segC | segD | segE ,					//u
	segNO | segB | segE | segF | segG ,				//v
	segNO | segB | segC | segD | segE | segF | segG,//w
	segNO | segB | segC | segE | segF | segG ,		//x
	segNO | segB | segC | segD | segF | segG ,		//y
	segNO | segA | segD | segG						//z
};


inline void Display_Init(unsigned int DTimer, DISPLAY_DATA* pDisp)
{
	DDRB = 0xFF;
	DDRD |= 0x07;
}


void Display_Step (const PWM_PARAM * pData, DISPLAY_DATA* pDisp)
{
	//! counts symbol and brightness counter inside it
	if (pDisp->SymbolBrightnessCounter < 3) 
		pDisp->SymbolBrightnessCounter++;
	else 
	{
		pDisp->SymbolBrightnessCounter=0;
		if(pDisp->SymbolNum < 2) pDisp->SymbolNum++; else pDisp->SymbolNum=0;
	}
	//! counts overall brightness and toggle dark|bright flag
	if (pDisp->OverallBrightnessCounter < 1000) pDisp->OverallBrightnessCounter++;
	else
	{
		pDisp->OverallBrightnessCounter = 0;
		pDisp->fOverallBrightness^=1;
	}

	//! lights symbols
	PORTD=PORTD|0x07;
	SET_BIT(PORTD, (2-pDisp->SymbolNum), 0); //symbol at [0] means left position, symbol at [2] means right position

	if ((pData->WorkMode==V220) || (pDisp->fOverallBrightness==1) || (pDisp->SymbolBrightnessCounter==0))
		PORTB = ASCIIto7[pDisp->SymbolData[pDisp->SymbolNum]];
	else PORTB = 0;

	//! lights decimal point at [1] symbol
	if (pDisp->SymbolNum==1)  if ((pData->fPowerOn) || (pDisp->SymbolBrightnessCounter==0)) { SET_BIT (PORTB, 7, 1); }
}





