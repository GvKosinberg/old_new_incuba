/*
static struct ButtonState	//Состояние кнопок
{
	unsigned char BT1;
	unsigned char BT2;
	unsigned char State;
	unsigned int Timer;
	unsigned char Summ;
	unsigned char ButtonSpeed;
	unsigned char Mode;
} ButtonStat={0};
*/
//void ButtonStateCheck();
//ButtonStat={0};
/*
void ButtonStateCheck()
{
float DataTempStore;//Для вывода на дисплей

if (!(PINC & (1<<SB1))) ButtonStat.BT1++;
if (!(PINC & (1<<SB2))) ButtonStat.BT2++;

		ButtonStat.Timer++;//Таймер нажимания кнопок
		if (ButtonStat.BT1>2) 
			{
			if (TempStore<1250) 
				{
				TempStore++;
				ButtonStat.ButtonSpeed++;
				if (ButtonStat.ButtonSpeed>9) {TempStore+=4;};
				if (ButtonStat.ButtonSpeed>19) {TempStore+=5;};
				}
			ButtonStat.BT1=0;
			ButtonStat.Timer=0;
			}
		if (ButtonStat.BT2>2) 
			{
			if (TempStore>-500) 
				{
				TempStore--;
				ButtonStat.ButtonSpeed++;
				if (ButtonStat.ButtonSpeed>9) {TempStore-=4;};
				if (ButtonStat.ButtonSpeed>19) {TempStore-=5;};
				}
			ButtonStat.BT2=0;
			ButtonStat.Timer=0;
			}

		if (ButtonStat.Timer==100)
			{
			ButtonStat.BT1=0;
			ButtonStat.BT2=0;
			eeprom_write_word(&TemperatureStore,TempStore);
			} 
		if (ButtonStat.Timer>2500) {ButtonStat.Timer=101;}
		
if (ButtonStat.Summ==(ButtonStat.BT1+ButtonStat.BT2))
	{
	ButtonStat.BT1=0;
	ButtonStat.BT2=0;
	ButtonStat.ButtonSpeed=0;
	}
ButtonStat.Summ=ButtonStat.BT1+ButtonStat.BT2;
//---------------------------Вывод установленной температуры на дисплей-------------------------------
if ((ButtonStat.Timer<102)||(ButtonStat.Timer>2200))
	{
	ButtonStat.Mode=1;
	TempStoreWire=DataTemp*10;
	DataTempStore=((float)TempStore)/10;
	DysplayDataInput();
	}
else ButtonStat.Mode=0;
//-----------------------------------------------------------------------------------------------------
}
*/
