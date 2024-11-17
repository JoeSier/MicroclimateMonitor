
/** 
 * @file Moisture.c
 * @brief This file contains functions to read moisture sensor data and calculate moisture percentage.
 * @author Joseph Sier
 *
 * Calibrated for the value in air (0%), and the value in water (100%). The ADC reading is then converted to a percentage within this.
 *
 * I used a Capacitive sensor instead of resistive as the resistive can corrode.
 */


#include "main.h"
#include "Moisture.h"

/** 
 * @brief Value when the sensor is exposed to air.
 */
#define AIR_VALUE 698

/** 
 * @brief Value when the sensor is submerged in water.
 */
#define WATER_VALUE 285

 
/** 
 * @brief Maps a value from one range to another.
 * @param x The value to map.
 * @param in_min The minimum value of the input range.
 * @param in_max The maximum value of the input range.
 * @param out_min The minimum value of the output range.
 * @param out_max The maximum value of the output range.
 * @return The mapped value.
 */
int map(int x, int in_min, int in_max, int out_min, int out_max)
{
	return (x - in_min) *(out_max - out_min) / (in_max - in_min) + out_min;
}

/** 
 * @brief Reads the analog value from the moisture sensor.
 * @note 	The sensor is connected to channel 7 (A2) of ADC3.
 * @return The analog value.
 */


uint16_t readMoisture()
{
	ADC_ChannelConfTypeDef sConfig1;
	sConfig1.Rank = 1;
	sConfig1.Channel = ADC_CHANNEL_7;
	sConfig1.SamplingTime = ADC_SAMPLETIME_28CYCLES;
	HAL_ADC_ConfigChannel(&hadc1, &sConfig1);
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
	return HAL_ADC_GetValue(&hadc1);
}

/** 
 * @brief Calculates the moisture percentage based on the analog value.
 * @return The moisture percentage.
 */

int readMoisturePercent(void){
	uint16_t soilMoistureValue = readMoisture();
	int8_t soilMoisturePercent = map(soilMoistureValue, AIR_VALUE, WATER_VALUE, 0, 100);
	return soilMoisturePercent;	
}


/**
 * @brief Get moisture data
 * 
 * This function retrieves moisture data and stores it in the provided 
 * Moisture_DataTypedef structure.
 * 
 * @param Moisture_Data: Pointer to the structure Moisture_DataTypedef.
 */
void Moisture_GetData (Moisture_DataTypedef *Moisture_Data){
	Moisture_Data->MoisturePercent = readMoisturePercent();
}
