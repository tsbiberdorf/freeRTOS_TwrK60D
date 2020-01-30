/*
 * FTM0Operation.c
 *
 *  Created on: Jan 29, 2020
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
#define BOARD_FTM_BASEADDR FTM0
#define BOARD_FTM_CHANNEL kFTM_Chnl_0
#define FTM_LED_HANDLER FTM0_IRQHandler

/* Interrupt to enable and flag to read; depends on the FTM channel used */
#define FTM_CHANNEL_INTERRUPT_ENABLE kFTM_Chnl0InterruptEnable
#define FTM_CHANNEL_FLAG kFTM_Chnl0Flag


const char *Ftm0TaskName = "FTM0Task";
static TaskHandle_t  tl_Ftm0TaskHandlerId = NULL;
/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile bool ftmIsrFlag = false;
volatile bool brightnessUp = true; /* Indicate LED is brighter or dimmer */
volatile uint8_t updatedDutycycle = 50U;

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


void Ftm0Task(void *pvParameters)
{
	BaseType_t xResult;
	uint8_t changeFlag = 0;
	uint32_t deadTimeDelay = 0;

	ftm_pwm_mode_t alignment = kFTM_CombinedPwm;
    ftm_pwm_level_select_t pwmLevel = kFTM_LowTrue;
    ftm_chnl_pwm_signal_param_t ftmParam;
	union _Long_Char_Join
	{
		uint32_t notifyBits;
		char notityBytes[4];
	} notifyData;

    ftm_chnl_t pwmHV_PH2 = kFTM_Chnl_0;
    ftm_chnl_t pwmHV_PH1 = kFTM_Chnl_1;
    ftm_chnl_t pwmHV_ENABLE = kFTM_Chnl_2;

    /* Configure ftm params with frequency 24kHZ */
    ftmParam.chnlNumber = pwmHV_PH2;
    ftmParam.level = pwmLevel;
    ftmParam.dutyCyclePercent = updatedDutycycle;
    ftmParam.firstEdgeDelayPercent = 0U;

    PRINTF("ftm0 task started\r\n");
	FTM_DisableInterrupts(BOARD_FTM_BASEADDR, FTM_CHANNEL_INTERRUPT_ENABLE);
    FTM_SetupPwm(BOARD_FTM_BASEADDR, &ftmParam, 1U, kFTM_CombinedPwm, 24000U, FTM_SOURCE_CLOCK);

    ftmParam.chnlNumber = kFTM_Chnl_2;

//    FTM_SetupPwm(BOARD_FTM_BASEADDR, &ftmParam, 1U, kFTM_CenterAlignedPwm, 24000U, FTM_SOURCE_CLOCK);

//	FTM_EnableInterrupts(BOARD_FTM_BASEADDR, FTM_CHANNEL_INTERRUPT_ENABLE);

    /*
     * sync channels 0/1
     * combine channels 0/1
     * make channels 0/1 a complement of each other
     * enable deadtime on channels 0/1
     */
    BOARD_FTM_BASEADDR->COMBINE |= (FTM_COMBINE_COMBINE0_MASK|FTM_COMBINE_COMP0_MASK
    		|FTM_COMBINE_SYNCEN0_MASK|FTM_COMBINE_DTEN0_MASK);

    /*
     * deadtime is configured with a pre-scaler and value
     */
    deadTimeDelay = 0;

	FTM_UpdateChnlEdgeLevelSelect(BOARD_FTM_BASEADDR, pwmHV_PH2, 0U);
//	FTM_UpdateChnlEdgeLevelSelect(BOARD_FTM_BASEADDR, pwmHV_PH1, 0U);
//	FTM_UpdateChnlEdgeLevelSelect(BOARD_FTM_BASEADDR, pwmHV_ENABLE, 0U);

	/* Update PWM duty cycle */
	FTM_UpdatePwmDutycycle(BOARD_FTM_BASEADDR, pwmHV_PH2, alignment, updatedDutycycle);
//	FTM_UpdatePwmDutycycle(BOARD_FTM_BASEADDR, pwmHV_PH1, alignment, updatedDutycycle);
//	FTM_UpdatePwmDutycycle(BOARD_FTM_BASEADDR, pwmHV_ENABLE, alignment, updatedDutycycle);

	/* Software trigger to update registers */
	FTM_SetSoftwareTrigger(BOARD_FTM_BASEADDR, true);

	/* Start channel output with updated dutycycle */
	FTM_UpdateChnlEdgeLevelSelect(BOARD_FTM_BASEADDR, pwmHV_PH2, pwmLevel);
//	FTM_UpdateChnlEdgeLevelSelect(BOARD_FTM_BASEADDR, pwmHV_PH1, pwmLevel);


    FTM_StartTimer(BOARD_FTM_BASEADDR, kFTM_SystemClock);

	while(1)
	{
		xResult = xTaskNotifyWait( 0x00,    /* Don't clear bits on entry. */
				0xffffffff,        /* Clear all bits on exit. */
				&notifyData.notifyBits, /* Stores the notified value. */
				portMAX_DELAY );

		if( xResult == pdPASS )
		{
			SEGGER_RTT_printf(0,"pwm bits ch:%d dc:%d dt:%d\r\n",
					notifyData.notityBytes[0],
					notifyData.notityBytes[1],
					notifyData.notityBytes[2]);
			switch(notifyData.notityBytes[0])
			{
			case 0:
				FTM_UpdateChnlEdgeLevelSelect(BOARD_FTM_BASEADDR, pwmHV_PH2, 0U);
//				FTM_UpdatePwmDutycycle(BOARD_FTM_BASEADDR, pwmHV_PH1, alignment, notifyData.notityBytes[1]);
				FTM_UpdatePwmDutycycle(BOARD_FTM_BASEADDR, pwmHV_PH2, alignment, notifyData.notityBytes[1]);
				FTM_SetSoftwareTrigger(BOARD_FTM_BASEADDR, true);
				deadTimeDelay = notifyData.notityBytes[2];
				BOARD_FTM_BASEADDR->DEADTIME = deadTimeDelay;
//				FTM_UpdateChnlEdgeLevelSelect(BOARD_FTM_BASEADDR, pwmHV_PH1, pwmLevel);
				FTM_UpdateChnlEdgeLevelSelect(BOARD_FTM_BASEADDR, pwmHV_PH2, pwmLevel);
				break;
			case 1:
				FTM_UpdateChnlEdgeLevelSelect(BOARD_FTM_BASEADDR, pwmHV_ENABLE, 0U);
				FTM_UpdatePwmDutycycle(BOARD_FTM_BASEADDR, pwmHV_ENABLE, alignment, notifyData.notityBytes[1]);
				FTM_SetSoftwareTrigger(BOARD_FTM_BASEADDR, true);
				FTM_UpdateChnlEdgeLevelSelect(BOARD_FTM_BASEADDR, pwmHV_ENABLE, pwmLevel);
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

TaskHandle_t GetFtm0TaskHandle()
{
	return tl_Ftm0TaskHandlerId;
}

void StartFtm0Task()
{
	xTaskCreate(Ftm0Task, Ftm0TaskName, FTM0_TASK_STACK_SIZE , NULL, FTM0_TASK_PRIORITY, &tl_Ftm0TaskHandlerId);

}
