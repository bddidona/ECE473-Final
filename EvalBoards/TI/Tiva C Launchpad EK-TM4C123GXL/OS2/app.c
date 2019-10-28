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
*
*                                           APPLICATION CODE
*
*                                      Texas Instruments TM4C129x
*                                                on the
*                                             DK-TM4C129X
*                                           Development Kit
*       		Modified by Dr. Samir A. Rawashdeh, for the TM4C123GH6PM microcontroller on the 
*						TM4C123G Tiva C Series Launchpad (TM4C123GXL), November 2014.
*
* Filename      : app.c
* Version       : V1.00
* Programmer(s) : FF
*********************************************************************************************************
* Note(s)       : None.
*********************************************************************************************************
*/
/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include  "app_cfg.h"
#include  <cpu_core.h>
#include  <os.h>

#include  "..\bsp\bsp.h"
#include  "..\bsp\bsp_led.h"
#include  "..\bsp\bsp_sys.h"

// SAR Addition
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
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "inc/hw_gpio.h"

#include "inc/hw_types.h"

#define PWM_SERVO_FREQUENCY 55
 

// Group 5 Addition 


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/
#define	DIRECTION1_UPDATE 0x01
#define DIRECTION2_UPDATE 0x02

/*
*********************************************************************************************************
*                                           LOCAL CONSTANTS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************
*/


/*$PAGE*/
/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/


OS_EVENT 					*UARTSem;	// TO-DO: Add comment
OS_FLAG_GRP 				*UpdateDirectionFlag; // // TO-DO: Add comment

static  OS_STK       AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE];
static  OS_STK       Task1Stk[APP_CFG_TASK_START_STK_SIZE];
static  OS_STK       Task2Stk[APP_CFG_TASK_START_STK_SIZE];
static  OS_STK       Task3Stk[APP_CFG_TASK_START_STK_SIZE];

/*
*********************************************************************************************************
*                                            LOCAL MACRO'S
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  AppTaskCreate        (void);
static  void  AppTaskStart         (void       *p_arg);
static  void  DirectionTask1       (void       *p_arg);
static  void  DirectionTask2       (void       *p_arg);
static  void  UpdateDirection      (void       *p_arg);


static uint8_t ui8Position = 85;
static uint32_t ui32Load;
static uint32_t ui32PWMClock;



/*$PAGE*/
/*
*********************************************************************************************************
*                                               main()
*
* Description : Entry point for C code.
*
* Arguments   : none.
*
* Returns     : none.
*
* Note(s)     : (1) It is assumed that your code will call main() once you have performed all necessary
*                   initialization.
*********************************************************************************************************
*/

int  main (void)
{
#if (OS_TASK_NAME_EN > 0)
    CPU_INT08U  err;
#endif

#if (CPU_CFG_NAME_EN == DEF_ENABLED)
    CPU_ERR     cpu_err;
#endif

#if (CPU_CFG_NAME_EN == DEF_ENABLED)
    CPU_NameSet((CPU_CHAR *)"TM4C129XNCZAD",
                (CPU_ERR  *)&cpu_err);
#endif


    CPU_IntDis();                                               /* Disable all interrupts.                              */

    OSInit();                                                   /* Initialize "uC/OS-II, The Real-Time Kernel"          */
		
/****************** Create UART Semaphore *************************************************/
		UARTSem					=	OSSemCreate(1);
		
/****************** Create Direction Flag ************************************************/
		UpdateDirectionFlag		=	OSFlagCreate(0x00, &err);

		
    OSTaskCreateExt((void (*)(void *)) AppTaskStart,           /* Create the start task                                */
                    (void           *) 0,
                    (OS_STK         *)&AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE - 1],
                    (INT8U           ) APP_CFG_TASK_START_PRIO,
                    (INT16U          ) APP_CFG_TASK_START_PRIO,
                    (OS_STK         *)&AppTaskStartStk[0],
                    (INT32U          ) APP_CFG_TASK_START_STK_SIZE,
                    (void           *) 0,
                    (INT16U          )(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));

#if (OS_TASK_NAME_EN > 0)
    OSTaskNameSet(APP_CFG_TASK_START_PRIO, "Start", &err);
#endif

    OSStart();                                                  /* Start multitasking (i.e. give control to uC/OS-II)   */

    while (1) {
        ;
    }
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                           App_TaskStart()
*
* Description : Startup task example code.
*
* Arguments   : p_arg       Argument passed by 'OSTaskCreate()'.
*
* Returns     : none.
*
* Created by  : main().
*
* Notes       : (1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                   used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/

static  void  AppTaskStart (void *p_arg)
{
		CPU_INT32U  cpu_clk_freq;
    CPU_INT32U  cnts;
    (void)p_arg;                                                /* See Note #1                                              */


   (void)&p_arg;

    BSP_Init();                                                 /* Initialize BSP functions                             */
		ui32PWMClock = SysCtlClockGet() / 64;  //PWM clock is 625 kHz
	ui32Load = (ui32PWMClock / PWM_SERVO_FREQUENCY) - 1;  //PWM period
	
		//
		// SW1 and S2 Initialization ******************************************************
		//
	
		// Pin F4 and F0  Setup
		// Initialize PF4 and PF0 as input
		// Enable the GPIO port that is used for the LED and SW2
		SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
		/* Wait for peripheral access to enable */
		SysCtlDelay(3);
	
		// Unlock port F 
		HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
		HWREG(GPIO_PORTF_BASE + GPIO_O_CR) = 0x01;
		GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0);
		GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);
		// Enable weak pullup resistor for PF4 and PF0
		GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
		GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
	
	
    cpu_clk_freq = BSP_SysClkFreqGet();                         /* Determine SysTick reference freq.                    */
    cnts         = cpu_clk_freq                                 /* Determine nbr SysTick increments                     */
                 / (CPU_INT32U)OS_TICKS_PER_SEC;

    OS_CPU_SysTickInit(cnts);
    CPU_Init();                                                 /* Initialize the uC/CPU services                       */

