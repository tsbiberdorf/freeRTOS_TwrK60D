/*
 * Cli.c
 *
 *  Created on: Dec 11, 2019
 *      Author: TBiberdorf
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "../DebugTask.h"
#include "GPIO/GpiocOperations.h"
#include "LED/LEDOperations.h"
#include "freeRTOSProject.h"
#include "CMSIS/RTT/SEGGER_RTT.h"
#include "fsl_debug_console.h"

#define CLI_BUFFER_SIZE (256)

static uint8_t tl_cliData[CLI_BUFFER_SIZE];
static uint16_t tl_cliIdx = 0;

extern TaskHandle_t GetFtm0TaskHandle();

/*
 * parse InputStr for decimal value.
 * @return ptrEnd will be null if no more data to parse
 */
static uint8_t * ParseDecimal(uint8_t *InputStr, uint32_t *PtrDecimal)
{
	int16_t idx;
	uint8_t *ptrStart;
	uint8_t *ptrEnd = NULL;
	uint32_t value;
	ptrStart = InputStr;
	idx = 0;

	/*
	 * first look for the first digit of the decimal number
	 */
	while(*(ptrStart+idx) == ' ')
	{
		idx++;
		/*
		 * @todo
		 * may want to put a check here, to make sure the idx does not
		 * get too big
		 */
	}
	ptrStart = ptrStart+idx;
	value = strtoul((char*)ptrStart,(char **)&ptrEnd,10);
	*PtrDecimal = value;

	if(ptrEnd == NULL)
	{
		/*
		 * no more data to parse
		 */
	}
	return ptrEnd;
}


/*
 * parse InputStr for hex value.
 * @return ptrEnd will be null if no more data to parse
 */
static uint8_t * ParseHex(uint8_t *InputStr, uint32_t *PtrHex)
{
	int16_t idx;
	uint8_t *ptrStart;
	uint8_t *ptrEnd = NULL;
	uint32_t value;
	ptrStart = InputStr;
	idx = 0;

	/*
	 * first look for the first digit of the decimal number
	 */
	while(*(ptrStart+idx) == ' ')
	{
		idx++;
		/*
		 * @todo
		 * may want to put a check here, to make sure the idx does not
		 * get too big
		 */
	}
	ptrStart = ptrStart+idx;
	value = strtoul((char*)ptrStart,(char **)&ptrEnd,16);
	*PtrHex = value;

	if(ptrEnd == NULL)
	{
		/*
		 * no more data to parse
		 */
	}
	return ptrEnd;
}

/**
 * @details set up options for a CLI command
 */
typedef struct _cliCommandOptions_s
{
	const char *option; /** text of the option, include '-' */
	const uint8_t optionSize; /** number of characters in the option text including the '-' character */
	int32_t (*optionCallback)(char *); /** callback method to call if the option is detected */
}s_cliCommandOptions_t;

typedef struct _cliCommands_s
{
	const char *cmdString;
	s_cliCommandOptions_t *option;
}s_cliCommands_t;


static const char *helpCmdText[] = {
		"NGR help commands:\r\n",
		"led\r\n",
		"pwm\r\n",
		"\0"
};

int32_t helpCmd(char *Param)
{
	uint16_t helpIdx=0;
	char *version[64];

	sprintf(version,"\r\nVersion %d.%d.%d.%d\r\n",MAJOR_VERSION,MINOR_VERSION,VERSION_VERSION,REVISION_VERSION);
	DebugTaskWrite(version,strlen(version));

	while(helpCmdText[helpIdx][0])
	{
		DebugTaskWrite(helpCmdText[helpIdx],strlen((const char *)helpCmdText[helpIdx]));
		helpIdx++;
	}
	return 0;
}

s_cliCommandOptions_t helpOptions[]= {
		{"-h",2,helpCmd},{"-?",2,helpCmd},{NULL,0,helpCmd}
};

const char *ledCmdText[] = {
		"led help commands:\r\n",
		"-r [0/1] turn Red LED on/off\r\n",
		"-g [0/1] turn Green LED on/off\r\n",
		"-b [0/1] turn Blue LED on/off\r\n",
		"\0"
};

static int32_t ledHelpCmd(char *Param)
{
	uint16_t helpIdx=0;

	while(ledCmdText[helpIdx][0])
	{
		DebugTaskWrite(ledCmdText[helpIdx],strlen(ledCmdText[helpIdx]));
		helpIdx++;
	}
	return 0;
}

static int32_t ledGreenSelect(char *Param)
{
	uint32_t value;
	PRINTF("led Green ");
	if( ParseDecimal((uint8_t *)Param,&value) )
	{
		if(value)
		{
			PRINTF("ON\r\n");
			xTaskNotify(GetLedTaskHandle(),GREEN_RED_ON,eSetBits);
		}
		else
		{
			PRINTF("OFF\r\n");
			xTaskNotify(GetLedTaskHandle(),GREEN_RED_OFF,eSetBits);
		}
	}
	else
	{
		PRINTF("\r\n");
	}
	return 0;
}

