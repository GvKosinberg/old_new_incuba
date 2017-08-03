/**
*@page Button
* <b>Event pressing stack</b><br>
*<i>
*<ul> 
*<li> Universal.
*<ul> 
*</i>
*/

typedef struct 	//Состояние кнопок
{
	PIN_CONFIG *ButtonPin;
	unsigned char ButtonCount;
	unsigned char ButtonSpeed;
	unsigned char Event;
} BUTTON;

inline void Button_Init (BUTTON *pButton, PIN_CONFIG *pButtonPin)
{
pButton->ButtonPin=pButtonPin;
}

inline void Button_Step(BUTTON *pButState)
{
if (PinSetIn_Read(pButState->ButtonPin)==0) pButState->ButtonCount++;
else pButState->ButtonCount=0;
if (pButState->ButtonCount>200) {pButState->Event=1;pButState->ButtonSpeed++;pButState->ButtonCount=0;}
if (pButState->ButtonSpeed>15) pButState->ButtonSpeed=15;
}
