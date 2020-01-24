/*
 * LEDOperations.c
 *
 *  Created on: Dec 17, 2019
 *      Author: TBiberdorf
 */

#include "FreeRTOS.h"
#include "task.h"
#include "MK60D10.h"
#include "fsl_uart.h"

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
	TaskHandle_t  ledTaskHandlerId = tl_LEDTaskHandlerId;

	while(1)
	{
//		tl_LEDTaskHandlerId = ledTaskHandlerId;
		xResult = xTaskNotifyWait( 0x00,    /* Don't clear bits on entry. */
				0xFFFFFFFF,        /* Clear all bits on exit. */
				&notifyBits, /* Stores the notified value. */
				xMaxBlockTime );

		if( xResult == pdPASS )
		{
			SEGGER_RTT_printf(0,"led %d\r\n",notifyBits);
			interruptFlag = 1;
			GPIO_SetPinsOutput(GPIOC,LED3);
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
				GPIO_ClearPinsOutput(GPIOC,LED3);
			}
		}

		if(ledFlashDelay++ > LED_FLASH_DELAY_TIME)
		{
			ledFlashDelay = 0;
			// flash LED to indicate still alive
			GPIO_TogglePinsOutput(GPIOC,LED2);
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
