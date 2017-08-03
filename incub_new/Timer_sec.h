/**
*@page Timer
* <b>Timer second realization</b><br>
*<i>
*<ul> 
*<li> Universal.
*<ul> 
*</i>
*/

typedef struct
{
unsigned int TimerSec;
unsigned int TimerSec_Limit;
unsigned int TimerSec_WorkPeriod;
unsigned int Timer;
} TIMER;

inline void Timer_Init(unsigned int TimerSec_Value, unsigned int WorkPeriod_Value, TIMER* pTimer)
{
pTimer->TimerSec=TimerSec_Value;
pTimer->TimerSec_Limit=TimerSec_Value;
pTimer->TimerSec_WorkPeriod=WorkPeriod_Value;
}

unsigned char Timer_Step(TIMER* pTimer)
{
	pTimer->Timer++;						//Таймер автопереворота (мсек)
	if (pTimer->Timer>=1000)				
		{
		pTimer->TimerSec++;				//Тамер автопереворота (сек)
		pTimer->Timer=0;
		}
	if (pTimer->TimerSec > pTimer->TimerSec_Limit)
		{
		return 1;
		if (pTimer->TimerSec > (pTimer->TimerSec_Limit+pTimer->TimerSec_WorkPeriod))
			{
			pTimer->TimerSec=0;
			return 0;
			}
		}
	return 1;
}
