/*
*********************************************************************************************************
*                                            EXAMPLE CODE
*
*               This file is provided as an example on how to use Micrium products.
*
*               Please feel free to use any application code labeled as 'EXAMPLE CODE' in
*               your application products.  Example code may be used as is, in whole or in
*               part, or may be used as a reference only. This file can be modified as
*               required to meet the end-product requirements.
*
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*
*               You can find our product's user manual, API reference, release notes and
*               more information at https://doc.micrium.com.
*               You can contact us at www.micrium.com.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                         BOARD SUPPORT PACKAGE
*
*                                      Texas Instruments TM4C129x
*                                                on the
*                                             DK-TM4C129X
*                                           Development Kit
*       		Modified by Dr. Samir A. Rawashdeh, for the TM4C123GH6PM microcontroller on the 
*						TM4C123G Tiva C Series Launchpad (TM4C123GXL), November 2014.
*
* Filename      : bsp.c
* Version       : V1.00
* Programmer(s) : FF
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                               INCLUDES
*********************************************************************************************************
*/

#include  "..\bsp\bsp_sys.h"
#include  "..\bsp\bsp_led.h"
#include  "..\bsp\bsp_int.h"

// SAR Addition
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/debug.h"
#include "driverlib/pwm.h"
#include "driverlib/pin_map.h"
#include "inc/hw_gpio.h"
#include "driverlib/rom.h"

#include <stdio.h>
#define PWM_SERVO_FREQUENCY 55
/*$PAGE*/
/*
*********************************************************************************************************
*                                         BSP INITIALIZATION
*
* Description: This function should be called by the application code before using any functions in this
*              module.
*
* Arguments  : none
*********************************************************************************************************
*/

void  BSP_Init (void)
{		
		volatile uint32_t ui32Load;
		volatile uint32_t ui32PWMClock;
		char str[10];		//temporary char array to print system clock to uart
	
    BSP_IntInit();
    BSP_SysInit();                                              /* Initialize system services, clocks, etc.             */
    BSP_LED_Init();
		
		//Initialize PWM subsystem
		////////////////////////////
	//Set PWM clock to 625 KHz
	SysCtlPWMClockSet(SYSCTL_PWMDIV_64);

	//Enable, Turn on clock to PWM1, GPIOD, GPIOF
	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	//Configure pin PDO for PWM output
	GPIOPinTypePWM(GPIO_PORTD_BASE, GPIO_PIN_0);
	GPIOPinConfigure(GPIO_PD0_M1PWM0);

	//Configure PORTF switches
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;  //unlock PF0
	HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;
	GPIODirModeSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_DIR_MODE_IN);
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

	//Set up PWM Clock
	ui32PWMClock = SysCtlClockGet() / 64;  //PWM clock is 625 kHz
	ui32Load = (ui32PWMClock / PWM_SERVO_FREQUENCY) - 1;  //PWM period
	PWMGenConfigure(PWM1_BASE, PWM_GEN_0, PWM_GEN_MODE_DOWN);  //Count down mode
	PWMGenPeriodSet(PWM1_BASE, PWM_GEN_0, ui32Load);  //Set the PWM period

	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, 85 * ui32Load / 1000); //Initial pulse width 1.5 msec
	PWMOutputState(PWM1_BASE, PWM_OUT_0_BIT, true);  //Enable PWM output
	PWMGenEnable(PWM1_BASE, PWM_GEN_0);  //Turn PWM1 on
	
	
	
		// SAR: Initialize UART using TI's standard peripheral library
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
		UARTStdioConfig(0, 115200, BSP_SysClkFreqGet()); 
   
		UARTprintf("\nUART0 Init Complete.");
		UARTprintf("\nSystem clock = ");
  	sprintf(str,"%d",BSP_SysClkFreqGet());  
	  UARTprintf(str);
		UARTprintf("\n");
}

