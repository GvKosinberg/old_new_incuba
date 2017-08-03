/**
*@page Power_Control
* <b>Power control realization</b><br>
*<i>
*<ul> 
*<li> Universal.
*<ul> 
*</i>
*/

#define WorkMode_TimeLim 150

typedef struct
{
	unsigned char fFront;
	unsigned char fPrevPowerDetect;
	enum {V220, V12} WorkMode;
	unsigned char WorkMode_Counter;
	unsigned char TimerReg;
	unsigned int PWM_Timer;
	signed int PWM_Period;
	unsigned char fPowerOn;
	unsigned int PWM_Value;
	unsigned char ImpDelayTimer;
} PWM_PARAM;

PIN_CONFIG Semistor_Heater= {(uint8_t*) &PORTD,(uint8_t*) &PIND,(uint8_t*) &DDRD, 4};
PIN_CONFIG Akkumulator= {(uint8_t*) &PORTD,(uint8_t*) &PIND,(uint8_t*) &DDRD, 5};
PIN_CONFIG PowerDetector= {(uint8_t*) &PORTC,(uint8_t*) &PINC,(uint8_t*) &DDRC, 3};

void SetPWM (PWM_PARAM* pPWM) 
{
}


inline void PowerControl_Init(PWM_PARAM* pPWM, signed int PWM_Period)
{
	pPWM->PWM_Period=PWM_Period;
	
	PinSetOut(&Akkumulator, 0);
	PinSetOut(&Semistor_Heater, 1); //off, inverted
}

void PowerControl_Step (PWM_PARAM* pPWM)
{	
	//! detecting rising edge on PowerDetector pin, this means 220V is oscillating
		
/*	if ((PinSetIn_Read(&PowerDetector)==1)&&(pPWM->fPrevPowerDetect==0)) pPWM->fFront=1;
	else pPWM->fFront = 0;
	pPWM->fPrevPowerDetect = PinSetIn_Read(&PowerDetector);
*/

	if ((PinSetIn_Read(&PowerDetector)==1)&&(pPWM->fPrevPowerDetect==0)) pPWM->ImpDelayTimer=0;  
	else pPWM->fFront = 0;
	if (pPWM->ImpDelayTimer==7) pPWM->fFront=1;
	if (pPWM->ImpDelayTimer<200) pPWM->ImpDelayTimer++; 
	pPWM->fPrevPowerDetect = PinSetIn_Read(&PowerDetector);


//	if ((PinSetIn_Read(&PowerDetector)==1)&&(pPWM->fFront==0)) pPWM->fFront=1;
//	else pPWM->fFront=0;
	
	//! if no front on 220V for WorkMode_TimeLim cycles, then assumes 12V mode
	if (pPWM->fFront) {pPWM->WorkMode=V220; pPWM->WorkMode_Counter=WorkMode_TimeLim;}
	else 
	{
		if (pPWM->WorkMode_Counter > 0) pPWM->WorkMode_Counter--;
		else pPWM->WorkMode=V12;
	}

	//! cycling pwm timer from 0 to period
	if (pPWM->PWM_Timer < pPWM->PWM_Period) pPWM->PWM_Timer++;
	else pPWM->PWM_Timer=0;

	// using current pwm value, fPowerOn flag is set
	if (( pPWM->PWM_Timer >=0 ) && ( pPWM->PWM_Timer < pPWM->PWM_Value )) pPWM->fPowerOn = 1;  
	else  pPWM->fPowerOn = 0;

	//! generating control signals depending on fPowerOn and WorkMode
	//! for 220V control signal is syncronized with fFront to make half-period power
	if (pPWM->fPowerOn!=1)
	{
		PinSetOut(&Semistor_Heater, 1); //off, inverted
		PinSetOut(&Akkumulator, 0);		
	}
	else
	{
		if (pPWM->WorkMode == V12) 
		{
			PinSetOut(&Semistor_Heater, 1); //off, inverted
			PinSetOut(&Akkumulator, 1);
		}
		else
		{
			PinSetOut(&Akkumulator, 0);
			if (pPWM->fFront==1)
			{
				pPWM->TimerReg=6;				//13		19.05.11
				PinSetOut(&Semistor_Heater, 0); //on, inverted
			}
			else 
			{
			if (pPWM->TimerReg==0)
				{
				PinSetOut(&Semistor_Heater, 1); //off, inverted
				}
			else pPWM->TimerReg--;	
			}
		}
	}
}
