/**
 * @file ultrasonic.h
 * @brief Declares functions for maxbotix ultrasonic sensor subroutine
 * @author Brandon Wong and Matt Bartos
 * @version TODO
 * @date 2017-06-19
 */

#include <device.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Type definition for maxbotix depth sensor struct
 *
 */
typedef struct{
    float temp;
    float depth;
    uint8 valid;
} UltrasonicReading;


/**
 * @brief Starts the ultrasonic sensor UART
 *
 * @return null
 */
void                ultrasonic_start();

/**
 * @brief Stops the ultrasonic sensor UART
 *
 * @return null
 */
void                ultrasonic_stop();

/**
 * @brief Powers on the ultrasonic sensor
 *
 * @param which_ultrasonic Which ultrasonic sensor to power on:
 * - 0: First ultrasonic sensor
 * - 1: Second ultrasonic sensor
 * @return 1 on success
 */
uint8               ultrasonic_power_on(uint8 which_ultrasonic);

/**
 * @brief Powers off the ultrasonic sensor
 *
 * @param which_ultrasonic Which ultrasonic sensor to power off:
 * - 0: First ultrasonic sensor
 * - 1: Second ultrasonic sensor
 * @return 1 on success
 */
uint8               ultrasonic_power_off(uint8 which_ultrasonic);

/**
 * @brief Takes a reading with the ultrasonic sensor
 *
 * @return 1 on success, 0 otherwise.
 */
uint8               ultrasonic_get_reading();

/**
 * @brief Inserts current values of @p ultrasonic_reading into labels and readings arrays.
 *
 * @param labels Array to store labels corresponding to each sensor reading
 * @param readings Array to store sensor readings as floating point values
 * @param array_ix Array index to label and readings
 * @param which_ultrasonic Which ultrasonic to take reading with:
 * - 0: First ultrasonic sensor
 * - 1: Second ultrasonic sensor
 * @param take_average Whether to take average or not
 * - 0: Take first valid reading 
 * - 1: Take average of valid readings
 * @param ultrasonic_loops Number of readings taken on each call.
 * @param max_size Maximum size of label and reading arrays (number of entries)
 *
 * @return (*array_ix) + number of entries filled
 */
uint8 zip_ultrasonic(char *labels[], float readings[], uint8 *array_ix, uint8 which_ultrasonic, uint8 take_average, int ultrasonic_loops, uint8 max_size);

/**
 * @brief Resets @p uart_ultrasonic_received_string, clears rx buffer, and 
 * resets @p uart_ultrasonic_string_index
 *
 * @return null
 */
void                uart_ultrasonic_string_reset();


//[] END OF FILE