static int32_t ledRedSelect(char *Param)
{
	uint32_t value;
	PRINTF("led Red ");
	if( ParseDecimal((uint8_t *)Param,&value) )
	{
		if(value)
		{
			PRINTF("ON\r\n");
			xTaskNotify(GetLedTaskHandle(),LED_RED_ON,eSetBits);
		}
		else
		{
			PRINTF("OFF\r\n");
			xTaskNotify(GetLedTaskHandle(),LED_RED_OFF,eSetBits);
		}
	}
	else
	{
		PRINTF("\r\n");
	}
	return 0;
}

static int32_t ledBlueSelect(char *Param)
{
	uint32_t value;
	PRINTF("led Blue");
	if( ParseDecimal((uint8_t *)Param,&value) )
	{
		if(value)
		{
			PRINTF("ON\r\n");
			xTaskNotify(GetLedTaskHandle(),BLUE_RED_ON,eSetBits);
		}
		else
		{
			PRINTF("OFF\r\n");
			xTaskNotify(GetLedTaskHandle(),BLUE_RED_OFF,eSetBits);
		}
	}
	else
	{
		PRINTF("\r\n");
	}
	return 0;	return 0;
}

/**
 * CLI options supported with the command: led
 */
s_cliCommandOptions_t ledOptions[]= {
		{"-g",2,ledGreenSelect},
		{"-r",2,ledRedSelect},
		{"-b",2,ledBlueSelect},

		{"-h",2,ledHelpCmd},{"-?",2,ledHelpCmd},{NULL,0,ledHelpCmd}
};

const char *pwmCmdText[] = {
		"pwm help commands:\r\n",
		"-c [0/1] [duty cycle] [dead time]\r\n",
		"\0"
};

static int32_t pwmHelpCmd(char *Param)
{
	uint16_t helpIdx=0;

	while(pwmCmdText[helpIdx][0])
	{
		DebugTaskWrite(pwmCmdText[helpIdx],strlen(pwmCmdText[helpIdx]));
		helpIdx++;
	}
	return 0;
}
/**
 * To change the percent of pwm active for the assigned channel
 */
static int32_t pwmChannelSetting(char *Param)
{
	union _Long_Char_Join
	{
		uint32_t pwmBits;
		uint8_t pwmBytes[4];
	} pwmTaskData;
	uint32_t channel,value,deadtime,freqKhz;
	uint8_t *ptrValue;
	PRINTF("pwm ch: ");

	ptrValue = ParseDecimal((uint8_t *)Param,&channel);
	if( ptrValue )
	{
		if(channel == 0 )
		{
			ptrValue = ParseDecimal((uint8_t *)ptrValue,&value);
			ptrValue = ParseDecimal((uint8_t *)ptrValue,&deadtime);
			ptrValue = ParseDecimal((uint8_t *)ptrValue,&freqKhz);
			if(value > 100)
			{
				value = 100;
			}
			pwmTaskData.pwmBytes[0] = (uint8_t)channel;
			pwmTaskData.pwmBytes[1] = (uint8_t)value;
			pwmTaskData.pwmBytes[2] = (uint8_t)deadtime;
			pwmTaskData.pwmBytes[3] = (uint8_t)freqKhz;
			xTaskNotify(GetFtm0TaskHandle(),pwmTaskData.pwmBits,eSetBits);
			PRINTF("%d %d %d\r\n",channel,value,deadtime);
		}
		else if(channel == 1 )
		{
			ptrValue = ParseDecimal((uint8_t *)ptrValue,&value);
			ptrValue = ParseDecimal((uint8_t *)ptrValue,&deadtime);
			ptrValue = ParseDecimal((uint8_t *)ptrValue,&freqKhz);
			if(value > 100)
			{
				value = 100;
			}
			pwmTaskData.pwmBytes[0] = (uint8_t)channel;
			pwmTaskData.pwmBytes[1] = (uint8_t)value;
			pwmTaskData.pwmBytes[2] = (uint8_t)deadtime;
			pwmTaskData.pwmBytes[3] = (uint8_t)freqKhz;
			xTaskNotify(GetFtm0TaskHandle(),pwmTaskData.pwmBits,eSetBits);
			PRINTF("%d %d %d\r\n",channel,value,deadtime);
		}
		else
		{
			PRINTF("n/a %d %d\r\n",value,deadtime);
		}
	}
	else
	{
		PRINTF("\r\n");
	}
	return 0;

}

/**
 *  CLI options supported with the command: pwm
 */
s_cliCommandOptions_t pwmOptions[]= {
		{"-c",2,pwmChannelSetting},
		{"-h",2,pwmHelpCmd},{"-?",2,pwmHelpCmd},{NULL,0,pwmHelpCmd}
};

s_cliCommands_t userCmds[]= {
		{"led",ledOptions},
		{"pwm",pwmOptions},
		{"help" ,helpOptions},
		{NULL,NULL}
};

