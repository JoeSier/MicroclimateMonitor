/**
 * @file mq135.h
 * @author Joseph Sier
 * @brief Header File Containing typedef definition
 *
 * This file defines a typedef struct for managing MQ135 data.
 */


#ifndef MQ135_H_
#define MQ135_H_

/**
 * @brief Struct for storing MQ135 data
 * 
 * This struct contains a single member, `PPM`, which represents 
 * the CO2 in PPM (Parts per million).
 */
 
typedef struct
{
	uint16_t PPM; /**< PPM of CO2 */
}MQ135_DataTypedef;

/**
 * @brief Get MQ135 data
 * 
 * This function retrieves MQ135 data and stores it in the provided 
 * MQ135_DataTypedef structure.
 * 
 * @param MQ135_Data: Pointer to the structure MQ135_DataTypedef.
 */

void MQ_GetData (MQ135_DataTypedef *MQ135_Data);

#endif /* MQ135_H_ */





