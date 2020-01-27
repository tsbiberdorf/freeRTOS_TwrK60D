/*
 * LEDOperations.h
 *
 *  Created on: Dec 13, 2019
 *      Author: TBiberdorf
 */

#ifndef LED_GPIOCOPERATIONS_H_
#define LED_GPIOCOPERATIONS_H_

#define LED_RED_ON (1<<0)
#define LED_RED_OFF (1<<1)
#define GREEN_RED_ON (1<<2)
#define GREEN_RED_OFF (1<<3)
#define BLUE_RED_ON (1<<4)
#define BLUE_RED_OFF (1<<5)

void StartLedTask();
TaskHandle_t GetLedTaskHandle();

#endif // LED_GPIOCOPERATIONS_H_
