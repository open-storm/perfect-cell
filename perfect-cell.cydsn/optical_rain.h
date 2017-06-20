/**
 * @file optical_rain.h
 * @brief Declares functions for hydreon optical rain sensor
 * @author Brandon Wong and Matt Bartos
 * @version TODO
 * @date 2017-06-19
 */

#include <project.h>

/**
 * @brief Starts the optical rain sensor UART
 *
 * @return null
 */
void optical_rain_start();

/**
 * @brief Stops the optical rain sensor UART
 *
 * @return null
 */
void optical_rain_stop();

/**
 * @brief Resets the counter for the optical rain sensor
 *
 * @return null
 */
void optical_rain_reset_count();

/**
 * @brief Gets the current count of the optical rain sensor counter
 *
 * @return @p count of optical rain sensor counter
 */
uint8 optical_rain_get_count();

/**
 * @brief Inserts current values of optical rain sensor counter into labels and readings arrays.
 *
 * @param labels Array to store labels corresponding to each sensor reading
 * @param readings Array to store sensor readings as floating point values
 * @param array_ix Array index to label and readings
 * @param max_size Maximum size of label and reading arrays (number of entries)
 *
 * @return (*array_ix) + number of entries filled
 */
uint8 zip_optical_rain(char *labels[], float readings[], uint8 *array_ix, uint8 max_size);

/* [] END OF FILE */
