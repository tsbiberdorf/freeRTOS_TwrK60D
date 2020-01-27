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
#include "fsl_debug_console.h"

#include "CMSIS/RTT/SEGGER_RTT.h"

/* The Flextimer instance/channel used for board */
#define BOARD_FTM_BASEADDR FTM2
#define BOARD_FTM_CHANNEL kFTM_Chnl_0
#define FTM_LED_HANDLER FTM2_IRQHandler

/* Interrupt to enable and flag to read; depends on the FTM channel used */
#define FTM_CHANNEL_INTERRUPT_ENABLE kFTM_Chnl0InterruptEnable
#define FTM_CHANNEL_FLAG kFTM_Chnl0Flag


const char *Ftm2TaskName = "FTM2Task";
static TaskHandle_t  tl_Ftm2TaskHandlerId = NULL;
/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile bool ftmIsrFlag = false;
volatile bool brightnessUp = true; /* Indicate LED is brighter or dimmer */
volatile uint8_t updatedDutycycle = 1U;

/* Get source clock for FTM driver */
#define FTM_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_BusClk)


void FTM_LED_HANDLER(void)
{
    ftmIsrFlag = true;

    if (brightnessUp)
    {
        /* Increase duty cycle until it reach limited value, don't want to go upto 100% duty cycle
         * as channel interrupt will not be set for 100%
         */
        if (++updatedDutycycle >= 99U)
        {
            updatedDutycycle = 99U;
            brightnessUp = false;
        }
    }
    else
    {
        /* Decrease duty cycle until it reach limited value */
        if (--updatedDutycycle == 1U)
        {
            brightnessUp = true;
        }
    }

    if ((FTM_GetStatusFlags(BOARD_FTM_BASEADDR) & FTM_CHANNEL_FLAG) == FTM_CHANNEL_FLAG)
    {
        /* Clear interrupt flag.*/
        FTM_ClearStatusFlags(BOARD_FTM_BASEADDR, FTM_CHANNEL_FLAG);
    }
}


