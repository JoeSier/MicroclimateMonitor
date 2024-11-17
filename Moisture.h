/**
 * @file Moisture.h
 * @author Joseph Sier
 * @brief Header File Containing typedef definition
 *
 * This file defines a typedef struct for managing moisture data.
 */



#ifndef Moisture_H_
#define Moisture_H_

/**
 * @brief Struct for storing moisture data
 * 
 * This struct contains a single member, `MoisturePercent`, which represents 
 * the percentage of moisture.
 */
typedef struct
{
    int8_t MoisturePercent; /**< Percentage of moisture */
} Moisture_DataTypedef;

/**
 * @brief Get moisture data
 * 
 * This function retrieves moisture data and stores it in the provided 
 * Moisture_DataTypedef structure.
 * 
 * @param Moisture_Data: Pointer to the structure Moisture_DataTypedef.
 */
void Moisture_GetData(Moisture_DataTypedef *Moisture_Data);

#endif /* Moisture_H_ */




