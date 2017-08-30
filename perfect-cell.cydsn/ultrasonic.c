/**
 * @file ultrasonic.c
 * @brief Implements functions for maxbotix ultrasonic sensors and senix
 * toughsonic sensors subroutines
 * @author Brandon Wong, Matt Bartos, Ivan Mondragon, Alec Beljanski
 * @version TODO
 * @date 2017-06-19
 */

#include "ultrasonic.h"

// Provide power to the ultrasonic sensor
uint8 ultrasonic_power_on(uint8 which_ultrasonic) {
    switch (which_ultrasonic) {
        case 0u:
            Ultrasonic_ON_Write(1u);
            break;
        case 1u:
            Ultrasonic_2_ON_Write(1u);
            break;
        case 2u:
            toughsonic_power_Write(1u);
    }
    return 1u;
}

// Cut power to the ultrasonic sensor
uint8 ultrasonic_power_off(uint8 which_ultrasonic) {
    switch (which_ultrasonic) {
        case 0u:
            Ultrasonic_ON_Write(0u);
            break;
        case 1u:
            Ultrasonic_2_ON_Write(0u);
            break;
        case 2u:
            toughsonic_power_Write(0u);
    }
    return 0u;
}

/**
 * @brief Parse raw UART received string from any senix sensor into @p
 * reading.
 *
 * @param reading Structure to store results into. Depth in millimeters.
 * @param str Raw UART received string.
 */
static void parse_senix_string(UltrasonicReading *reading, const char *str) {
    char *cr = strchr(str, '\r');  // find the first carriage return
    const unsigned int depth = strtoul(cr - 5, &cr, 10);  // get counts
    const float counts_to_mm = 0.003384f * 4u * 25.4f;
    reading->depth = depth * counts_to_mm;  // convert to millimeters
    reading->valid = !!depth;
}

/**
 * @brief Parse raw UART received string from any maxbotix sensor into @p
 * reading.
 *
 * @param reading Structure to store results into. Depth in millimeters.
 * @param str Raw UART received string.
 */
static void parse_maxbotix_string(UltrasonicReading *reading, const char *str) {
    // Expected model name in UART "PN:MB7384\r", or corresponding sensor
    // long range : 7383
    // short range: 7384

    // Expect the UART to contain something like "Sonar..copyright..
    // \rTempI\rR1478\rR1477\r..."
    char depth_str[5] = {'\0'};
    if (strextract(str, depth_str, "TempI\rR", "\r")) {
        float depth = strtof(depth_str, NULL);
        int valid = 0;

        char model[5] = {'\0'};
        strextract(str, model, "PN:MB", "\r");

        if (strcmp(model, "7384") == 0) {  // Short range sensor
            valid = strcmp(depth_str, "5000");
        } else if (strcmp(model, "7383") == 0) {  // Long range sensor
            valid = strcmp(depth_str, "9999");
        }

        reading->valid = !!valid;  // valid could be any non-zero number
        reading->depth = valid ? depth : -depth;
    } else {
        reading->valid = 0u;
        reading->depth = -9999.0f;
    }
}

// Start ISR, Fill array, Return array, Stop ISR
uint8 ultrasonic_get_reading(UltrasonicReading *reading,
                             uint8_t which_ultrasonic) {
    sensors_uart_clear_string();
    sensors_uart_set_baud(9600u);
    sensors_uart_start();

    if (which_ultrasonic == 2u) {
    Senix_Comp_Start();
    }
    ultrasonic_power_on(which_ultrasonic);  // Power on the sensor
    CyDelay(800u);  // Wait for UART to get readings from sensor
    ultrasonic_power_off(which_ultrasonic);  // Power off the sensor

    sensors_uart_stop();
    if (which_ultrasonic == 2u) {
    Senix_Comp_Stop();
    }
    char *uart_string = sensors_uart_get_string();

    if (which_ultrasonic == 2u) {
        parse_senix_string(reading, uart_string);
    } else {
        parse_maxbotix_string(reading, uart_string);
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

    float measurement = 0.0f;
    uint8_t valid_count = 0u, read_iter = 0u;
    UltrasonicReading ultrasonic_reading = {0u, 0u};

    char *ultrasonics[] = {"maxbotix_depth", "maxbotix_2_depth", "senix_depth"};
    labels[*array_ix] = ultrasonics[which_ultrasonic];

    // Start the MUX
    mux_controller_Wakeup();

    // Set MUX to read from 1st input
    mux_controller_Write(which_ultrasonic);

    for (read_iter = 0; read_iter < ultrasonic_loops; read_iter++) {
        ultrasonic_get_reading(&ultrasonic_reading, which_ultrasonic);
        if (ultrasonic_reading.valid == 1u) {
            valid_count++;
            measurement += ultrasonic_reading.depth;
            // If not taking the average, break the loop at the first valid
            // reading
            if (take_average == 0u) {
                break;
            }
        }
    }

    // If taking the average, divide by the number of valid readings
    if (take_average && valid_count > 0) {
        measurement /= valid_count;
    }
    /* 2017 02 05: Send -1 instead of 9999 to avoid confusion
     * TODO: Test and then check in this update on GitHub
     * If there are no valid readings, send 9999 */
    else if (valid_count == 0u) {
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
