/**
*@page Tempreader
* <b>1-wire interface realization</b><br>
*<i>
*<ul> 
*<li> Universal for DS1820.
*<ul> 
*</i>
*/

typedef struct
{
	//TEMPREADER_STATE State;
	enum  {DataWait, TrasformWait, SecondWait} State;
    signed int Tmpr; // fixed point value in scale 0,1C = 1;
	ONEWIRE_PARAM* pOneWireBus;
	unsigned char CommandBuffer[11];
	unsigned int Second;
} TEMPREADER_PARAM;

void TempReader_Init (TEMPREADER_PARAM *Param,ONEWIRE_PARAM* OneWireBus)
{
	Param->pOneWireBus=OneWireBus;
	Param->CommandBuffer[0]=0xCC;
	Param->CommandBuffer[1]=0x44;
	OneWire_CommandIO (Param->pOneWireBus, &(Param->CommandBuffer[0]), 0x02);
	OneWire_StateMachineStep(Param->pOneWireBus);
	while (OneWire_IsCommandDone(Param->pOneWireBus)==0) 
		{
		_delay_us(100);
		OneWire_StateMachineStep(Param->pOneWireBus);
		}
	Param->Second=900;
	Param->State=SecondWait;
}

char w1_crc8(void *p, char n)
{
	char *pp;																										//Вспомогательный указатель
	char buf[11];																									//Вспомогательный массив (Необходим чтобы в основной программе не портить значения массива)
	char CRC=0;																										//Вычисляемая контрольная сумма
	char a=0, b=0;																									//Вспомогательные переменные для преобразования полинома
	int temp,temp2;																									//Вспомогательные переменные
	
	pp=p;																											//Настройка вспомогательного указателя на основной
	for (temp=0; temp<n; temp++)																					//Перекопирование участка памяти в буфер для сохранности данных (в результате работы функции буфер будет обнулен)
	{
		buf[temp]=*pp;
		pp++;
	}

	//--------------------- Расчет CRC свертки с использованием полинома X8+X5+X4+1 ----------------------
	for (temp=0; temp<n; temp++)
	for (temp2=0; temp2<8; temp2++)
	{
		a=buf[temp];
		a^=CRC;
		if ((a & 0x01)!=0) CRC=((CRC^0x18)>>1) | 0x80;
		else CRC>>=1;
		a=(a>>1) | b;
		if ((buf[temp] & 0x01)!=0) b=0x80;
		else b=0;
		buf[temp]>>=1;
	}
	
	return CRC;																										//Возврат расчитанной CRC
}

void TempReader_Step (TEMPREADER_PARAM *Param)
{
	static signed int * temp = 0;
	static uint8_t err_cntr = 0;
 	static char crc_calc = 0;
	static char crc_snc = 0;
	static char buf_temp [8];

	switch (Param->State)
	{
	case DataWait:
	{
		if (OneWire_IsCommandDone(Param->pOneWireBus)==1)
		{
			for (int i = 0; i<8; i++) {buf_temp[i] = Param->CommandBuffer[i+2];}
   			crc_calc = w1_crc8(&buf_temp, 8);
			crc_snc = Param->CommandBuffer[10];
  			if (crc_snc == crc_calc)
  			{
				err_cntr = 0;
				temp = (signed int *) (&buf_temp[0]);//2->8
 				Param->Tmpr = (*temp)*0.625;
 				//Param->Tmpr = ((*temp)*0.625)-5;											//для бракованных датчиков
  			}
			else
			{
				if (err_cntr<200)
				{
					err_cntr++;
					if (err_cntr>10)
					{
						Param->CommandBuffer[0]=0xCC;
						Param->CommandBuffer[1]=0x44;
						OneWire_CommandIO (Param->pOneWireBus, &(Param->CommandBuffer[0]), 0x02);
						OneWire_StateMachineStep(Param->pOneWireBus);
						while (OneWire_IsCommandDone(Param->pOneWireBus)==0)
						{
							_delay_us(100);
							OneWire_StateMachineStep(Param->pOneWireBus);
						}
						Param->Second=900;
						Param->State=SecondWait;

						*temp = 999;
						Param->Tmpr = (*temp);

						break;
					}
				}
			}
			Param->CommandBuffer[0]=0xCC;
			Param->CommandBuffer[1]=0x44;
			OneWire_CommandIO (Param->pOneWireBus, &(Param->CommandBuffer[0]), 0x02);

			Param->State=TrasformWait;
		}
		break;
	}
	case TrasformWait:
	{
		if (OneWire_IsCommandDone(Param->pOneWireBus)==1)
			{
			Param->State=SecondWait;		
			Param->Second=900;
			}
		break;
	}
	case SecondWait:
	{
		if (Param->Second>0) Param->Second--;
		else 
			{
			Param->CommandBuffer[0]=0xCC;
			Param->CommandBuffer[1]=0xBE;
			Param->CommandBuffer[2]=0xFF;
			Param->CommandBuffer[3]=0xFF;
			Param->CommandBuffer[4]=0xFF;
			Param->CommandBuffer[5]=0xFF;
			Param->CommandBuffer[6]=0xFF;
			Param->CommandBuffer[7]=0xFF;
			Param->CommandBuffer[8]=0xFF;
			Param->CommandBuffer[9]=0xFF;
			Param->CommandBuffer[10]=0xFF;

			OneWire_CommandIO (Param->pOneWireBus, &(Param->CommandBuffer[0]), 0x0B);
			Param->State=DataWait;
			}
		break;
	}
	}
}
