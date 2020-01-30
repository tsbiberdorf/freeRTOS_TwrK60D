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
#include "pin_mux.h"
#include "fsl_gpio.h"
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
volatile uint32_t frequencyPWM = 16000;
	ftm_pwm_mode_t alignment = kFTM_CombinedPwm;
    ftm_pwm_level_select_t pwmLevel = kFTM_LowTrue;
    ftm_chnl_pwm_signal_param_t ftmParam[4];
	union _Long_Char_Join
	{
		uint32_t notifyBits;
		char notityBytes[4];
	} notifyData;

    ftm_chnl_t pwmHV_PH2 = kFTM_Chnl_0;
    ftm_chnl_t pwmHV_PH1 = kFTM_Chnl_1;
    ftm_chnl_t pwmHV_ENABLE = kFTM_Chnl_2;

    /* Configure ftm params with frequency 24kHZ */
    ftmParam[0].chnlNumber = kFTM_Chnl_0;
    ftmParam[0].level = kFTM_LowTrue;
    ftmParam[0].dutyCyclePercent = 50;
    ftmParam[0].firstEdgeDelayPercent = 0U;

    ftmParam[1].chnlNumber = kFTM_Chnl_1;
    ftmParam[1].level = kFTM_LowTrue;
    ftmParam[1].dutyCyclePercent = 50;
    ftmParam[1].firstEdgeDelayPercent = 0U;

    ftmParam[2].chnlNumber = kFTM_Chnl_2;
    ftmParam[2].level = kFTM_LowTrue;
    ftmParam[2].dutyCyclePercent = 50;
    ftmParam[2].firstEdgeDelayPercent = 0U;

    ftmParam[3].chnlNumber = kFTM_Chnl_3;
    ftmParam[3].level = kFTM_LowTrue;
    ftmParam[3].dutyCyclePercent = 50;
    ftmParam[3].firstEdgeDelayPercent = 0U;

    PRINTF("ftm0 task started\r\n");
	FTM_DisableInterrupts(BOARD_FTM_BASEADDR, FTM_CHANNEL_INTERRUPT_ENABLE);
    FTM_SetupPwm(BOARD_FTM_BASEADDR, &(ftmParam[0]), 1U, kFTM_CombinedPwm, frequencyPWM, FTM_SOURCE_CLOCK);
    FTM_SetupPwm(BOARD_FTM_BASEADDR, &(ftmParam[2]), 1U, kFTM_EdgeAlignedPwm, frequencyPWM, FTM_SOURCE_CLOCK);

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

//    BOARD_FTM_BASEADDR->COMBINE |= (FTM_COMBINE_COMBINE1_MASK|FTM_COMBINE_COMP1_MASK
//    		|FTM_COMBINE_SYNCEN1_MASK|FTM_COMBINE_DTEN1_MASK);
    /*
     * deadtime is configured with a pre-scaler and value
     */
    deadTimeDelay = 0;

    /* Software trigger to update registers */
	FTM_SetSoftwareTrigger(BOARD_FTM_BASEADDR, true);

//	FTM_UpdateChnlEdgeLevelSelect(BOARD_FTM_BASEADDR, kFTM_Chnl_0, 0U);
//	FTM_UpdatePwmDutycycle(BOARD_FTM_BASEADDR, kFTM_Chnl_0, kFTM_CombinedPwm, 50);
//	FTM_UpdateChnlEdgeLevelSelect(BOARD_FTM_BASEADDR, kFTM_Chnl_0, kFTM_LowTrue);
//
//	FTM_UpdateChnlEdgeLevelSelect(BOARD_FTM_BASEADDR, kFTM_Chnl_2, 0U);
//	FTM_UpdatePwmDutycycle(BOARD_FTM_BASEADDR, kFTM_Chnl_2, kFTM_CombinedPwm, 50);
//	FTM_UpdateChnlEdgeLevelSelect(BOARD_FTM_BASEADDR, kFTM_Chnl_2, kFTM_LowTrue);

//	FTM_UpdateChnlEdgeLevelSelect(BOARD_FTM_BASEADDR, pwmHV_PH2, 0U);
//	FTM_UpdateChnlEdgeLevelSelect(BOARD_FTM_BASEADDR, pwmHV_PH1, 0U);
//	FTM_UpdateChnlEdgeLevelSelect(BOARD_FTM_BASEADDR, pwmHV_ENABLE, 0U);

	/* Update PWM duty cycle */
