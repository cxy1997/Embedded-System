/*
 * GPIO Interrupts !!
 */

#include <stdint.h>
#include <stdbool.h>
#include "hw_memmap.h"
#include "debug.h"
#include "gpio.h"
#include "hw_i2c.h"
#include "hw_types.h"
#include "i2c.h"
#include "pin_map.h"
#include "sysctl.h"
#include "systick.h"
#include "interrupt.h"
#include "hw_ints.h"
#define SYSTICK_FREQUENCY		1000			//1000hz

#define	I2C_FLASHTIME				500				//500mS
#define GPIO_FLASHTIME			300				//300mS
//*****************************************************************************
//
//I2C GPIO chip address and resigster define
//
//*****************************************************************************
#define TCA6424_I2CADDR 					0x22
#define PCA9557_I2CADDR						0x18

#define PCA9557_INPUT							0x00
#define	PCA9557_OUTPUT						0x01
#define PCA9557_POLINVERT					0x02
#define PCA9557_CONFIG						0x03

#define TCA6424_CONFIG_PORT0			0x0c
#define TCA6424_CONFIG_PORT1			0x0d
#define TCA6424_CONFIG_PORT2			0x0e

#define TCA6424_INPUT_PORT0				0x00
#define TCA6424_INPUT_PORT1				0x01
#define TCA6424_INPUT_PORT2				0x02

#define TCA6424_OUTPUT_PORT0			0x04
#define TCA6424_OUTPUT_PORT1			0x05
#define TCA6424_OUTPUT_PORT2			0x06




void 		Delay(uint32_t value);
void 		S800_GPIO_Init(void);
uint8_t 	I2C0_WriteByte(uint8_t DevAddr, uint8_t RegAddr, uint8_t WriteData);
uint8_t 	I2C0_ReadByte(uint8_t DevAddr, uint8_t RegAddr);
void		S800_I2C0_Init(void);
void GPIOJ_Handler(void);
void plus_one_sec(void);
void waiting(void);
void Display(void);

//systick software counter define
uint8_t idx = 0, times = 100;
volatile uint16_t minute_counter, second_counter;
volatile uint8_t	minute_status, second_status, min = 0, sec = 0;

volatile uint8_t result,cnt,key_value,gpio_status;
volatile uint8_t rightshift = 0x01;
uint32_t ui32SysClock;
uint8_t seg7[] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,0x7c,0x58,0x5e,0x079,0x71,0x5c};
uint8_t periods[] = {5, 10, 20, 2};

int main(void)
{
	volatile uint16_t	i2c_flash_cnt,gpio_flash_cnt;
	//use internal 16M oscillator, PIOSC
   //ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_16MHZ |SYSCTL_OSC_INT |SYSCTL_USE_OSC), 16000000);		
	//ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_16MHZ |SYSCTL_OSC_INT |SYSCTL_USE_OSC), 8000000);		
	//use external 25M oscillator, MOSC
   //ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |SYSCTL_OSC_MAIN |SYSCTL_USE_OSC), 25000000);		

	//use external 25M oscillator and PLL to 120M
   //ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |SYSCTL_OSC_MAIN | SYSCTL_USE_PLL |SYSCTL_CFG_VCO_480), 120000000);;		
	ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_16MHZ |SYSCTL_OSC_INT | SYSCTL_USE_PLL |SYSCTL_CFG_VCO_480), 20000000);

  SysTickPeriodSet(ui32SysClock/SYSTICK_FREQUENCY);
	SysTickEnable();
	SysTickIntEnable();
   IntMasterEnable();		

	
	S800_GPIO_Init();
	S800_I2C0_Init();
	minute_counter = 600 * times;
	second_counter = 10 * times;
	while (1)
	{
		if (second_status)
		{
			second_status = 0;
			plus_one_sec();
		}
		if (minute_status)
		{
			minute_status = 0;
			min = (min + 1) % 60;
		}
		Display();
	}
}

void Display(void)
{
	waiting();
	I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[(min)/10]);
	I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1);
	waiting();
	I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[(min)%10]);
	I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,2);
	waiting();
	I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,6);
	I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,4);
	waiting();
	I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[(sec)/10]);
	I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,8);
	waiting();
	I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[(sec)%10]);
	I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,16);
}

void waiting(void)
{
	Delay(I2C_FLASHTIME*40);
}

void Delay(uint32_t value)
{
	uint32_t ui32Loop;
	for(ui32Loop = 0; ui32Loop < value; ui32Loop++){};
}

