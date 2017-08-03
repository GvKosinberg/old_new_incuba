/**
*@page Pin_Control
* <b>Pin control realization</b><br>
*<i>
*<ul> 
*<li> Universal.
*<ul> 
*</i>
*/

typedef const struct
{
	unsigned char * Port;
	unsigned char * Pin;
	unsigned char * Ddr;
	unsigned char BitNum;
} PIN_CONFIG;

inline void PinSetOut (PIN_CONFIG *pPinConf, unsigned char Value);
inline unsigned char PinSetIn_Read(PIN_CONFIG *pPinConf);

inline void PinSetOut (PIN_CONFIG *pPinConf, unsigned char Value)
{
if(Value) *(pPinConf->Port)|= (1<<pPinConf->BitNum); 
else *(pPinConf->Port)&=(~(1<<pPinConf->BitNum));
}

inline unsigned char PinSetIn_Read(PIN_CONFIG *pPinConf)
{
return ( (*(pPinConf->Pin)) &  (1<<pPinConf->BitNum)) ? 1 : 0; 
}
