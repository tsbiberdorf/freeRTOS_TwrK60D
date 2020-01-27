/*
 * LEDOperations.c
 *
 *  Created on: Dec 17, 2019
 *      Author: TBiberdorf
 */

#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "MK60D10.h"
#include "fsl_uart.h"
#include "freeRTOSProject.h"

#include "fsl_debug_console.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
#include "fsl_common.h"
#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "CMSIS/RTT/SEGGER_RTT.h"
#include "TaskParameters.h"
#include "LEDOperations.h"


#define LED_FLASH_DELAY_TIME (100) //* delay time for flashing LED, should be about 1 sec


TaskHandle_t  tl_LEDTaskHandlerId = NULL;
const char *LedTaskName = "LedTask";

static void LedTask(void *pvParameters)
{
	uint32_t notifyBits;
	uint16_t ledFlashDelay = 0;
	uint16_t interruptFlag = 0;
	const TickType_t xMaxBlockTime = pdMS_TO_TICKS( 10 );
	BaseType_t xResult;
	SEGGER_RTT_printf(0,"LED Task started\r\n");

	PRINTF("LED Task Started\r\n");
	while(1)
	{
//		tl_LEDTaskHandlerId = ledTaskHandlerId;
		xResult = xTaskNotifyWait( 0x00,    /* Don't clear bits on entry. */
				0xFFFFFFFF,        /* Clear all bits on exit. */
				&notifyBits, /* Stores the notified value. */
				xMaxBlockTime );

		if( xResult == pdPASS )
		{
			if( notifyBits & LED_RED_ON)
			{
#if (BOARD_HW == MP8000_HW)
				GPIO_SetPinsOutput(BOARD_INITPINS_RedLED_GPIO,1<<BOARD_INITPINS_RedLED_PIN);
#elif (BOARD_HW == TWR_K60_HW)
#else
#endif
			}
			if( notifyBits & LED_RED_OFF)
			{
#if (BOARD_HW == MP8000_HW)
				GPIO_ClearPinsOutput(BOARD_INITPINS_RedLED_GPIO,1<<BOARD_INITPINS_RedLED_PIN);
#elif (BOARD_HW == TWR_K60)
#else
#endif
			}
			if( notifyBits & GREEN_RED_ON)
			{
#if (BOARD_HW == MP8000_HW)
				GPIO_SetPinsOutput(BOARD_INITPINS_GreenLED_GPIO,1<<BOARD_INITPINS_GreenLED_PIN);
#elif (BOARD_HW == TWR_K60)
#else
#endif
			}
			if( notifyBits & GREEN_RED_OFF)
			{
#if (BOARD_HW == MP8000_HW)
				GPIO_ClearPinsOutput(BOARD_INITPINS_GreenLED_GPIO,1<<BOARD_INITPINS_GreenLED_PIN);
#elif (BOARD_HW == TWR_K60)
#else
#endif
			}
			if( notifyBits & BLUE_RED_ON)
			{
#if (BOARD_HW == MP8000_HW)
				GPIO_SetPinsOutput(BOARD_INITPINS_BlueLED_GPIO,1<<BOARD_INITPINS_BlueLED_PIN);
#elif (BOARD_HW == TWR_K60)
#else
#endif
			}
			if( notifyBits & BLUE_RED_OFF)
			{
#if (BOARD_HW == MP8000_HW)
				GPIO_ClearPinsOutput(BOARD_INITPINS_BlueLED_GPIO,1<<BOARD_INITPINS_BlueLED_PIN);
#elif (BOARD_HW == TWR_K60)
#else
#endif
			}

//			SEGGER_RTT_printf(0,"led %d\r\n",notifyBits);
//			GPIO_TogglePinsOutput(BOARD_INITPINS_RedLED_GPIO,1<<BOARD_INITPINS_RedLED_PIN);
//			GPIO_TogglePinsOutput(BOARD_INITPINS_GreenLED_GPIO,1<<BOARD_INITPINS_GreenLED_PIN);
//			GPIO_TogglePinsOutput(BOARD_INITPINS_BlueLED_GPIO,1<<BOARD_INITPINS_BlueLED_PIN);
		}
		else
		{
		}

		if(interruptFlag)
		{
			if( interruptFlag++ > LED_FLASH_DELAY_TIME)
			{
				interruptFlag = 0;
				// flash LED to indicate still alive
//				GPIO_ClearPinsOutput(GPIOC,LED3);
			}
		}

		if(ledFlashDelay++ > LED_FLASH_DELAY_TIME)
		{
			ledFlashDelay = 0;
			// flash LED to indicate still alive
//			GPIO_TogglePinsOutput(GPIOC,LED2);
		}


	}


}

TaskHandle_t GetLedTaskHandle()
{
	return tl_LEDTaskHandlerId;
}

void StartLedTask()
{
	xTaskCreate(LedTask, LedTaskName, LED_TASK_STACK_SIZE , NULL, LED_TASK_PRIORITY, &tl_LEDTaskHandlerId);

}