void S800_GPIO_Init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);						//Enable PortF
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));			//Wait for the GPIO moduleF ready
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);						//Enable PortJ	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ));			//Wait for the GPIO moduleJ ready	
	
	IntMasterEnable();
	IntEnable(INT_GPIOJ_TM4C123);
	GPIOIntRegister(GPIO_PORTJ_BASE, GPIOJ_Handler);
	
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0);			//Set PF0 as Output pin
	GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1);//Set the PJ0,PJ1 as input pin
	GPIOPadConfigSet(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
	
	GPIOIntTypeSet(GPIO_PORTJ_BASE, GPIO_INT_PIN_0 | GPIO_INT_PIN_1, GPIO_FALLING_EDGE); // interrupts when pressed (rising edge for released)
	GPIOIntEnable(GPIO_PORTJ_BASE, GPIO_INT_PIN_0 | GPIO_INT_PIN_1);
}

void S800_I2C0_Init(void)
{
	uint8_t result;
  SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	GPIOPinConfigure(GPIO_PB2_I2C0SCL);
  GPIOPinConfigure(GPIO_PB3_I2C0SDA);
  GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
  GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);

	I2CMasterInitExpClk(I2C0_BASE,ui32SysClock, true);										//config I2C0 400k
	I2CMasterEnable(I2C0_BASE);	

	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_CONFIG_PORT0,0x0ff);		//config port 0 as input
	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_CONFIG_PORT1,0x0);			//config port 1 as output
	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_CONFIG_PORT2,0x0);			//config port 2 as output 

	result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_CONFIG,0x00);					//config port as output
	result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0x0ff);				//turn off the LED1-8
	
}


uint8_t I2C0_WriteByte(uint8_t DevAddr, uint8_t RegAddr, uint8_t WriteData)
{
	uint8_t rop;
	while(I2CMasterBusy(I2C0_BASE)){};
	I2CMasterSlaveAddrSet(I2C0_BASE, DevAddr, false);
	I2CMasterDataPut(I2C0_BASE, RegAddr);
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);
	while(I2CMasterBusy(I2C0_BASE)){};
	rop = (uint8_t)I2CMasterErr(I2C0_BASE);

	I2CMasterDataPut(I2C0_BASE, WriteData);
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
	while(I2CMasterBusy(I2C0_BASE)){};

	rop = (uint8_t)I2CMasterErr(I2C0_BASE);
	return rop;
}

uint8_t I2C0_ReadByte(uint8_t DevAddr, uint8_t RegAddr)
{
	uint8_t value,rop;
	while(I2CMasterBusy(I2C0_BASE)){};	
	I2CMasterSlaveAddrSet(I2C0_BASE, DevAddr, false);
	I2CMasterDataPut(I2C0_BASE, RegAddr);
//	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);		
	I2CMasterControl(I2C0_BASE,I2C_MASTER_CMD_SINGLE_SEND);
	while(I2CMasterBusBusy(I2C0_BASE));
	rop = (uint8_t)I2CMasterErr(I2C0_BASE);
	Delay(1);
	//receive data
	I2CMasterSlaveAddrSet(I2C0_BASE, DevAddr, true);
	I2CMasterControl(I2C0_BASE,I2C_MASTER_CMD_SINGLE_RECEIVE);
	while(I2CMasterBusBusy(I2C0_BASE));
	value=I2CMasterDataGet(I2C0_BASE);
		Delay(1);
	return value;
}

/*
	Corresponding to the startup_TM4C129.s vector table systick interrupt program name
*/

void plus_one_sec(void)
{
  ++sec;
	if (sec == 60)
	{
		sec = 0;
		min = (min + 1) % 60;
	}
}

void GPIOJ_Handler(void)
{
	GPIOIntClear(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	if (!GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0))
	{
		plus_one_sec();
		second_counter = times * 2;
	}
	if (!GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1))
	{
		min = (min + 1) % 60;
		minute_counter = times * 2;
	}
}
void SysTick_Handler(void)
{
	if (second_counter != 0)
		--second_counter;
	else
	{
		if (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0))
			second_counter = times * 10;
		else
			second_counter	= times * 2;
		second_status 	= 1;
	}
	if (minute_counter	!= 0)
		--minute_counter;
	else
	{
		if (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1))
			minute_counter	= times * 600;
		else
			minute_counter	= times * 2;
		minute_status 	= 1;
	}
}