#if (OS_TASK_STAT_EN > 0)
    OSStatInit();                                               /* Determine CPU capacity                                   */
#endif

    Mem_Init();

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif


		BSP_LED_Toggle(0);
		OSTimeDlyHMSM(0, 0, 0, 200);
		BSP_LED_Toggle(0);
		BSP_LED_Toggle(1);
		OSTimeDlyHMSM(0, 0, 0, 200);
		BSP_LED_Toggle(1);
		BSP_LED_Toggle(2);
		OSTimeDlyHMSM(0, 0, 0, 200);    
		BSP_LED_Toggle(2);

		OSTimeDlyHMSM(0, 0, 1, 0);   

		AppTaskCreate();                                            /* Creates all the necessary application tasks.         */

    while (DEF_ON) {

        OSTimeDlyHMSM(0, 0, 0, 200);			

    }
}


/*
*********************************************************************************************************
*                                         AppTaskCreate()
*
* Description :  Create the application tasks.
*
* Argument(s) :  none.
*
* Return(s)   :  none.
*
* Caller(s)   :  AppTaskStart()
*
* Note(s)     :  none.
*********************************************************************************************************
*/

static  void  AppTaskCreate (void)
{
OSTaskCreate((void (*)(void *)) DirectionTask1,           /* Create the second task                                */
                    (void           *) 0,							// argument
                    (OS_STK         *)&Task1Stk[APP_CFG_TASK_START_STK_SIZE - 1],
                    (INT8U           ) 5 );  						// Task Priority
                

OSTaskCreate((void (*)(void *)) DirectionTask2,           /* Create the second task                                */
                    (void           *) 0,							// argument
                    (OS_STK         *)&Task2Stk[APP_CFG_TASK_START_STK_SIZE - 1],
                    (INT8U           ) 6 );  						// Task Priority
         										
OSTaskCreate((void (*)(void *)) UpdateDirection,           /* Create the second task                                */
                    (void           *) 0,							// argument
                    (OS_STK         *)&Task3Stk[APP_CFG_TASK_START_STK_SIZE - 1],
                    (INT8U           ) 7);  
										// Task Priority
         																
}

static  void  DirectionTask1 (void *p_arg)
{
	INT8U err;
	OS_FLAGS flag;


   (void)p_arg;
		
    while (1) {        
			OSTaskQuery(OS_PRIO_SELF, &tcb_snapshot);
			flag = OSFlagPend(UpdateDirectionFlag, DIRECTION1_UPDATE, OS_FLAG_WAIT_SET_ALL + OS_FLAG_CONSUME,  10, &err);
			if (flag == DIRECTION1_UPDATE)
			{
				ui8Position = ui8Position + 2;

			}
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, ui8Position * ui32Load / 1000);
			BSP_LED_Toggle(1);
			OSSemPend(UARTSem, 0, &err);
			UARTprintf("T1 ");	// Probably needs to be protected by semaphore
			OSSemPost(UARTSem);
			

			OSTimeDlyHMSM(0, 0, 0, 10);
			
		}
}

static  void  DirectionTask2 (void *p_arg)
{
	INT8U err;
	OS_FLAGS flag;
   (void)p_arg;
	
    while (1) {   
					
			flag = OSFlagPend(UpdateDirectionFlag, DIRECTION2_UPDATE, OS_FLAG_WAIT_SET_ALL + OS_FLAG_CONSUME,  10, &err);
			if (flag == DIRECTION2_UPDATE)
			{
				ui8Position = ui8Position - 2;
			}
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, ui8Position * ui32Load / 1000);
			BSP_LED_Toggle(2);
			OSSemPend(UARTSem, 0, &err);
			UARTprintf("T2 ");  // Probably needs to be protected by semaphore
			OSSemPost(UARTSem);
			 
			OSTimeDlyHMSM(0, 0, 0, 10);
	}
}

static  void  UpdateDirection (void *p_arg)		//Update direction flags
{
	INT8U err;
    (void)p_arg;

	 while(1)
	 {
			if (!GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4))
			{
					// if SW1 is pressed set DIRECTION1_UPDATE flag
					OSFlagPost(UpdateDirectionFlag, DIRECTION1_UPDATE, OS_FLAG_SET, &err);
			}
			else if (!GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0) )
			{
					// if SW2 is pressed set DIRECTION2_UPDATE flag
					OSFlagPost(UpdateDirectionFlag, DIRECTION2_UPDATE, OS_FLAG_SET, &err);
			}
			OSTimeDlyHMSM(0, 0, 0, 50);
		}
}

