/**
 * @file ultrasonic.h
 * @brief Implements functions for maxbotix ultrasonic sensors and senix
 * toughsonic sensors subroutines
 * @author Brandon Wong, Matt Bartos, Ivan Mondragon, Alec Beljanski
 * @version TODO
 * @date 2017-06-19
 */
#ifndef ULTRASONIC_H
#define ULTRASONIC_H
#include <device.h>
#include <stdlib.h>
#include <string.h>
#include "sensors_uart_control.h"
#include "strlib.h"

/**
 * @brief Type definition for maxbotix depth sensor struct
 *
 */
typedef struct {
    float depth;
    uint8 valid;
} UltrasonicReading;

/**
 * @brief Powers on the ultrasonic sensor
 *
 * @param which_ultrasonic Which ultrasonic sensor to power on:
 * - 0: First ultrasonic sensor
 * - 1: Second ultrasonic sensor
 * - 2: Senix toughsonic sensor
 * @return 1 on success
 */
uint8 ultrasonic_power_on(uint8 which_ultrasonic);

/**
 * @brief Powers off the ultrasonic sensor
 *
 * @param which_ultrasonic Which ultrasonic sensor to power off:
 * - 0: First ultrasonic sensor
 * - 1: Second ultrasonic sensor
 * - 2: Senix toughsonic sensor
 * @return 0 on success
 */
uint8 ultrasonic_power_off(uint8 which_ultrasonic);

/**
 * @brief Takes a reading with the ultrasonic sensor selected by @p
 * which_ultrasonic.
 *
 * @param reading Structure to store results into. Depth in millimeters.
 * @param which_ultrasonic Which ultrasonic sensor to use to take reading:
 * - 0: First ultrasonic sensor
 * - 1: Second ultrasonic sensor
 * - 2: Senix toughsonic sensor
 * @return 1 on success, 0 otherwise.
 */
uint8 ultrasonic_get_reading(UltrasonicReading *reading,
                             uint8_t which_ultrasonic);
/**
 * @brief Inserts current values of @p ultrasonic_reading into labels and
 * readings arrays.
 *
 * @param labels Array to store labels corresponding to each sensor reading
 * @param readings Array to store sensor readings as floating point values
 * @param array_ix Array index to label and readings
 * @param which_ultrasonic Which ultrasonic to take reading with:
 * - 0: First ultrasonic sensor
 * - 1: Second ultrasonic sensor
 * - 2: Senix toughsonic sensor
 * @param take_average Whether to take average or not
 * - 0: Take first valid reading
 * - 1: Take average of valid readings
 * @param ultrasonic_loops Number of readings taken on each call.
 * @param max_size Maximum size of label and reading arrays (number of
 * entries)
 *
 * @return (*array_ix) + number of entries filled
 */
uint8 zip_ultrasonic(char *labels[], float readings[], uint8 *array_ix,
                     uint8 which_ultrasonic, uint8 take_average,
                     int ultrasonic_loops, uint8 max_size);

#endif
//[] END OF FILE
