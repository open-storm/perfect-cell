/**
 * @file decagon.h
 * @brief Declares functions for Decagon GS3 soil moisture sensor
 * @author Brandon Wong, Matt Bartos, Ivan Mondragon
 * @version TODO
 * @date 2017-06-19
 */
#ifndef DECAGON_H
#define DECAGON_H
#include <project.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "cytypes.h"
#include "sensors_uart_control.h"
#include "strlib.h"

/**
 * @brief Type definition for decagon GS3 sensor struct
 *
 */
struct {
    float dielectric;
    float temp;
    int   conductivity;
    uint8 valid;
	uint8 err;
}typedef DecagonGS3;

/**
 * @brief Takes a reading with the Decagon GS3 sensor
 *
 * @return DecagonGS3 struct containing sensor readings
 */
uint8_t Decagon_Take_Reading(DecagonGS3 *decagon_reading);

/**
 * @brief TODO: Not implemented
 */
DecagonGS3 Decagon_Convert_Raw_Reading(char* raw_D);

/**
 * @brief Inserts current values of @p decagon_reading into labels and readings arrays.
 *
 * @param labels Array to store labels corresponding to each sensor reading
 * @param readings Array to store sensor readings as floating point values
 * @param array_ix Array index to label and readings
 * @param take_average Whether to take average or not
 * - 0: Take first valid reading 
 * - 1: Take average of valid readings
 * @param decagon_loops Number of readings taken on each call.
 * @param max_size Maximum size of label and reading arrays (number of entries)
 *
 * @return (*array_ix) + number of entries filled
 */
uint8 zip_decagon(char *labels[], float readings[], uint8 *array_ix, uint8 take_average, int decagon_loops, uint8 max_size);

#endif
/* [] END OF FILE */
