/**
*@mainpage Incubator
* <b>Temperature regulator program</b><br>
* F_CPU 8000000UL
*/
#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/eeprom.h>

#define SET_BIT(port,bit,value) if(value) port|=(1<<bit); else port&=(~(1<<bit));
//!defines PIN_CONFIG struct and IO functions as addition to SET_BIT     /*! */
#include "Pin_Control.h" 
//!defines PWM_PARAM struct, PowerControl Init and Step. Depends on PIN_CONFIG
#include "Power_Control.h" 
//!defines DISPLAY_DATA struct, Display Init and Step. Depends on PWM_PARAM.
#include "Display.h"
//!defines ONEWIRE_PARAM struct, Onewire Init and Step
#include "onewire.h" 
//!defines TIMER struct, Timer Init and Step
#include "Timer_sec.h" 
//!defines BUTTON struct, Button Init and Step
#include "Button.h" 
//!defines TEMPREADER_PARAM struct, Tempreader Init and Step, wich use ONEWIRE_PARAM struct
#include "Tempreader.h" 

//#include "ButtonState.h"

//!Semistor autooverturn
PIN_CONFIG Semistor_Perevorot= {(uint8_t*) &PORTD,(uint8_t*) &PIND,(uint8_t*) &DDRD, 3};
//!Button increment
PIN_CONFIG SBInc= {(uint8_t*)&PORTC,(uint8_t*) &PINC,(uint8_t*) &DDRC, 1};
//!Button decrement
PIN_CONFIG SBDec= {(uint8_t*)&PORTC,(uint8_t*) &PINC,(uint8_t*) &DDRC, 0};	
//-------------------------------------------------------------------------------------------------------------------
//! A structure BUTTON_DATA.
/*!  */
typedef struct
{
unsigned char Timer;	/*!< Pause between button events */
signed int TempStore;	/*!< Stored temperature value */
unsigned char DisplayTimeTimer; //Пауза для просмотра температуры уставки не записывая ничего	13.05.11							
} BUTTON_DATA;
//--------------------------------------------------------------------------------------------------------------------
//EEPROM
unsigned int EEMEM TemperatureStore=370;

uint8_t temp_ee [16] EEMEM;
uint8_t temp_ram [16];


//Program usage
static TEMPREADER_PARAM TempReader;										
static ONEWIRE_PARAM OneWire;	
static PWM_PARAM Pwm;
//static TIMER Perevorot;  Автопереворот не работает, поскольку таймер написан непонятно
static DISPLAY_DATA DisplayData;
static BUTTON ButInc;
static BUTTON ButDec;
static BUTTON_DATA ButtonCheck;

unsigned char OperationCounter;					
unsigned char But;
unsigned int Wire;											//ProgTimers

unsigned int PerevorotTimer_Sec=0;							//Auto overturn timer second's
unsigned int PerevorotTimer_mSec=0;							//Auto overturn timer msecond's
#define	PerevorotPeriod_Sec 1800
#define PerevorotWorkTime_Sec 30
#define Second 1000

//!function DysplayDataInput<br> 
//!DISPLAY_DATA * pDisp - pointer to the structure of dysplay control <br> 	
//!Function transmits data to the display
void DysplayDataInput(DISPLAY_DATA* pDisp, float Data);

void remember_me(int tempa)
{
	static uint8_t counter = 0;
	static uint8_t tempa_10 = 0;
	
	tempa_10 = (uint8_t)((tempa/100)*10+tempa%100/10);
	
	if (counter<5) 
	{
		temp_ram[3*counter]=((tempa_10%100)/10)+0x30;
		temp_ram[(3*counter)+1]=((tempa_10%100)%10)+0x30;
		temp_ram[(3*counter)+2] = '/';
		counter++;
	}
	else
	{
		for (int i=0; i<12; i++)
		{
			temp_ram[i]=temp_ram[i+3];
		}
		temp_ram[12]=((tempa_10%100)/10)+0x30;
		temp_ram[13]=((tempa_10%100)%10)+0x30;
		temp_ram[14] = '/';
	}
	
}

void update_ee()
{
	static uint8_t cnt = 0;
	//uint8_t metka = 'x';
	
	eeprom_update_byte(&temp_ee[cnt], temp_ram[cnt]);
	//eeprom_write_byte(&temp_ram[i+1], metka);
	
	if (cnt<15) cnt++;
	else cnt=0;
}

//!function ButtonStateCheck<br> 
//!BUTTON_DATA *pButtonCheck - pointer to the structure of button info <br>
//!BUTTON *pButInc,*pButDec - pointer to the structure of button <br> 	
//!Function check button events(pressings), save data in EEPROM and send data to display <br>
void ButtonStateCheck(BUTTON_DATA* pButtonCheck, BUTTON *pButInc, BUTTON *pButDec);

//! Main program function<br> 
int main(void)
{
	PORTC=0x03;
	DDRC=0x00;

	PORTD=0x00;
	DDRD=0xFF;

	//Timer0  initialization
	TCCR0=0x03;
	TCNT0=0x83;

	// Timers/Counters Interrupts initialization
	TIMSK=0x01;

	ButtonCheck.TempStore=eeprom_read_word(&TemperatureStore);

	OneWire_Init (&OneWire,(uint8_t*) &PORTC,(uint8_t*) &DDRC,(uint8_t*) &PINC, PC2);
	TempReader_Init (&TempReader,&OneWire);
	PowerControl_Init (&Pwm, 100);
	Display_Init (1000, &DisplayData);
//	Timer_Init (9000, 150, &Perevorot);
	Button_Init (&ButInc,&SBInc);
	Button_Init (&ButDec,&SBDec);
	sei();
	
	while(1) {};
}

