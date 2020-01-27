/*
 * debugTask.c
 *
 *  Created on: Dec 11, 2019
 *      Author: TBiberdorf
 */

#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "MK60D10.h"
#include "fsl_uart.h"
#include "CLI/Cli.h"
#include "freeRTOSProject.h"
#include "TaskParameters.h"
#include "DebugTask.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* UART instance and clock */
#define DEBUG_UART UART3
#define DEBUG_UART_CLKSRC kCLOCK_BusClk
#define DEBUG_UART_CLK_FREQ CLOCK_GetFreq(kCLOCK_BusClk)
#define DEBUG_UART_IRQn UART3_RX_TX_IRQn
#define DEBUG_UART_IRQHANDLER UART3_RX_TX_IRQHandler
#define BOARD_DEBUG_UART_BAUDRATE 115200

/*! @brief Ring buffer size (Unit: Byte). */
#define DEBUG_RING_BUFFER_SIZE (256)

/*! @brief Ring buffer to save received data. */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
const char *DebugTaskName = "DebugTask";
static TaskHandle_t  tl_DebugTaskHandlerId = NULL;

/*
  Ring buffer for data input and output, in this example, input data are saved
  to ring buffer in IRQ handler. The main function polls the ring buffer status,
  if there are new data, then send them out.
  Ring buffer full: (((rxIndex + 1) % DEMO_RING_BUFFER_SIZE) == txIndex)
  Ring buffer empty: (rxIndex == txIndex)
 */
uint8_t tl_DebugInRingBuffer[DEBUG_RING_BUFFER_SIZE];
volatile uint16_t tl_DebugInRingEndIdx; /* Index of the data to send out. */
volatile uint16_t tl_DebugInRingStartIdx; /* Index of the memory to save new arrived data. */

uint8_t tl_DebugOutRingBuffer[DEBUG_RING_BUFFER_SIZE];
volatile uint16_t tl_DebugOutRingEndIdx; /* Index of the data to send out. */
volatile uint16_t tl_DebugOutRingStartIdx; /* Index of the memory to save new arrived data. */

/*******************************************************************************
 * Code
 ******************************************************************************/

void DEBUG_UART_IRQHANDLER(void)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	uint8_t data;
	/* If new data arrived. */
	if ((kUART_RxDataRegFullFlag | kUART_RxOverrunFlag) & UART_GetStatusFlags(DEBUG_UART))
	{
		data = UART_ReadByte(DEBUG_UART);

		/* If ring buffer is not full, add data to ring buffer. */
		if (((tl_DebugInRingStartIdx + 1) % DEBUG_RING_BUFFER_SIZE) != tl_DebugInRingEndIdx)
		{
			tl_DebugInRingBuffer[tl_DebugInRingStartIdx] = data;
			tl_DebugInRingStartIdx++;
			tl_DebugInRingStartIdx %= DEBUG_RING_BUFFER_SIZE;

			xTaskNotifyFromISR(GetDebugdTaskHandle(),0xFFFF,eSetBits,&xHigherPriorityTaskWoken);
		}
	}
}

/**
 * @brief output write data to debug uart port
 *
 * @param WriteData pointer to buffer to write
 * @param Size number of bytes to send
 * @return retCode
 */
uint32_t DebugTaskWrite(const char *WriteData, size_t Size)
{
	uint32_t retCode = 0;
	uint16_t idx;
	for(idx=0; idx < Size; idx++)
	{
		if (((tl_DebugOutRingStartIdx + 1) % DEBUG_RING_BUFFER_SIZE) != tl_DebugOutRingEndIdx)
		{
			tl_DebugOutRingBuffer[tl_DebugOutRingStartIdx] = (uint8_t)WriteData[idx];
			tl_DebugOutRingStartIdx++;
			tl_DebugOutRingStartIdx %= DEBUG_RING_BUFFER_SIZE;
		}
		else
		{
			retCode = idx;
			break;
		}
	}

	return retCode;
}