int32_t LoopOptions(s_cliCommands_t *UserCmdOptions,uint8_t *PtrOption)
{
#define MAX_OPTION_SIZE (5) // max size the option field can be
	char userOption[MAX_OPTION_SIZE];
	char *ptrUserOption;
	int8_t idx;
	int8_t optionSize=0;
	int32_t status = -1;
	int8_t userOptionSize=0;

	ptrUserOption = (char *)PtrOption;
	idx = 0;
	while( (ptrUserOption[idx] != ' ') && (ptrUserOption[idx] != 0))
	{
		userOption[idx] = ptrUserOption[idx];
		idx++;
		userOptionSize++;
		if(idx == MAX_OPTION_SIZE)
		{
			SEGGER_RTT_printf(0,"unknown command\r\n");
			status = -1;
			goto exitMethod;
		}
		else
		{
			userOption[idx] = 0;
		}
	}
	for(idx=0; UserCmdOptions->option[idx].option != NULL;)
	{
		optionSize = UserCmdOptions->option[idx].optionSize; // str len of the option to compare against
//		userOptionSize = strlen(PtrOption); // str len of the option requested by the user
		if(optionSize == userOptionSize)
		{
			if( strncmp(userOption,UserCmdOptions->option[idx].option,optionSize) == 0)
			{
				/*
				 * we have a matching option and the pointer to where the parameter
				 * will be located.
				 */

				/*
				 * verify we have a method to call
				 */
				if( UserCmdOptions->option[idx].optionCallback != 0)
				{
					status = UserCmdOptions->option[idx].optionCallback((char *)(PtrOption+optionSize));
				}
			}
		}
		idx++;
	}

	/*
	 * for commands that have no options
	 */
	if( status < 0 )
	{
		/*
		 * verify we have a method to call
		 */
		if( UserCmdOptions->option[idx].optionCallback != 0)
		{
			status = UserCmdOptions->option[idx].optionCallback((char *)(PtrOption+optionSize));
		}
	}

	exitMethod:
	return status;
}

int32_t LoopCmds(uint8_t *EnteredCmd)
{
	int32_t status = -1;
	int32_t cmdStringLength;
	uint8_t *ptrOption = NULL;
	s_cliCommands_t *ptrUserCmds = &userCmds[0];
	while( ptrUserCmds->cmdString != NULL)
	{
		cmdStringLength = strlen(ptrUserCmds->cmdString);
		if( strncmp((char *)EnteredCmd,ptrUserCmds->cmdString,cmdStringLength) == 0)
		{
			if(*(EnteredCmd+cmdStringLength) == '+')
			{
				ptrOption = EnteredCmd+cmdStringLength;
			}
			else
			{
				ptrOption = EnteredCmd+cmdStringLength+1;
			}
			status = LoopOptions(ptrUserCmds,ptrOption);
		}
		ptrUserCmds++;
	}

	if(status < 0)
	{
		SEGGER_RTT_printf(0,"unknown command\r\n");
	}

	return status;
}

//static int32_t parseInputCLI( uint8_t *Input,int8_t Size)
//{
//	int32_t cmdFound = 0;
//	uint16_t idx;
//	for(idx=0;idx<Size;idx++)
//	{
//		tl_cliData[tl_cliIdx++] = Input[idx];
//
//		if( tl_cliData[tl_cliIdx-1] == '\n')
//		{
//			cmdFound = 1;
//			tl_cliData[tl_cliIdx-1] = 0;
//			tl_cliIdx = 0;
//			LoopCmds(tl_cliData);
//		}
//		if(tl_cliIdx == CLI_BUFFER_SIZE)
//		{
//			tl_cliData[CLI_BUFFER_SIZE-1] = 0x0;
//			tl_cliIdx = 0;
//			LoopCmds(tl_cliData);
//		}
//	}
//	return cmdFound;
//}

void cli(uint8_t InputKey)
{
	tl_cliData[tl_cliIdx++] = InputKey;

	if( (tl_cliData[tl_cliIdx-1] == '\n') || (tl_cliData[tl_cliIdx-1] == '\r'))
	{
		tl_cliData[tl_cliIdx-1] = 0;
		if( (tl_cliData[tl_cliIdx-2] == '\r') || (tl_cliData[tl_cliIdx-2] == '\n') )
		{
			tl_cliData[tl_cliIdx-2] = 0;
		}

		tl_cliIdx = 0;
		LoopCmds(tl_cliData);
	}

	if(tl_cliIdx == CLI_BUFFER_SIZE)
	{
		tl_cliData[CLI_BUFFER_SIZE-1] = 0x0;
		tl_cliIdx = 0;
		LoopCmds(tl_cliData);
	}

//
//#define MSG_BUFFER_SIZE (64)
//	static uint8_t msg[MSG_BUFFER_SIZE];
//	static int8_t msgIdx;
//
//	msg[msgIdx++] = InputKey;
//	if( parseInputCLI(msg, msgIdx) )
//	{
//		msgIdx = 0;
//	}
}

