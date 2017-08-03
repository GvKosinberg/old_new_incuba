/**
*@page OneWire
* <b>1-wire protocol realization</b><br>
*<i>
*<ul> 
*<li> Universal.
*<ul> 
*</i>
*/

//---------------------- ONE WIRE VARIABLES, STATE AND FLAGS -----------------//

typedef enum 
{
	IDLE,
	//reset metastate
	RESET_PULSE,
	RESET_PRESENCE,
	//general IO metastate
	BIT_IO,
	BIT_RECOVERY

} ONEWIRE_STATE;

typedef struct 
{
	unsigned char *pBusPort;
	unsigned char *pBusDdr;
	unsigned char *pBusPin;
	unsigned char BusBit;

	unsigned char * Buf; // buffer for output-input data
	unsigned char Cnt; 
	unsigned char Pulse_timer;
	unsigned char Index;
	unsigned char Mask;
	unsigned char Data;

	ONEWIRE_STATE State;
	unsigned char fIO;

} ONEWIRE_PARAM;

/**@function InitOneWire 
 Инициализация 1-wire
 Parameters:
 @param pBusPort - указатель на регистр PORT порта шины,
 @param pBusDdr - указатель на регистр DDR порта шины,
 @param pBusPin - ножка на которой находится шина   
*/
//void OneWire_Init(ONEWIRE_PARAM *Param, 
//				unsigned char *pBusPort, unsigned char *pBusDdr, 
//				unsigned char *pBusPin,  unsigned char BusBit)

/**@function OneWire
 Машина состояния
*/
//void OneWire_StateMachineStep(ONEWIRE_PARAM *Param);
//как часто она должна вызываться - требуется указать ограничение

/**@function Reset
 Функция сброса/присутствия
 @param pBusDdr - указатель на DDR порт шины,
 @param pBusPin - ножка шины 1-wire
*/
//void OneWire_CommandReset(ONEWIRE_PARAM *Param);

/**@function IO
 Функция ввода/вывода.
 При чтении шлется команда вида 0xFF
 @param pBuf - команда,
 @param pBusPort - указатель на регистр PIN порта шины
 @param pBusDdr - указатель на регистр DDR порта шины
 @param pBusPin - ножка на которой находится шина
*/
//void OneWire_CommandIO(ONEWIRE_PARAM *Param, unsigned char *pBuf, unsigned char nCnt);

//unsigned char OneWire_IsCommandDone(ONEWIRE_PARAM *Param);


//-------------------------------- INIT ----------------------------------//

inline void OneWire_Init(ONEWIRE_PARAM *Param, 
				unsigned char *pBusPort, unsigned char *pBusDdr, 
				unsigned char *pBusPin,  unsigned char BusBit)
{
	SET_BIT(*pBusPort, BusBit,0);
	SET_BIT(*pBusDdr, BusBit,0);

	Param->pBusPort = pBusPort;
	Param->pBusDdr = pBusDdr;
	Param->pBusPin = pBusPin;
	Param->BusBit = BusBit;
	Param->State = IDLE;
}


//------------------------------ INTERFACE ------------------------------//
inline unsigned char OneWire_IsCommandDone(ONEWIRE_PARAM *Param)
{
	if (Param->fIO == IDLE) return 1;
	else return 0;
}


void OneWire_CommandIO(ONEWIRE_PARAM *Param, unsigned char *pBuf, unsigned char nCnt)
{
	if (Param->fIO != 0) return;
	Param->Buf = pBuf;
	Param->Cnt = nCnt;
	Param->fIO = 1;
}


//---------------------------- STATE MACHINE -----------------------------//

void OneWire_StateMachineStep(ONEWIRE_PARAM *Param)
{
	if (Param->Pulse_timer>0) Param->Pulse_timer--;

	switch (Param->State)
	{
	case IDLE:
		if (Param->fIO) 
		{
			SET_BIT(*(Param->pBusDdr), Param->BusBit, 1);//pin_lo
			Param->Pulse_timer = 5;
			Param->State = RESET_PULSE;
		}
	break;

	case RESET_PULSE:
		if (Param->Pulse_timer == 0) 
		{
			SET_BIT(*(Param->pBusDdr), Param->BusBit, 0);//pin_hi
			Param->Pulse_timer = 5;
			Param->State = RESET_PRESENCE;
		}
	break;

	case RESET_PRESENCE:
		if (Param->Pulse_timer == 0) 
		{
			Param->Index = 0;
			Param->Mask = 0x01;
			Param->State = BIT_IO;
		}
	break;

	case BIT_IO:
		if (Param->Buf[Param->Index] & Param->Mask)
		{ //1
			SET_BIT(*(Param->pBusDdr), Param->BusBit, 1);//pin_lo
			_delay_us(2); 								//short pulse
			SET_BIT(*(Param->pBusDdr), Param->BusBit, 0);//pin_hi
			_delay_us(13);
			if (!(*(Param->pBusPin) & (1<<Param->BusBit))) (Param->Buf[Param->Index]^= Param->Mask);
		}
		else
		{ 
			SET_BIT(*(Param->pBusDdr), Param->BusBit, 1);//pin_lo
			_delay_us(65);								// 60<t<120 us, else Reset!
			SET_BIT(*(Param->pBusDdr), Param->BusBit, 0);//pin_hi
		}
		Param->State = BIT_RECOVERY;
	break;

	case BIT_RECOVERY:
		SET_BIT(*(Param->pBusDdr), Param->BusBit, 0);	//pin_hi
		Param->Mask<<=1;
		if (Param->Mask == 0) 
		{
			Param->Mask = 0x01;
			if (++Param->Index >= Param->Cnt)
			{
				Param->State = IDLE;
				Param->fIO = 0;
				break;
			}
		}
		Param->State = BIT_IO;
	break;
	} 
}