ISR(TIMER0_OVF_vect) 
{	
	//Срабатывает раз в милисекунду
	//Enter to interrupt takes 30 us @ 8MHz 
	TCNT0=0x83;								//Реинициализация таймера
	OperationCounter=0;						//Флаг разрешения выполнения операций по таймеру
	But++;									//Таймер кнопок
	Wire++;									//Таймер 1-wire


	//Автопереворот
	//Autooverturn
//	if (Timer_Count(&Perevorot)==1) {PinSetOut(&Semistor_Perevorot, 0);PinSetOut(&DigitOut1, 1);}
//	else {PinSetOut(&Semistor_Perevorot, 1);PinSetOut(&DigitOut1, 0);}
	if (PerevorotTimer_Sec>=PerevorotPeriod_Sec) PerevorotTimer_Sec=0;
	if (PerevorotTimer_Sec<PerevorotWorkTime_Sec) PinSetOut(&Semistor_Perevorot, 0);
	else PinSetOut(&Semistor_Perevorot, 1);
	PerevorotTimer_mSec++;
	if (PerevorotTimer_mSec>=Second) {PerevorotTimer_Sec++;PerevorotTimer_mSec=0;}
		
	/////////eeeprom///////
	update_ee();
	//////////////////////


	// 1-wire
	TempReader_Step (&TempReader);
	OneWire_StateMachineStep(TempReader.pOneWireBus);		

	//Алгоритм управления и регулятор
	if (TempReader.Tmpr < ButtonCheck.TempStore) 
		Pwm.PWM_Value = 102;
	else Pwm.PWM_Value = 0;

	PowerControl_Step (&Pwm);
	
	Display_Step(&Pwm, &DisplayData);
	
	//Button 
	Button_Step (&ButInc);									//Опрос и выставление флага события
	Button_Step (&ButDec);									
															//Обработка событий
	if ((But>100)&&(OperationCounter==0))					//время исполнения кода 485мкС
	{
		ButtonStateCheck(&ButtonCheck, &ButInc, &ButDec);
		But=0;
		OperationCounter=1;
	}
																//Датчик 1-wire
	if ((Wire>1000)&&(OperationCounter==0))						
	{		
		Wire=0;
		if ((ButtonCheck.Timer>150)&&(ButtonCheck.DisplayTimeTimer==0))	//время исполнения кода 230 мкС   13.05.11
		{
			if ((TempReader.Tmpr<1000)&&(TempReader.Tmpr>-550)) 
			{
				sprintf ( (char*) &DisplayData.SymbolData[0], "%03d", TempReader.Tmpr);
				remember_me(TempReader.Tmpr);
			}
			else
			{ 
				sprintf ( (char*) &DisplayData.SymbolData[0], "06p" );
			}
		}
	}
}

//!function ButtonStateCheck<br> 
//!BUTTON_DATA *pButtonCheck - pointer to the structure of button info <br>
//!BUTTON *pButInc,*pButDec - pointer to the structure of button <br> 	
//!Function check button events(pressings), save data in EEPROM and send data to display <br>
void ButtonStateCheck(BUTTON_DATA* pButtonCheck, BUTTON *pButInc, BUTTON *pButDec)
{
	if ((pButtonCheck->Timer==200)&&(pButtonCheck->DisplayTimeTimer==0))	//13.05.11
	{
	if ((pButDec->Event==1)||(pButInc->Event==1))
		{
		pButtonCheck->DisplayTimeTimer=25;
		pButtonCheck->Timer=160;
		pButInc->Event=0;
		pButDec->Event=0;
		}
	}
	pButtonCheck->Timer++;
	if (pButtonCheck->DisplayTimeTimer==0)
	{
	if (pButInc->Event==1) 
		{
		if (pButtonCheck->TempStore<450) //1250
			{
			pButInc->Event=0;
			pButtonCheck->TempStore++;
			if (pButInc->ButtonSpeed>5) {pButtonCheck->TempStore+=4;};
			if (pButInc->ButtonSpeed>10) {pButtonCheck->TempStore+=5;};
			pButInc->Event=0;
			pButtonCheck->Timer=0;
			}
		}
	if (pButDec->Event==1) 
		{
		if (pButtonCheck->TempStore>250) //-550
			{
			pButtonCheck->TempStore--;
			if (pButInc->ButtonSpeed>5) {pButtonCheck->TempStore-=4;};
			if (pButInc->ButtonSpeed>10) {pButtonCheck->TempStore-=5;};
			pButDec->Event=0;
			pButtonCheck->Timer=0;
			}
		}
	}
	if ((pButtonCheck->Timer)>8)
		{
		pButInc->ButtonSpeed=0;
		pButDec->ButtonSpeed=0;
		}
	if ((pButtonCheck->Timer)==150)
		{
		eeprom_write_word(&TemperatureStore,pButtonCheck->TempStore);
		}
	if ((pButtonCheck->Timer)>200) {pButtonCheck->Timer=200;}
//---------------------------Вывод установленной температуры на дисплей-------------------------------
	if ((pButtonCheck->Timer<150)||(pButtonCheck->DisplayTimeTimer>0))					//13.05.11
	{
		sprintf ( (char*) &DisplayData.SymbolData[0], "%03d", pButtonCheck->TempStore);
	}
//----------------------------------------------------------------------------------------------------
	if (pButtonCheck->DisplayTimeTimer>0) {pButtonCheck->DisplayTimeTimer--;}			//13.05.11
	else pButtonCheck->DisplayTimeTimer=0;
}




