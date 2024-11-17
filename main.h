/**
 * @file main.h
 * @author Joseph Sier
 * @brief File Containing definitions for hadc, buffer and userTrigger
 *
 * Defines string buffer and the usertrigger toggled by interrupt,
 *  as well as the ADC_HandleTypeDef for the sensors.
 */


#ifndef MAIN_H
#define MAIN_H

#include <stdbool.h>

 extern char buffer[128]; 
 extern bool userTrigger;

#endif 

#ifndef ADC_H_
#define ADC_H_

#include "stm32f7xx_hal.h"

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern ADC_HandleTypeDef hadc3;

#endif  


