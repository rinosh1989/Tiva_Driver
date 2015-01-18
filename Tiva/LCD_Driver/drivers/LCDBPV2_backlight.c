/*
 * LCDBPV2_backlight.c
 *
 *  Created on: 27/12/2013
 *      Author: Dani
 */



//**Define backlight: 				BK_I2C (#define BK_I2C) o PARALELO (defecto)
#define BK_I2C
//**Define pines I2C: SDA=, SCL=(A_I2C) o I2C: SDA=, SCL=(defecto)
//#define A_I2C
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
//#include "driverlib/i2c.h"
//#include "inc/hw_adc.h"
//#include "inc/hw_gpio.h"
////#include "inc/hw_ints.h"
//#include "inc/hw_memmap.h"
//#include "inc/hw_timer.h"
//#include "inc/hw_types.h"
//#include "driverlib/adc.h"
//#include "driverlib/gpio.h"
//#include "driverlib/interrupt.h"
//#include "driverlib/sysctl.h"
//#include "driverlib/timer.h"
//#include "grlib/grlib.h"
//#include "grlib/widget.h"
#include "driverlib/i2c.h"
//#include "driverlib/rom.h"
//#include "driverlib/eeprom.h"

#include "LCDBPV2_backlight.h"

#ifdef BK_I2C

#ifdef A_I2C
	#define I2C_BASE 	GPIO_PORTA_BASE
	#define I2C_PERIPH	SYSCTL_PERIPH_GPIOA
	#define I2C_SYSCTL_PERIPH	SYSCTL_PERIPH_I2C1
	#define I2C_CONF_SCL GPIO_PA6_I2C1SCL
	#define I2C_CONF_SDA GPIO_PA7_I2C1SDA
	#define I2C_SCL		GPIO_PIN_6
	#define I2C_SDA		GPIO_PIN_7
	#define I2C_M_BASE  I2C1_MASTER_BASE
#else
	#define I2C_BASE 	GPIO_PORTE_BASE
	#define I2C_PERIPH	SYSCTL_PERIPH_GPIOE
	#define I2C_SYSCTL_PERIPH	SYSCTL_PERIPH_I2C2
	#define I2C_CONF_SCL GPIO_PE4_I2C2SCL
	#define I2C_CONF_SDA GPIO_PE5_I2C2SDA

	#define I2C_SCL		GPIO_PIN_4
	#define I2C_SDA		GPIO_PIN_5
	#define I2C_M_BASE  I2C2_MASTER_BASE
#endif

unsigned long WaitI2CDone( unsigned int long ulBase)
{
	// Wait until done transmitting
	while( I2CMasterBusy(I2C_M_BASE));
	// Return I2C error code
	return I2CMasterErr( I2C_M_BASE);
}


void SetupI2C()
{
	/*    **
	 * Shared Pin Setting

    // You need to set the shared pins as input.
    GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_6 | GPIO_PIN_7);
    // Change the pad configuration to WPU
    GPIOPadConfigSet( GPIO_PORTB_BASE, GPIO_PIN_6 | GPIO_PIN_7, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_OD_WPU);*/

	/***
	 * I2C Setting
	 */
		//#define I2C_BASE 	GPIO_PORTE_BASE
		//#define I2C_PERIPH	SYSCTL_PERIPH_GPIOE
		//#define I2C_SYSCTL_PERIPH	SYSCTL_PERIPH_I2C2
		//#define I2C_CONF_SCL GPIO_PE4_I2C2SCL
		//#define I2C_CONF_SDA GPIO_PE5_I2C2SDA
		//#define I2C_SCL		GPIO_PIN_4
		//#define I2C_SDA		GPIO_PIN_5
		//#define I2C_M_BASE  I2C2_MASTER_BASE


}