//	FTM_UpdatePwmDutycycle(BOARD_FTM_BASEADDR, pwmHV_PH2, alignment, updatedDutycycle);
//	FTM_UpdatePwmDutycycle(BOARD_FTM_BASEADDR, pwmHV_PH1, alignment, updatedDutycycle);
//	FTM_UpdatePwmDutycycle(BOARD_FTM_BASEADDR, pwmHV_ENABLE, alignment, updatedDutycycle);


	/* Start channel output with updated dutycycle */
//	FTM_UpdateChnlEdgeLevelSelect(BOARD_FTM_BASEADDR, pwmHV_PH2, pwmLevel);
//	FTM_UpdateChnlEdgeLevelSelect(BOARD_FTM_BASEADDR, pwmHV_PH1, pwmLevel);


    FTM_StartTimer(BOARD_FTM_BASEADDR, kFTM_SystemClock);


//    changeFlag = 6;
//    while(changeFlag--)
//    {
//    	GPIO_ClearPinsOutput(BOARD_INITPINS_ch2_GPIO, 1U << BOARD_INITPINS_ch2_PIN);
//    	GPIO_ClearPinsOutput(BOARD_INITPINS_ch3_GPIO, 1U << BOARD_INITPINS_ch3_PIN);
//
//    	GPIO_SetPinsOutput(BOARD_INITPINS_ch2_GPIO, 1U << BOARD_INITPINS_ch2_PIN);
//    	GPIO_SetPinsOutput(BOARD_INITPINS_ch3_GPIO, 1U << BOARD_INITPINS_ch3_PIN);
//    }

	while(1)
	{
		xResult = xTaskNotifyWait( 0x00,    /* Don't clear bits on entry. */
				0xffffffff,        /* Clear all bits on exit. */
				&notifyData.notifyBits, /* Stores the notified value. */
				portMAX_DELAY );

		if( xResult == pdPASS )
		{
			SEGGER_RTT_printf(0,"pwm bits ch:%d dc:%d dt:%d fr:%d\r\n",
					notifyData.notityBytes[0],
					notifyData.notityBytes[1],
					notifyData.notityBytes[2],
					notifyData.notityBytes[3]*1000);
			switch(notifyData.notityBytes[0])
			{
			case 0:
				FTM_UpdateChnlEdgeLevelSelect(BOARD_FTM_BASEADDR, pwmHV_PH2, 0U);
			    FTM_SetupPwm(BOARD_FTM_BASEADDR, &(ftmParam[0])
			    		, 1U
						, kFTM_CombinedPwm
						, notifyData.notityBytes[3]*1000
						, FTM_SOURCE_CLOCK);
			    FTM_SetupPwm(BOARD_FTM_BASEADDR, &(ftmParam[2])
			    		, 1U
						, kFTM_EdgeAlignedPwm
						, notifyData.notityBytes[3]*1000
						, FTM_SOURCE_CLOCK);
				FTM_SetSoftwareTrigger(BOARD_FTM_BASEADDR, true);
				deadTimeDelay = notifyData.notityBytes[2];
				BOARD_FTM_BASEADDR->DEADTIME = deadTimeDelay;
				FTM_UpdatePwmDutycycle(BOARD_FTM_BASEADDR, pwmHV_PH2, alignment, notifyData.notityBytes[1]);

				//				FTM_UpdateChnlEdgeLevelSelect(BOARD_FTM_BASEADDR, pwmHV_PH1, pwmLevel);
				FTM_UpdateChnlEdgeLevelSelect(BOARD_FTM_BASEADDR, pwmHV_PH2, pwmLevel);
				break;
			case 1:
				FTM_UpdateChnlEdgeLevelSelect(BOARD_FTM_BASEADDR, pwmHV_ENABLE, 0U);
				FTM_UpdatePwmDutycycle(BOARD_FTM_BASEADDR, pwmHV_ENABLE, kFTM_EdgeAlignedPwm, notifyData.notityBytes[1]);
				FTM_SetSoftwareTrigger(BOARD_FTM_BASEADDR, true);
				FTM_UpdateChnlEdgeLevelSelect(BOARD_FTM_BASEADDR, pwmHV_ENABLE, pwmLevel);
				break;
			}
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
