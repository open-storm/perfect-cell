/**
 * @file ultrasonic.c
 * @brief Implements functions for maxbotix ultrasonic sensor subroutine
 * @author Brandon Wong and Matt Bartos
 * @version TODO
 * @date 2017-06-19
 */

#include "ultrasonic.h"

#define DEPTH_STRING_LENGTH     4

// Provide power to the ultrasonic sensor
uint8 ultrasonic_power_on(uint8 which_ultrasonic){
    if (which_ultrasonic == 0u){
        Ultrasonic_ON_Write(1u);
    }
    else if (which_ultrasonic == 1u){
        Ultrasonic_2_ON_Write(1u);
    }
    return 1u;
}

// Cut power to the ultrasonic sensor
uint8 ultrasonic_power_off(uint8 which_ultrasonic){
    if (which_ultrasonic == 0u){
        Ultrasonic_ON_Write(0u);
    }
    else if (which_ultrasonic == 1u){
        Ultrasonic_2_ON_Write(0u);
    }
    return 0u;
}

// Start ISR, Fill array, Return array, Stop ISR
uint8 ultrasonic_get_reading(UltrasonicReading *reading) {
    char depth_str[DEPTH_STRING_LENGTH + 1] = {'\0'};
    uint8 which_ultrasonic = mux_controller_Read();

    sensors_uart_clear_string();
    sensors_uart_set_baud(9600u);
    sensors_uart_start();

    ultrasonic_power_on(which_ultrasonic);  // Power on the sensor
    CyDelay(750u);  // Wait for UART to get readings from sensor
    ultrasonic_power_off(which_ultrasonic);  // Power off the sensor

    sensors_uart_stop();
    char *uart_string = sensors_uart_get_string();

    // Expected name in UART "PN:MB7384\r", or corresponding sensor
    // long range : 7383
    // short range: 7384

    // Expect the UART to contain something like "Sonar..copyright..
    // \rTempI\rR1478\rR1477\r..."
    if (strextract(uart_string, depth_str, "TempI\rR", "\r")) {
        float depth = strtof(depth_str, NULL);
        int valid = 0;

        char name[5] = {'\0'};
        strextract(uart_string, name, "PN:MB", "\r");

        if (strcmp(name, "7383") == 0) {  // Short range sensor
            valid = strcmp(depth_str, "5000");
        } else if (strcmp(name, "7384") == 0) {  // Long range sensor
            valid = strcmp(depth_str, "9999");
        }

        reading->depth = valid ? depth : -depth;
        reading->valid = !!valid;  // valid could be any non-zero number
    } else {
        reading->valid = 0u;
        reading->depth = -9999.0f;
    }

    return reading->valid;
}

uint8 zip_ultrasonic(char *labels[], float readings[], uint8 *array_ix,
                     uint8 which_ultrasonic, uint8 take_average,
                     int ultrasonic_loops, uint8 max_size) {
    // Ensure we don't access nonexistent array index
    uint8 nvars = 1;
    if (*array_ix + nvars >= max_size) {
        return *array_ix;
    }

    float valid_iter = 0.0;
    int read_iter = 0;
    UltrasonicReading ultrasonic_reading = {0u, 0u};
    float measurement = 0.0f;

    char *ultrasonics[] = {"maxbotix_depth", "maxbotix_2_depth"};
    labels[*array_ix] = ultrasonics[which_ultrasonic];

    // Start the MUX
    mux_controller_Wakeup();

    // Set MUX to read from 1st input
    mux_controller_Write(which_ultrasonic);

    for (read_iter = 0; read_iter < ultrasonic_loops; read_iter++) {
        ultrasonic_get_reading(&ultrasonic_reading);
        if (ultrasonic_reading.valid == 1u) {
            valid_iter++;
            measurement += ultrasonic_reading.depth;
            // If not taking the average, break the loop at the first valid
            // reading
            if (take_average == 0u) {
                break;
            }
        }
    }

    // If taking the average, divide by the number of valid readings
    if (take_average && valid_iter > 0) {
        measurement = measurement / valid_iter;
    }
    /* 2017 02 05: Send -1 instead of 9999 to avoid confusion
     * TODO: Test and then check in this update on GitHub
     * If there are no valid readings, send 9999 */
    else if (valid_iter == 0.0) {
        measurement = -1;
    }

    // Save MUX configuration + put MUX to sleep
    mux_controller_Sleep();

    // Store measurement + increment array_ix
    readings[*array_ix] = measurement;
    *array_ix += 1;
    return *array_ix;
}

/* [] END OF FILE */
