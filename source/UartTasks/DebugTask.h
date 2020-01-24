/*
 * debugTask.h
 *
 *  Created on: Dec 11, 2019
 *      Author: TBiberdorf
 */

#ifndef UARTTASKS_DEBUGTASK_H_
#define UARTTASKS_DEBUGTASK_H_

uint32_t DebugTaskWrite( const char *WriteData, size_t Size);
TaskHandle_t GetDebugdTaskHandle();


#endif /* UARTTASKS_DEBUGTASK_H_ */
