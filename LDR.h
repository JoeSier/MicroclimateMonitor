/**
 * @file LDR.h
 * @author Joseph Sier
 * @brief Header File Containing typedef definition
 *
 * This file defines a typedef struct for managing LDR data.
 */


#ifndef LDR_H_
#define LDR_H_

/**
 * @brief Struct for storing LDR data
 * 
 * This struct contains a single member, `LDR_Val`, which represents 
 * the analog reading from LDR.
 */
typedef struct
{
	int LDR_Val;  /**< Analog LDR value */
}LDR_DataTypedef;

/**
 * @brief Get LDR data
 * 
 * This function retrieves moisture data and stores it in the provided 
 * LDR_DataTypedef structure.
 * 
 * @param LDR_Data: Pointer to the structure LDR_DataTypedef.
 */
void LDR_GetData (LDR_DataTypedef *LDR_Data);

#endif /* LDT_H_ */





