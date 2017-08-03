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
unsigned char DisplayTimeTimer; //����� ��� ��������� ����������� ������� �� ��������� ������	13.05.11							
} BUTTON_DATA;
//--------------------------------------------------------------------------------------------------------------------
//EEPROM
unsigned int EEMEM TemperatureStore=370;


//Program usage
static TEMPREADER_PARAM TempReader;										
static ONEWIRE_PARAM OneWire;	
static PWM_PARAM Pwm;
//static TIMER Perevorot;  ������������� �� ��������, ��������� ������ ������� ���������
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
	//����������� ��� � �����������
	//Enter to interrupt takes 30 us @ 8MHz 
	TCNT0=0x83;								//��������������� �������
	OperationCounter=0;						//���� ���������� ���������� �������� �� �������
	But++;									//������ ������
	Wire++;									//������ 1-wire


	//�������������
	//Autooverturn
//	if (Timer_Count(&Perevorot)==1) {PinSetOut(&Semistor_Perevorot, 0);PinSetOut(&DigitOut1, 1);}
//	else {PinSetOut(&Semistor_Perevorot, 1);PinSetOut(&DigitOut1, 0);}
	if (PerevorotTimer_Sec>=PerevorotPeriod_Sec) PerevorotTimer_Sec=0;
	if (PerevorotTimer_Sec<PerevorotWorkTime_Sec) PinSetOut(&Semistor_Perevorot, 0);
	else PinSetOut(&Semistor_Perevorot, 1);
	PerevorotTimer_mSec++;
	if (PerevorotTimer_mSec>=Second) {PerevorotTimer_Sec++;PerevorotTimer_mSec=0;}


	// 1-wire
	TempReader_Step (&TempReader);
	OneWire_StateMachineStep(TempReader.pOneWireBus);		

	//�������� ���������� � ���������
	if (TempReader.Tmpr < ButtonCheck.TempStore) 
		Pwm.PWM_Value = 102;
	else Pwm.PWM_Value = 0;

	PowerControl_Step (&Pwm);
	
	Display_Step(&Pwm, &DisplayData);
	
	//Button 
	Button_Step (&ButInc);									//����� � ����������� ����� �������
	Button_Step (&ButDec);									
															//��������� �������
	if ((But>100)&&(OperationCounter==0))					//����� ���������� ���� 485���
	{
		ButtonStateCheck(&ButtonCheck, &ButInc, &ButDec);
		But=0;
		OperationCounter=1;
	}
																//������ 1-wire
	if ((Wire>1000)&&(OperationCounter==0))						
	{		
		Wire=0;
		if ((ButtonCheck.Timer>150)&&(ButtonCheck.DisplayTimeTimer==0))	//����� ���������� ���� 230 ���   13.05.11
		{
			if ((TempReader.Tmpr<1000)&&(TempReader.Tmpr>-550)) 

				sprintf ( (char*) &DisplayData.SymbolData[0], "%03d", TempReader.Tmpr);
			else 
				sprintf ( (char*) &DisplayData.SymbolData[0], "06p" );
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
//---------------------------����� ������������� ����������� �� �������-------------------------------
	if ((pButtonCheck->Timer<150)||(pButtonCheck->DisplayTimeTimer>0))					//13.05.11
	{
		sprintf ( (char*) &DisplayData.SymbolData[0], "%03d", pButtonCheck->TempStore);
	}
//----------------------------------------------------------------------------------------------------
	if (pButtonCheck->DisplayTimeTimer>0) {pButtonCheck->DisplayTimeTimer--;}			//13.05.11
	else pButtonCheck->DisplayTimeTimer=0;
}




