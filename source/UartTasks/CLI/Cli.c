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
#include "freeRTOSProject.h"
#include "CMSIS/RTT/SEGGER_RTT.h"

#define CLI_BUFFER_SIZE (256)

static uint8_t tl_cliData[CLI_BUFFER_SIZE];
static uint16_t tl_cliIdx = 0;


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

typedef struct _cliCommandOptions_s
{
	const char *option;
	const uint8_t optionSize;
	int32_t (*optionCallback)(char *);
}s_cliCommandOptions_t;

typedef struct _cliCommands_s
{
	const char *cmdString;
	s_cliCommandOptions_t *option;
}s_cliCommands_t;


static const char *helpCmdText[] = {
		"NGR help commands:\r\n",
		"gpio\r\n",
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

const char *gpioCmdText[] = {
		"gpio help commands:\r\n",
		"-p pin select\r\n",
		"-c current configuration\r\n",
		"-pp pullup power level {0|1}\r\n",
		"-pf passive filter on/off {0|1}\r\n",
		"-af active filter on/off {0|1}\r\n",
		"-clr clear interrupt count\r\n",
		"\0"
};

static int32_t gpioHelpCmd(char *Param)
{
	uint16_t helpIdx=0;

	while(gpioCmdText[helpIdx][0])
	{
		DebugTaskWrite(gpioCmdText[helpIdx],strlen(gpioCmdText[helpIdx]));
		helpIdx++;
	}
	return 0;
}

static int32_t gpioPinConfiguration(char *Param)
{
	size_t strSize = CLI_BUFFER_SIZE;
//	printf("gpio pin configuration \r\n");
	getInputPinConfiguration((char *)tl_cliData,&strSize);

	DebugTaskWrite((char *)tl_cliData,strSize);

	return 0;
}

static int32_t gpioPinSelect(char *Param)
{
	uint8_t *parseStatus;
	uint32_t pinSelect;

	parseStatus = ParseDecimal((uint8_t *)Param, &pinSelect);
	SEGGER_RTT_printf(0,"gpio pin select %d \r\n",pinSelect);
	setInputPin(pinSelect);
	return 0;
}

static int32_t gpioPullupPwrSelect(char *Param)
{
	uint8_t *parseStatus;
	uint32_t pullupPwrSelect;

	parseStatus = ParseDecimal((uint8_t *)Param, &pullupPwrSelect);
	SEGGER_RTT_printf(0,"gpio pin pullup power select %d\r\n",pullupPwrSelect);
	setPullupPowerLevel(pullupPwrSelect);
	return 0;
}

static int32_t gpioPassiveFilterSelect(char *Param)
{
	uint8_t *parseStatus;
	uint32_t filterSelect;

	parseStatus = ParseDecimal((uint8_t *)Param, &filterSelect);
	SEGGER_RTT_printf(0,"gpio pin passive filter select %f\r\n",filterSelect);
	setPassiveFilter(filterSelect);
	return 0;
}

static int32_t gpioActiveFilterSelect(char *Param)
{
	uint8_t *parseStatus;
	uint32_t filterSelect;

	parseStatus = ParseDecimal((uint8_t *)Param, &filterSelect);
	SEGGER_RTT_printf(0,"gpio pin active filter select %d\r\n",filterSelect);
	setActiveFilter(filterSelect);
	return 0;
}
static int32_t gpioClearTestCnt(char *Param)
{
	gpioClearTestCount();
	return 0;
}

s_cliCommandOptions_t gpioOptions[]= {
		{"-pf",3,gpioPassiveFilterSelect},
		{"-af",3,gpioActiveFilterSelect},
		{"-pp",3,gpioPullupPwrSelect},
		{"-clr",4,gpioClearTestCnt},
		{"-c",2,gpioPinConfiguration},
		{"-p",2,gpioPinSelect},

		{"-h",2,gpioHelpCmd},{"-?",2,gpioHelpCmd},{NULL,0,gpioHelpCmd}
};

s_cliCommands_t userCmds[]= {
		{"gpio",gpioOptions},
		{"help" ,helpOptions},
		{NULL,NULL}
};

int32_t LoopOptions(s_cliCommands_t *UserCmdOptions,uint8_t *PtrOption)
{
#define MAX_OPTION_SIZE (5) // max size the option field can be
	char userOption[MAX_OPTION_SIZE],*ptrUserOption;
	int8_t idx;
	int8_t optionSize=0;
	int32_t status = -1;
	int8_t userOptionSize=0;

	ptrUserOption = PtrOption;
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

