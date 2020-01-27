/*
 * GpiocOperations.c
 *
 *  Created on: Dec 13, 2019
 *      Author: TBiberdorf
 */

#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "MK60D10.h"
#include "fsl_uart.h"
#include "TaskParameters.h"

#include "fsl_debug_console.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
#include "fsl_common.h"
#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "CMSIS/RTT/SEGGER_RTT.h"
#include "LED/LEDOperations.h"
#include "UartTasks/DebugTask.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define BOARD_INPUT_IRQ_HANDLER GPIO_1_IRQHANDLER

#define BOARD_TestC1_GPIO GPIOC
#define BOARD_TestC1_PORT PORTC
#define BOARD_TestC1_PIN 1
#define BOARD_SW_IRQ BOARD_SW1_IRQ
#define BOARD_SW_NAME BOARD_SW1_NAME

static volatile uint16_t ts_Pin1TestCount = 0;
static uint32_t tl_ButtonPressed = false;
static uint32_t tl_ChangeConfigurationFlag = 0;
const char *GpioTaskName = "GpioTask";

//volatile uint32_t ts_IrqCount = 0;

typedef enum _PinSelection_e
{
	ePinSelectedNone,
	ePinSelectedOne,
}ePinSelection_t;

typedef enum _PullUpPower_e
{
	ePullUpPowerLow,
	ePullUpPowerHigh,
}ePullUpPower_t;

typedef enum _PassiveFilter_e
{
	ePassiveFilterOff,
	ePassiveFilterOn,
}ePassiveFilter_t;

typedef enum _ActiveFilter_e
{
	eActiveFilterOff,
	eActiveFilterOn,
}eActiveFilter_t;

typedef struct _InputPinOperation_s
{
	ePinSelection_t PinSelection;
	ePullUpPower_t PullUpPower;
	ePassiveFilter_t PassiveFilter;
	ePassiveFilter_t ActiveFilter;
}sInputPinOperation_t;

sInputPinOperation_t tl_CurrentInputPin = {
		.PinSelection = ePinSelectedOne,
		.PullUpPower=ePullUpPowerHigh,
		.PassiveFilter=ePassiveFilterOn,
		.ActiveFilter=eActiveFilterOff};

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Interrupt service fuction of switch.
 *
 * This function toggles the LED
 */
void PORTC_IRQHandler(void)
{
	uint32_t irqPinState;

	irqPinState = GPIO_GetPinsInterruptFlags(BOARD_TestC1_GPIO);

	if( irqPinState & (1U << BOARD_TestC1_PIN))
	{
//		ts_IrqCount++;
    /* Clear external interrupt flag. */
		GPIO_ClearPinsInterruptFlags(BOARD_TestC1_GPIO, 1U << BOARD_TestC1_PIN);
		ts_Pin1TestCount++;
	}
    /* Change state of button. */
    tl_ButtonPressed = true;
}


/**
 * @brief set the input pin
 */
uint32_t setInputPin(uint32_t Pin)
{

	tl_CurrentInputPin.PinSelection = ePinSelectedOne;
	tl_ChangeConfigurationFlag = 1;
	return tl_CurrentInputPin.PinSelection = ePinSelectedOne;
}

/**
 * @brief modify the Pullup power selection
 */
uint32_t setPullupPowerLevel(uint32_t PowerLevel)
{
	uint32_t powerLevel = 0;
	if(PowerLevel)
	{
		tl_CurrentInputPin.PullUpPower = ePullUpPowerHigh;
		powerLevel = 1;
	}
	else
	{
		tl_CurrentInputPin.PullUpPower = ePullUpPowerLow;
	}

	tl_ChangeConfigurationFlag = 1;
	return powerLevel;
}

/**
 * @brief modify the passive filter selection
 */
