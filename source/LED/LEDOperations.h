/*
 * LEDOperations.h
 *
 *  Created on: Dec 13, 2019
 *      Author: TBiberdorf
 */

#ifndef LED_GPIOCOPERATIONS_H_
#define LED_GPIOCOPERATIONS_H_

#define LED1 (1<<13)
#define LED2 (1<<14)
#define LED3 (1<<15)

void StartLedTask();
TaskHandle_t GetLedTaskHandle();

#endif // LED_GPIOCOPERATIONS_H_