void Ftm2Task(void *pvParameters)
{
	BaseType_t xResult;
	uint8_t changeFlag = 0;
    ftm_pwm_level_select_t pwmLevel = kFTM_LowTrue;
    ftm_chnl_pwm_signal_param_t ftmParam;
    ftm_pwm_sync_method_t syncMethod = kFTM_SoftwareTrigger;
	union _Long_Char_Join
	{
		uint32_t notifyBits;
		char notityBytes[4];
	} notifyData;

    /* Configure ftm params with frequency 24kHZ */
    ftmParam.chnlNumber = BOARD_FTM_CHANNEL;
    ftmParam.level = pwmLevel;
    ftmParam.dutyCyclePercent = updatedDutycycle;
    ftmParam.firstEdgeDelayPercent = 0U;

    ftm_chnl_t pwmChA = kFTM_Chnl_0;
    ftm_chnl_t pwmChB = kFTM_Chnl_1;
    PRINTF("ftm2 task started\r\n");
	FTM_DisableInterrupts(BOARD_FTM_BASEADDR, FTM_CHANNEL_INTERRUPT_ENABLE);
    FTM_SetupPwm(BOARD_FTM_BASEADDR, &ftmParam, 1U, kFTM_CenterAlignedPwm, 24000U, FTM_SOURCE_CLOCK);
//	FTM_EnableInterrupts(BOARD_FTM_BASEADDR, FTM_CHANNEL_INTERRUPT_ENABLE);

    /*
     * sync channels 0/1
     * combine channels 0/1
     * make channels 0/1 a complement of each other
     * enable deadtime on channels 0/1
     */
    BOARD_FTM_BASEADDR->COMBINE |= (FTM_COMBINE_COMBINE0_MASK|FTM_COMBINE_COMP0_MASK
    		|FTM_COMBINE_SYNCEN0_MASK|FTM_COMBINE_DTEN0_MASK);

	FTM_UpdateChnlEdgeLevelSelect(BOARD_FTM_BASEADDR, pwmChA, 0U);
	FTM_UpdateChnlEdgeLevelSelect(BOARD_FTM_BASEADDR, pwmChB, 0U);

	/* Update PWM duty cycle */
	FTM_UpdatePwmDutycycle(BOARD_FTM_BASEADDR, pwmChA, kFTM_CenterAlignedPwm, updatedDutycycle);
	FTM_UpdatePwmDutycycle(BOARD_FTM_BASEADDR, pwmChB, kFTM_CenterAlignedPwm, updatedDutycycle);

	/* Software trigger to update registers */
	FTM_SetSoftwareTrigger(BOARD_FTM_BASEADDR, true);

	/* Start channel output with updated dutycycle */
	FTM_UpdateChnlEdgeLevelSelect(BOARD_FTM_BASEADDR, pwmChA, pwmLevel);
	FTM_UpdateChnlEdgeLevelSelect(BOARD_FTM_BASEADDR, pwmChB, pwmLevel);


    FTM_StartTimer(BOARD_FTM_BASEADDR, kFTM_SystemClock);

	while(1)
	{
		xResult = xTaskNotifyWait( 0x00,    /* Don't clear bits on entry. */
				0xffffffff,        /* Clear all bits on exit. */
				&notifyData.notifyBits, /* Stores the notified value. */
				portMAX_DELAY );

		if( xResult == pdPASS )
		{
			SEGGER_RTT_printf(0,"pwm bits %d %d\r\n",notifyData.notityBytes[0],notifyData.notityBytes[1]);
			switch(notifyData.notityBytes[0])
			{
			case 0:
				FTM_UpdateChnlEdgeLevelSelect(BOARD_FTM_BASEADDR, pwmChA, 0U);
				FTM_UpdatePwmDutycycle(BOARD_FTM_BASEADDR, pwmChA, kFTM_CenterAlignedPwm, notifyData.notityBytes[1]);
				FTM_SetSoftwareTrigger(BOARD_FTM_BASEADDR, true);
				FTM_UpdateChnlEdgeLevelSelect(BOARD_FTM_BASEADDR, pwmChA, pwmLevel);
				break;
			case 1:
				FTM_UpdateChnlEdgeLevelSelect(BOARD_FTM_BASEADDR, pwmChB, 0U);
				FTM_UpdatePwmDutycycle(BOARD_FTM_BASEADDR, pwmChB, kFTM_CenterAlignedPwm, notifyData.notityBytes[1]);
				FTM_SetSoftwareTrigger(BOARD_FTM_BASEADDR, true);
				FTM_UpdateChnlEdgeLevelSelect(BOARD_FTM_BASEADDR, pwmChB, pwmLevel);
				break;
			}
		}

		{
			if( changeFlag )
			{
				changeFlag = 0;
				FTM_UpdateChnlEdgeLevelSelect(BOARD_FTM_BASEADDR, BOARD_FTM_CHANNEL, 0U);

				/* Update PWM duty cycle */
				FTM_UpdatePwmDutycycle(BOARD_FTM_BASEADDR, BOARD_FTM_CHANNEL, kFTM_CenterAlignedPwm, updatedDutycycle);

				/* Software trigger to update registers */
				FTM_SetSoftwareTrigger(BOARD_FTM_BASEADDR, true);

				/* Start channel output with updated dutycycle */
				FTM_UpdateChnlEdgeLevelSelect(BOARD_FTM_BASEADDR, BOARD_FTM_CHANNEL, pwmLevel);
			}
			vTaskDelay(100);
		}

	}
}

TaskHandle_t GetFtm2TaskHandle()
{
	return tl_Ftm2TaskHandlerId;
}

void StartFtm2Task()
{
	xTaskCreate(Ftm2Task, Ftm2TaskName, FTM2_TASK_STACK_SIZE , NULL, FTM2_TASK_PRIORITY, &tl_Ftm2TaskHandlerId);

}