uint32_t setPassiveFilter(uint32_t PassiveFilter)
{
	uint32_t passiveFilter= 0;
	if(PassiveFilter)
	{
		tl_CurrentInputPin.PassiveFilter = ePassiveFilterOn;
		passiveFilter = 1;
	}
	else
	{
		tl_CurrentInputPin.PassiveFilter = ePassiveFilterOff;
	}

	tl_ChangeConfigurationFlag = 1;
	return passiveFilter;
}

/**
 * @brief modify the active filter selection
 */
uint32_t setActiveFilter(uint32_t ActiveFilter)
{
	uint32_t activeFilter= 0;
	if(ActiveFilter)
	{
		tl_CurrentInputPin.ActiveFilter = eActiveFilterOn;
		activeFilter = 1;
	}
	else
	{
		tl_CurrentInputPin.ActiveFilter = eActiveFilterOff;
	}

	tl_ChangeConfigurationFlag = 1;
	return activeFilter;
}

/**
 * @brief display current input pin selection settings
 */
void getInputPinConfiguration(char *CurrentConfigurationPtr,size_t *Size)
{
	const char *pinConfigStr1 ="Input Pin: ";
	const char *pinConfigStr2 ="one\r\n";
	const char *pinConfigStr3 ="none\r\n";
	const char *pinConfigStr4 ="Pullup Pwr: ";
	const char *pinConfigStr5 ="high\r\n";
	const char *pinConfigStr6 ="low\r\n";
	const char *pinConfigStr7 ="Passive Filter: ";
	const char *pinConfigStr8 ="on\r\n";
	const char *pinConfigStr9 ="off\r\n";
	const char *pinConfigStr10 ="Active Filter: ";
	const char *pinConfigStr11 ="on\r\n";
	const char *pinConfigStr12 ="off\r\n";
	const char *pinConfigStr13 ="test Count: ";

//	GPIO_TogglePinsOutput(GPIOC,LED2);
//	GPIO_SetPinsOutput(GPIOC,LED1);
	uint16_t currentPin1TestCount = 0;

	uint16_t pin1TestCount;
	size_t sentSize = 0;
	size_t length;
	currentPin1TestCount = ts_Pin1TestCount;
	char *ptrDetails = CurrentConfigurationPtr;
	length = strlen(pinConfigStr1);
	memcpy(ptrDetails,pinConfigStr1,length);
	ptrDetails += length;
	sentSize += length;
	switch (tl_CurrentInputPin.PinSelection)
	{
	case ePinSelectedOne:
		length = strlen(pinConfigStr2);
		memcpy(ptrDetails,pinConfigStr2,length);
		ptrDetails += length;
		sentSize += length;
		break;
	case ePinSelectedNone:
		length = strlen(pinConfigStr3);
		memcpy(ptrDetails,pinConfigStr3,length);
		ptrDetails += length;
		sentSize += length;
		break;
	}


	// display pullup strength
	length = strlen(pinConfigStr4);
	memcpy(ptrDetails,pinConfigStr4,length);
	ptrDetails += length;
	sentSize += length;
	switch (tl_CurrentInputPin.PullUpPower)
	{
	case ePullUpPowerHigh:
		length = strlen(pinConfigStr5);
		memcpy(ptrDetails,pinConfigStr5,length);
		ptrDetails += length;
		sentSize += length;
		break;
	case ePullUpPowerLow:
		length = strlen(pinConfigStr6);
		memcpy(ptrDetails,pinConfigStr6,length);
		ptrDetails += length;
		sentSize += length;
		break;
	}

	// display passive filter settings
	length = strlen(pinConfigStr7);
	memcpy(ptrDetails,pinConfigStr7,length);
	ptrDetails += length;
	sentSize += length;
	switch (tl_CurrentInputPin.PassiveFilter)
	{
	case ePassiveFilterOn:
		length = strlen(pinConfigStr8);
		memcpy(ptrDetails,pinConfigStr8,length);
		ptrDetails += length;
		sentSize += length;
		break;
	case ePassiveFilterOff:
		length = strlen(pinConfigStr9);
		memcpy(ptrDetails,pinConfigStr9,length);
		ptrDetails += length;
		sentSize += length;
		break;
	}


	// display Active filter settings
	length = strlen(pinConfigStr10);
	memcpy(ptrDetails,pinConfigStr10,length);
	ptrDetails += length;
	sentSize += length;
	switch (tl_CurrentInputPin.ActiveFilter)
	{
	case eActiveFilterOn:
		length = strlen(pinConfigStr11);
		memcpy(ptrDetails,pinConfigStr11,length);
		ptrDetails += length;
		sentSize += length;
		break;
	case eActiveFilterOff:
		length = strlen(pinConfigStr12);
		memcpy(ptrDetails,pinConfigStr12,length);
		ptrDetails += length;
		sentSize += length;
		break;
	}

	// display test count settings
	length = strlen(pinConfigStr13);
	memcpy(ptrDetails,pinConfigStr13,length);
	ptrDetails += length;
	sentSize += length;

	length = sprintf(ptrDetails,"%d\r\n",currentPin1TestCount);
	ptrDetails += length;
	sentSize += length;

	*Size = sentSize;
}

