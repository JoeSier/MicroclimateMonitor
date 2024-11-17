
/**
  * @file    LDR.c
	*	@author Joseph Sier
  * @brief   This file contains functions to read data from the Light Dependent Resistor (LDR).
	*
  */

#include "main.h"
#include "LDR.h"


 
/**
  * @brief  Reads the analog value from the LDR.
	* @note 	The sensor is connected to channel 6 (A3) of ADC3.
  * @return The value read from the LDR.
  */

uint32_t readLDR() {
	ADC_ChannelConfTypeDef sConfig3;
	sConfig3.Rank = 1;
	sConfig3.Channel = ADC_CHANNEL_6;
	sConfig3.SamplingTime = ADC_SAMPLETIME_28CYCLES;
	HAL_ADC_ConfigChannel( & hadc3, & sConfig3);
	HAL_ADC_Start( & hadc3);
	HAL_ADC_PollForConversion( & hadc3, HAL_MAX_DELAY);
	return HAL_ADC_GetValue( & hadc3);
}




/**
 * @brief Get LDR data
 * 
 * This function retrieves moisture data and stores it in the provided 
 * LDR_DataTypedef structure.
 * 
 * @param LDR_Data: Pointer to the structure LDR_DataTypedef.
 */

void LDR_GetData (LDR_DataTypedef *LDR_Data){
	LDR_Data->LDR_Val=readLDR();
}

