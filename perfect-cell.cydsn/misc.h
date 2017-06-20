/**
 * @file misc.h
 * @brief Declares miscellaneous utilites
 * @author Matt Bartos and Brandon Wong
 * @version TODO
 * @date 2017-06-19
 */
#include <device.h>

/**
 * @brief Reads the current battery voltage
 *
 * @return Battery voltage as a 32-bit float
 */
float32 read_vbat();

/**
 * @brief Inserts current value of @p v_bat into readings array
 *
 * @param labels Array to store labels corresponding to each trigger result
 * @param readings Array to store trigger results as floating point values
 * @param array_ix Array index to label and readings
 * @param max_size Maximum size of label and reading arrays (number of entries)
 *
 * @return (*array_ix) + number of entries filled
 */
uint8 zip_vbat(char *labels[], float readings[], uint8 *array_ix, uint8 max_size);

/**
 * @brief Blinks the onboard LED.
 *
 * @return 1u on success
 */
uint8   blink_LED(uint8 n_blinks);

/**
 * @brief Initializes pins by writing them low
 *
 * @return null
 */
void    init_pins();

/* [] END OF FILE */