/**
 * @brief clear the interrupt test count
 */
void gpioClearTestCount()
{
	SEGGER_RTT_printf(0,"clear ts_Pin1TestCount");
	ts_Pin1TestCount = 0;
}

static void GpioTask(void *pvParameters)
{
	SEGGER_RTT_printf(0,"Gpio Task started\r\n");
	uint16_t currentPin1TestCount = 0;


	while(1)
	{
//		if(tl_ChangeConfigurationFlag)
//		{
//			tl_ChangeConfigurationFlag = 0;
//			switch(tl_CurrentInputPin.PinSelection)
//			{
//			case ePinSelectedOne:
//			    PORTC->PCR[1] &= ~(PORT_PCR_PFE(kPORT_PassiveFilterEnable)|PORT_PCR_DSE(kPORT_HighDriveStrength));
//			    if(tl_CurrentInputPin.PassiveFilter)
//			    {
//			         /* Passive Filter Enable: Passive input filter is disabled on the corresponding pin. */
//			    	PORTC->PCR[1] |= PORT_PCR_PFE(kPORT_PassiveFilterEnable);
//					SEGGER_RTT_printf(0,"passive On\r\n");
//			    }
//			    if(tl_CurrentInputPin.PullUpPower)
//			    {
//			         /* Drive Strength Enable: High drive strength is configured on the corresponding pin, if pin is
//			          * configured as a digital output. */
//			    	PORTC->PCR[1] |= PORT_PCR_DSE(kPORT_HighDriveStrength);
//					SEGGER_RTT_printf(0,"power high\r\n");
//			    }
//				break;
//			case ePinSelectedNone:
//				break;
//
//			}
//		}
//
//		if( currentPin1TestCount != ts_Pin1TestCount)
//		{
//			currentPin1TestCount = ts_Pin1TestCount;
//			SEGGER_RTT_printf(0,"irq cnt: %d\r\n",currentPin1TestCount);
//			if(currentPin1TestCount)
//			{
//				SEGGER_RTT_printf(0,"debug ");
//				if(xTaskNotify(GetDebugdTaskHandle(),(uint32_t)currentPin1TestCount,eSetBits) == pdPASS)
//				{
//					SEGGER_RTT_printf(0,"sent\r\n");
//				}
//				SEGGER_RTT_printf(0,"LED  ");
//				if( xTaskNotify(GetLedTaskHandle(),(uint32_t)currentPin1TestCount,eSetBits) == pdPASS )
//				{
//					SEGGER_RTT_printf(0,"sent\r\n");
//				}
//				else
//				{
//					SEGGER_RTT_printf(0,"fail\r\n");
//				}
//				SEGGER_RTT_printf(0,"done\r\n");
//
//			}
//		}
		vTaskDelay(1000);
	}
}


void StartGpioTask()
{
	xTaskCreate(GpioTask, GpioTaskName, GPIO_TASK_STACK_SIZE , NULL, GPIO_TASK_PRIORITY, NULL);

}