void ConfigBK_I2C()
{
	//send config byte i2c
	//SetupI2C();
	//I2C write

	//start
	//send address byte I2C_SLAVE_ADDRESS
	I2CMasterSlaveAddrSet( I2C_M_BASE, 0x45, false);
	//send command byte
	I2CMasterDataPut( I2C_M_BASE, 0x10);
	I2CMasterControl( I2C_M_BASE, I2C_MASTER_CMD_BURST_SEND_START);
	WaitI2CDone( I2C_M_BASE);

	I2CMasterDataPut( I2C_M_BASE, 0x00);
	I2CMasterControl( I2C_M_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
	WaitI2CDone( I2C_M_BASE);

	I2CMasterDataPut( I2C_M_BASE, 0xFF);
	I2CMasterControl( I2C_M_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
	WaitI2CDone( I2C_M_BASE);

	I2CMasterDataPut( I2C_M_BASE, 0x0);
	I2CMasterControl( I2C_M_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
	WaitI2CDone( I2C_M_BASE);

	I2CMasterDataPut( I2C_M_BASE, 0x44);
	I2CMasterControl( I2C_M_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
	WaitI2CDone( I2C_M_BASE);

	I2CMasterDataPut( I2C_M_BASE, 0x44);
	I2CMasterControl( I2C_M_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
	WaitI2CDone( I2C_M_BASE);

	I2CMasterDataPut( I2C_M_BASE, 0x44);
	I2CMasterControl( I2C_M_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
	WaitI2CDone( I2C_M_BASE);

	I2CMasterDataPut( I2C_M_BASE, 0x44);
	I2CMasterControl( I2C_M_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
	WaitI2CDone( I2C_M_BASE);

	I2CMasterDataPut( I2C_M_BASE, 0x44);
	I2CMasterControl( I2C_M_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
	WaitI2CDone( I2C_M_BASE);

	I2CMasterDataPut( I2C_M_BASE, 0xFF);
	I2CMasterControl( I2C_M_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
	WaitI2CDone( I2C_M_BASE);

	I2CMasterDataPut( I2C_M_BASE, 0x00);
	I2CMasterControl( I2C_M_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
	WaitI2CDone( I2C_M_BASE);
}
void Config()
{
	SetupI2C();

}
extern void LCDBPV2_backlight_init()
{
	//SetupI2C();
	ConfigBK_I2C();
}

#else //Backlight paralelo
extern 	void LCDBPV2_backlight_init(int value)
	{
		Config();
		//GPIOPinWrite(LCD_BL_BASE, LCD_BL_PIN, LCD_BL_PIN);
	}
	void Config()
	{
		SysCtlPeripheralEnable(LCD_BL_PERIPH);
		GPIODirModeSet(LCD_BL_BASE, LCD_BL_PIN, GPIO_DIR_MODE_OUT);
		GPIOPadConfigSet(LCD_BL_BASE, LCD_BL_PIN, GPIO_STRENGTH_8MA,GPIO_PIN_TYPE_STD);
		 //GPIOPinWrite(LCD_BL_BASE, LCD_BL_PIN, 0x00);
		 GPIOPinWrite(LCD_BL_BASE, LCD_BL_PIN, LCD_BL_PIN);
		 //GPIOPinWrite(LCD_BL_BASE, LCD_BL_PIN, LCD_BL_PIN);

		     	      /* // Set timer 3A (outputing to CCP6/PH0) into PWM mode with a 1KHz
		     	       // frequency.
		     	       //

		     	       //
		     	       // Enable the GPIO and timer peripherals used to control backlight
		     	       // brightness.
		     	       //

		     	       SysCtlPeripheralEnable(LCD_BL_PERIPH);
		     	       SysCtlPeripheralEnable(LCD_BL_TIMER_PERIPH);

		     	       //
		     	       // Configure the GPIO pin connected to the backlight as a GPIO output and
		     	       // set it low (turning the backlight off).
		     	       //
		     	       GPIOPinTypeGPIOOutput(LCD_BL_BASE, LCD_BL_PIN);
		     	       GPIOPinWrite(LCD_BL_BASE, LCD_BL_PIN, LCD_BL_PIN);

		     	       //
		     	       // Configure timer 3 as a 16-bit pair, with the first producing a
		     	       // PWM output and the other in periodic mode.
		     	       //
		     	       TimerConfigure(LCD_BL_TIMER_BASE, (TIMER_CFG_SPLIT_PAIR |
		     	                       TIMER_CFG_A_PWM | TIMER_CFG_B_PERIODIC));

		     	       //
		     	       // Invert the PWM output, so that the match value relative to zero always
		     	       // defines the amount of time the PWM signal is high (regardless of the
		     	       // current load value).
		     	       //
		     	       TimerControlLevel(LCD_BL_TIMER_BASE, LCD_BL_TIMER, true);

		     	       //
		     	       // Set the PWM frequency to run at 1KHz.
		     	       //
		     	       TimerLoadSet(LCD_BL_TIMER_BASE, LCD_BL_TIMER, (SysCtlClockGet()/ 1000) - 1);

		     	       //
		     	       // Enable the timer.
		     	       //
		     	       TimerEnable(LCD_BL_TIMER_BASE, LCD_BL_TIMER);*/
	}
#endif