void DebugTask(void *pvParameters)
{
	uart_config_t config;
	/*
	 * config.baudRate_Bps = 115200U;
	 * config.parityMode = kUART_ParityDisabled;
	 * config.stopBitCount = kUART_OneStopBit;
	 * config.txFifoWatermark = 0;
	 * config.rxFifoWatermark = 1;
	 * config.enableTx = false;
	 * config.enableRx = false;
	 */
	UART_GetDefaultConfig(&config);
	config.baudRate_Bps = BOARD_DEBUG_UART_BAUDRATE;
	config.enableTx = true;
	config.enableRx = true;

	char startUpMessage[64];
	int32_t strLength;
	const TickType_t xMaxBlockTime = portMAX_DELAY;//pdMS_TO_TICKS( 1 );
	union _Long_Char_Join
	{
		uint32_t notifyBits;
		char notityBytes[4];
	} notifyData,storedData;
	uint16_t irqCountDelay = 0;
	uint16_t irqCountDelayPeriod = 100;

	BaseType_t xResult;

//	UART_Init(DEBUG_UART, &config, DEBUG_UART_CLK_FREQ);
	strLength = sprintf(startUpMessage,"\r\nNGR Relay Board version %d.%d.%d.%d\r\n",MAJOR_VERSION,MINOR_VERSION,VERSION_VERSION,REVISION_VERSION);

	/* Send g_tipString out. */
	UART_WriteBlocking(DEBUG_UART, (uint8_t *)startUpMessage, strLength);

	/* Enable RX interrupt. */
//	UART_EnableInterrupts(DEBUG_UART, kUART_RxDataRegFullInterruptEnable | kUART_RxOverrunInterruptEnable);
//	NVIC_SetPriority(DEBUG_UART_IRQn, 1);
//	EnableIRQ(DEBUG_UART_IRQn);

	printf("CLI/Debug Task started\r\n");
	storedData.notifyBits = 0;
	while(1)
	{

		xResult = xTaskNotifyWait( 0x00,    /* Don't clear bits on entry. */
				0xffffffff,        /* Clear all bits on exit. */
				&notifyData.notifyBits, /* Stores the notified value. */
				xMaxBlockTime );

		if( xResult == pdPASS )
		{
//			SEGGER_RTT_printf(0,"debug %d\r\n",notifyData.notifyBits);

//			while ( !(kUART_TxDataRegEmptyFlag & UART_GetStatusFlags(DEBUG_UART)) )
//			{
//				; // wait for tx buffer to be empty so we can send a byte
//			}
//			// output information to debug port
//			UART_WriteByte(DEBUG_UART, notifyData.notityBytes[0]);
//			storedData.notifyBits = notifyData.notifyBits;
//			vTaskDelay(1);

		}
//		else
//		{
//			if( irqCountDelay++ > irqCountDelayPeriod)
//			{
//				irqCountDelay = 0;
//				while ( !(kUART_TxDataRegEmptyFlag & UART_GetStatusFlags(DEBUG_UART)) )
//				{
//					; // wait for tx buffer to be empty so we can send a byte
//				}
//				// output information to debug port
////				UART_WriteByte(DEBUG_UART, storedData.notityBytes[0]);
//				vTaskDelay(1);
//			}
//		}

		/* Send data only when UART TX register is empty and ring buffer has data to send out. */
		while (tl_DebugInRingStartIdx != tl_DebugInRingEndIdx)
		{
			// determine if CLI input command is present
			cli(tl_DebugInRingBuffer[tl_DebugInRingEndIdx]);
			tl_DebugInRingEndIdx++;
			tl_DebugInRingEndIdx %= DEBUG_RING_BUFFER_SIZE;
		}
		while(tl_DebugOutRingStartIdx != tl_DebugOutRingEndIdx)
		{
			if ((kUART_TxDataRegEmptyFlag & UART_GetStatusFlags(DEBUG_UART)) && (tl_DebugOutRingStartIdx != tl_DebugOutRingEndIdx))
			{
				// output information to debug port
				UART_WriteByte(DEBUG_UART, tl_DebugOutRingBuffer[tl_DebugOutRingEndIdx]);
				tl_DebugOutRingEndIdx++;
				tl_DebugOutRingEndIdx %= DEBUG_RING_BUFFER_SIZE;
			}

			vTaskDelay(1);

		}


//		vTaskDelay(10);
	}
}

TaskHandle_t GetDebugdTaskHandle()
{
	return tl_DebugTaskHandlerId;
}

void StartDebugTask()
{
	xTaskCreate(DebugTask, DebugTaskName, DEBUG_TASK_STACK_SIZE , NULL, DEBUG_TASK_PRIORITY, &tl_DebugTaskHandlerId);

}
