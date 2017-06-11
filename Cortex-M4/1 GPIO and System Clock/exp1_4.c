#include <stdint.h>
#include <stdbool.h>
#include "hw_memmap.h"
#include "debug.h"
#include "gpio.h"
#include "hw_types.h"
#include "pin_map.h"
#include "sysctl.h"


#define   FASTFLASHTIME			(uint32_t)50000
#define   SLOWFLASHTIME			(uint32_t)100000

uint32_t delay_time,key_value;
uint32_t last = 0, state = 0, counter = 0;

void 		Delay(uint32_t value);
void 		S800_GPIO_Init(void);

void display(uint32_t state, uint32_t* counter)
{
	*counter = (*counter + 1) % SLOWFLASHTIME;
	switch (state)
	{
		case 0: GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0);
						GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
						break;
		case 1: GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
						GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, (*counter > FASTFLASHTIME));
						break;
		case 2: GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0);
						GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
						break;
		case 3: GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0);
						GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, (*counter > FASTFLASHTIME)<<1);
						break;
	}
}


int main(void)
{
	S800_GPIO_Init();
	delay_time = SLOWFLASHTIME;
	while(1)
  {
		key_value = GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)	;				//read the PJ0 key value

		if (key_value == 0 && last == 1)
		{
			state = (state + 1) % 4;
			counter = 0;
		}
		display(state, &counter);
		
		last = key_value;
  }
}


void S800_GPIO_Init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);						//Enable PortF
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));			//Wait for the GPIO moduleF ready
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);						//Enable PortJ	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ));			//Wait for the GPIO moduleJ ready	
	
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_1);			//Set PF0 as Output pin
	GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE,GPIO_PIN_0 | GPIO_PIN_1);//Set the PJ0,PJ1 as input pin
	GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0 | GPIO_PIN_1,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);
}


