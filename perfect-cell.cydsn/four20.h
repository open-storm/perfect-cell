/*
 * @file four20.h
 * @brief Take an analog reading from a current loop (4-20 mA) sensor
 * @author Brandon Wong
 * @version TODO
 * @date 2018-04-03
 */

#ifndef FOUR20_H
#define FOUR20_H
    
#include <project.h>

/**
 * @brief Take a an analog reading from a 4-20 mA sensor across a 150 ohm resistor
 * 
 * @param v_420mA float32 of the voltage across 150 ohm resistor
 *
 */    
uint8 read_420mA(float32 *v_420mA);

/**
 * @brief Inserts current value of 4-20 mA sensor into labels and
 * readings arrays.
 *
 * @param labels Array to store labels corresponding to each sensor reading
 * @param readings Array to store sensor readings as floating point values
 * @param array_ix Array index to label and readings
 * @param max_size Maximum size of label and reading arrays (number of
 * entries)
 *
 * @return (*array_ix) + number of entries filled
 */
uint8 zip_420mA(char *labels[], float readings[], uint8 *array_ix, uint8 max_size);

#endif
/* [] END OF FILE */
