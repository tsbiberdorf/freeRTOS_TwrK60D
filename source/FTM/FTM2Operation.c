/*
 * FTM2Operation.c
 *
 *  Created on: Jan 24, 2020
 *      Author: TBiberdorf
 */

#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "MK60D10.h"
#include "fsl_ftm.h"
#include "freeRTOSProject.h"
#include "TaskParameters.h"

const char *Ftm2TaskName = "FTM2Task";
static TaskHandle_t  tl_Ftm2TaskHandlerId = NULL;


void Ftm2Task(void *pvParameters)
{
	while(1)
	{
		vTaskDelay(1000);
	}
}

void StartFtm2Task()
{
	xTaskCreate(Ftm2Task, Ftm2TaskName, FTM2_TASK_STACK_SIZE , NULL, FTM2_TASK_PRIORITY, &tl_Ftm2TaskHandlerId);

}
