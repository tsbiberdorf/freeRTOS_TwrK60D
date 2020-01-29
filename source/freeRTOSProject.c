/*
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of NXP Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
 
/**
 * @file    freeRtosExample.c
 * @brief   Application entry point.
 */
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MK60D10.h"
#include "fsl_debug_console.h"

/* TODO: insert other include files here. */

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/* TODO: insert other definitions and declarations here. */
/*******************************************************************************
 * Definitions
 ******************************************************************************/

extern void StartDebugTask();
extern void StartGpioTask();
extern void StartLedTask();
extern void StartFtm0Task();

/* Task priorities. */
#define hello_task_PRIORITY (configMAX_PRIORITIES - 3)
#define trigger_task_PRIORITY (configMAX_PRIORITIES - 3)
static TaskHandle_t  tl_HelloTaskHandlerId = NULL;


/*******************************************************************************
 * Code
 ******************************************************************************/

static void trigger_task(void *pvParameters)
{
	for(;;)
	{
//		xTaskNotify(tl_HelloTaskHandlerId,0xFFFF,eSetBits);

//		xTaskNotify(GetDebugdTaskHandle(),0xFFFF,eSetBits);

		vTaskDelay(1000);
	}
}
/*!
 * @brief Task responsible for printing of "Hello world." message.
 */
static void hello_task(void *pvParameters)
{
	BaseType_t xResult;
	union _Long_Char_Join
	{
		uint32_t notifyBits;
		char notityBytes[4];
	} notifyData;

    for (;;)
    {
		xResult = xTaskNotifyWait( 0x00,    /* Don't clear bits on entry. */
				0xffffffff,        /* Clear all bits on exit. */
				&notifyData.notifyBits, /* Stores the notified value. */
				portMAX_DELAY );

		if( xResult == pdPASS )
		{
			PRINTF("Hello world.\r\n");
		}
//        vTaskDelay(1000);
    }
}

/*
 * @brief   Application entry point.
 */
int main(void) {

  	/* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
  	/* Init FSL debug console. */
    BOARD_InitDebugConsole();

    xTaskCreate(hello_task, "Hello_task", configMINIMAL_STACK_SIZE + 10, NULL, hello_task_PRIORITY, &tl_HelloTaskHandlerId);
    xTaskCreate(trigger_task, "triggerHello", configMINIMAL_STACK_SIZE + 10, NULL, hello_task_PRIORITY, NULL);
    StartDebugTask();
    StartFtm0Task();
//    xTaskCreate(GpioTask, GpioTaskName, GPIO_TASK_STACK_SIZE , NULL, GPIO_TASK_PRIORITY, NULL);
//    StartGpioTask();

    StartLedTask();

    vTaskStartScheduler();
    for (;;)
        ;
}
