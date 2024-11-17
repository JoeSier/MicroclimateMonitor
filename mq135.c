/**
  ******************************************************************************
  * @file mq135.c
	*	@author Joseph Sier
  * @brief   This file contains functions to read data from the MQ135 sensor and calculate the PPM (Parts Per Million) of CO2.
  *          
	*	The implementation is based on the MQ135 Arduino library.
	*
	* The MQ135 sensor is a multi purpose sensor, but i have it configured for CO2, using 21K total resistance (_Rload).
	* It requires a preheat before use, which is why there is a warmup page at the beginning. It should be around 20 seconds to 2 minutes,
  * but decreased for useability.
  * The analog output increases when the tin oxide (SnO2) pad inside the MQ135 comes into contact with CO2, which increases its conductivity.
  * This gives a value proportional to the gas concentration, which is converted to volts.
  * It is calibrated using the resistance at atmospheric co2 level, but is not adjusted for temperature and humidity.
	*
	*
	*
  ******************************************************************************
  */



#include "main.h"
#include "mq135.h"


/** @brief Load on board in kOhm **/
#define _rload 21.0f
/** @brief resistance at atmospheric CO2 level **/
#define _rzero  1766.01f
/** @brief Param for getting CO2 PPM from sensor resistance **/
#define PARA  116.6020682
/** @brief Param for getting CO2 PPM from sensor resistance **/
#define PARB 2.769034857
 
 /**
  * @brief   Reads the analog value from the MQ135 sensor.
  * @note    The sensor is connected to channel 8 (A1) of ADC3.
  * @return  The analog value read from the sensor.
  */
 
uint32_t readMQ135() {
	// channel 8 is A1, ADC3_IN8, gas
	ADC_ChannelConfTypeDef sConfig;
	sConfig.Rank = 1;
	sConfig.Channel = ADC_CHANNEL_8;
	sConfig.SamplingTime = ADC_SAMPLETIME_84CYCLES;
	HAL_ADC_ConfigChannel( & hadc2, & sConfig);
	HAL_ADC_Start( & hadc2);
	HAL_ADC_PollForConversion( & hadc2, HAL_MAX_DELAY);
	return HAL_ADC_GetValue( & hadc2);
}

/**
  * @brief   Custom power function.
  * @param   base: Base value.
  * @param   exponent: Exponent value.
  * @return  Result of base raised to the power of exponent.
  */

double my_pow(double base, int exponent) {
	double result = 1.0;

	if (exponent == 0)
		return 1.0;
	else if (exponent > 0) {
		int i;
		for (i = 0; i < exponent; ++i) {
			result *= base;
		}
	} else {
		int i;
		for (i = 0; i < -exponent; ++i) {
			result /= base;
		}
	}

	return result;
}

/**
  * @brief   Calculates the resistance of the MQ135 sensor from the analog reading.
  * @return  Resistance of the MQ135 sensor in volts.
  */

float getMQ135Resistance() {
	int val = readMQ135();
	return ((1023.0f / (float) val) - 1.0f) * _rload;
}

/**
  * @brief   Calculates the PPM of CO2 using calibrated values.
  * @return  PPM of CO2.
  */
float getMQ135PPM() {
	return (PARA * my_pow((getMQ135Resistance() / _rzero), -PARB));
}

/**
 * @brief Get MQ135 data
 * 
 * This function retrieves MQ135 data and stores it in the provided 
 * MQ135_DataTypedef structure.
 * 
 * @param MQ135_Data: Pointer to the structure MQ135_DataTypedef.
 */

void MQ_GetData (MQ135_DataTypedef *MQ135_Data)
{
			MQ135_Data->PPM = getMQ135PPM();
	
}

