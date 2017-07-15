/**
 * @file decagon.c
 * @brief Implements functions for Decagon GS3 soil moisture sensor
 * @author Brandon Wong, Matt Bartos, Ivan Mondragon
 * @version TODO
 * @date 2017-06-19
 */

#include "decagon.h"
#include "sensors_uart_control.h"
#include "strlib.h"

uint8_t Decagon_Take_Reading(DecagonGS3* decagon_reading) {
    char *dielectric, *temp, *conductivity;
    uint8_t err = 0u;

    sensors_uart_clear_string();

    // Divide clock to get baud rate of 1200
    // Ideally we want every function that needs to use the UART
    // to state the baud rate that they want to use beforehand
    sensors_uart_set_baud(1200u);

    sensors_uart_start();

    // Power cycle sensor to gather readings
    Decagon_Power_Write(1u);
    CyDelay(1000u);
    Decagon_Power_Write(0u);

    sensors_uart_stop();

    // Restore old baud rate, read above for ideal world
    sensors_uart_set_baud(9600u);

    char *raw_serial_data_d = sensors_uart_get_string();

    // Convert the raw data
    if ((dielectric = strtok(raw_serial_data_d, " ")) == NULL) {
        err += 1u;
    }
    if ((temp = strtok(NULL, " ")) == NULL) {
        err += 2u;
    }
    if ((conductivity = strtok(NULL, " ")) == NULL) {
        err += 4u;
    }

    decagon_reading->dielectric = atof(dielectric);
    decagon_reading->temp = atof(temp);
    decagon_reading->conductivity = atoi(conductivity);
    decagon_reading->valid = (err == 0u);
    decagon_reading->err = err;

    return err;
}

DecagonGS3 Decagon_Convert_Raw_Reading(char* raw_D) {
    DecagonGS3 final_reading;
    final_reading.valid = 0u;

    final_reading.valid =
        sscanf(raw_D, "\x09%f %f %d\r%*s", &final_reading.dielectric,
               &final_reading.temp, &final_reading.conductivity);

    // if you know the the calibration equation, you can caalcualte the soil
    // moisutre here, too
    if (final_reading.valid != 4) {
        final_reading.valid = 0;
    }

    return final_reading;
}

uint8 zip_decagon(char* labels[], float readings[], uint8* array_ix,
                  uint8 take_average, int decagon_loops, uint8 max_size) {
    // Ensure we don't access nonexistent array index
    uint8 nvars = 3;
    if (*array_ix + nvars >= max_size) {
        return *array_ix;
    }

    int valid_count = 0u, read_iter = 0u;
    float conductivity = 0.0f, temp = 0.0f dielectric = 0.0f;

    DecagonGS3 decagon_reading = {0u, 0u, 0u, 0u, 0u};

    // with the begin/end paradigm, end must always be `one past the end`
    char** begins = labels + *array_ix;
    char** ends = begins + nvars;
    zips(begins, ends, "decagon_soil_conduct", "decagon_soil_temp",
         "decagon_soil_dielec");

    // Set the mux to read from the decagon pin
    mux_controller_Wakeup();
    mux_controller_Write(3u);

    for (read_iter = 0; read_iter < decagon_loops; read_iter++) {
        Decagon_Take_Reading(&decagon_reading);
        if (decagon_reading.valid == 1u) {
            valid_count++;
            conductivity += decagon_reading.conductivity;
            temp += decagon_reading.temp;
            dielectric += decagon_reading.dielectric;
            if (take_average == 0u) {
                break;
            }
        }
    }
    if (take_average == 1u && valid_count > 0) {
        conductivity /= valid_count;
        temp /= valid_count;
        dielectric /= valid_count;
    } else if (valid_count == 0u) {
        conductivity = 9999;
        temp = 9999;
        dielectric = 9999;
    }

    // Save mux configuration and put mux to sleep
    mux_controller_Sleep();

    // with the begin/end paradigm, end must always be `one past the end`
    float* beginf = readings + *array_ix;
    float* endf = beginf + nvars;
    zipf(beginf, endf, conductivity, temp, dielectric);

    *array_ix += 3;
    return *array_ix;
}
