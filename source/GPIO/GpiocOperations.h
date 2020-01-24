/*
 * GpiocOperations.h
 *
 *  Created on: Dec 13, 2019
 *      Author: TBiberdorf
 */

#ifndef GPIO_GPIOCOPERATIONS_H_
#define GPIO_GPIOCOPERATIONS_H_


void getInputPinConfiguration(char *PtrCurrentConfiguration,size_t *PtrSize);
uint32_t setPassiveFilter(uint32_t PassiveFilter);
uint32_t setPullupPowerLevel(uint32_t PowerLevel);
uint32_t setPassiveFilter(uint32_t PassiveFilter);
void gpioClearTestCount();
void StartGpioTask();

#endif /* GPIO_GPIOCOPERATIONS_H_ */
