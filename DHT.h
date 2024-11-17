/**
 * @file DHT.h
 * @author Joseph Sier
 * @brief Header File Containing typedef definition
 *
 * This file defines a typedef struct for managing moisture data.
 */


#ifndef DHT_H_
#define DHT_H_

/**
 * @brief Struct for storing DHT data
 * 
 * This struct contains two members, `Temperature` and `Humidity`, which represents 
 * the temperature and humidity from the sensor.
 */
typedef struct
{
	uint8_t Temperature; /**< Temperature */
	uint8_t Humidity; /**< Humidity */
}DHT_DataTypedef;

/**
 * @brief Get DHT data
 * 
 * This function retrieves DHT data and stores it in the provided 
 * DHT_DataTypedef structure.
 * 
 * @param DHT_Data: Pointer to the structure DHT_DataTypedef.
 */
void DHT_GetData (DHT_DataTypedef *DHT_Data);

#endif /* DHT_H_ */







