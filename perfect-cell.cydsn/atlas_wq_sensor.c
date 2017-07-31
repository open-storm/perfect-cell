/**
 * @file atlas_wq_sensor.c
 * @brief Implements functions for Atlas water quality sensors
 * @author Abhiramm Mullapudi and Matt Bartos
 * @version TODO
 * @date 2017-06-19
 */
#include "atlas_wq_sensor.h"
const uint8_t ATLAS_MAX_ITER = 100u;
const uint8_t CONDUCTIVITY = 100u;
const uint8_t TEMPERATURE = 102u;
const uint8_t DO = 97u;
const uint8_t ORP = 98u;
const uint8_t PH = 99u;

int atlas_sensor_sleep(uint8_t sensor_address) {
    uint8 command[] = "Sleep";
    uint8 command_buffer_size = 7;
    uint8 status;
    int write_iter;

    for (write_iter = 0; write_iter < ATLAS_MAX_ITER; write_iter++) {
        status =
            (I2C_MasterWriteBuf(sensor_address, command, command_buffer_size,
                                I2C_MODE_COMPLETE_XFER) &
             I2C_MSTAT_WR_CMPLT);
        if (!status) {
            break;
        }
    }
    return 1;
}

int atlas_sensor_calibrate(uint8_t sensor_address) {
    // This can probably be combined with the previous function into a single
    // "send command" function
    uint8 command[] = "Cal,1";
    uint8 command_buffer_size = 7;
    uint8 status;
    int write_iter;

    for (write_iter = 0; write_iter < ATLAS_MAX_ITER; write_iter++) {
        status =
            (I2C_MasterWriteBuf(sensor_address, command, command_buffer_size,
                                I2C_MODE_COMPLETE_XFER) &
             I2C_MSTAT_WR_CMPLT);
        if (!status) {
            break;
        }
    }
    return 1;
}

inline static void parse_conductivity_string(con_reading_t *reading,
                                             char *str) {
    char *ec = strtok(str, ",");
    char *tds = strtok(NULL, ",");
    char *sal = strtok(NULL, ",");
    char *sg = strtok(NULL, ",");

    reading->ec = atof(ec);
    reading->sal = atof(sal);
    reading->tds = atof(tds);
    reading->sg = atof(sg);
}

uint8_t atlas_take_single_reading(uint8_t sensor_address, reading_u reading) {
    uint8 command[] = "R";
    uint8 command_buffer_size = sensor_address == CONDUCTIVITY ? 19u : 7u;
    uint8 raw_reading[20] = {0};
    char *reading_start;

    // Clear any existing buffers
    I2C_MasterClearStatus();
    I2C_MasterClearWriteBuf();
    I2C_MasterClearReadBuf();

    // Send I2C command
    I2C_MasterWriteBuf(sensor_address, command, command_buffer_size,
                       I2C_MODE_COMPLETE_XFER);
    // Wait for complete transfer
    while (!(I2C_MasterStatus() & I2C_MSTAT_WR_CMPLT));

    // Wait for sensor to take readings
    CyDelay(1000u);

    // Send I2C read command
    I2C_MasterReadBuf(sensor_address, raw_reading, command_buffer_size,
                      I2C_MODE_COMPLETE_XFER);
    // Wait until read is complete
    while (!(I2C_MasterStatus() & I2C_MSTAT_RD_CMPLT));

    // Parse the buffer if it is valid
    reading_start = strchr((const char *) raw_reading, 1u);
    if (reading_start == NULL) {
        return 0u;
    }

    if (sensor_address == CONDUCTIVITY) {
        parse_conductivity_string(reading.co_reading,
                                  (char *) reading_start + 1u);
    } else {
        *reading.generic_reading = strtof(reading_start + 1, NULL);
    }
    return 1u;
}

uint8_t zip_atlas_wq(char *labels[], float readings[], uint8_t *array_ix,
                     uint8_t max_size) {
    // Ensure we don't access nonexistent array index
    uint8 nvars = 8;
    if (*array_ix + nvars >= max_size) {
        return *array_ix;
    }

    con_reading_t atlas_conductivity = {-9999, -9999, -9999, -9999};
    float atlas_water_temp = -9999;
    float atlas_do = -9999;
    float atlas_orp = -9999;
    float atlas_ph = -9999;

    WQ_Power_Write(1u);
    WQ_Power_1_Write(1u);
    WQ_Power_2_Write(1u);

    CyDelay(1000);
    I2C_Wakeup();
    I2C_Start();
    CyDelay(500);

    // Execute readings
    reading_u rd = &atlas_water_temp;
    atlas_take_single_reading(TEMPERATURE, rd);

    rd = &atlas_do;
    atlas_take_single_reading(DO, rd);

    rd = &atlas_orp;
    atlas_take_single_reading(ORP, rd);

    rd = &atlas_ph;
    atlas_take_single_reading(PH, rd);

    rd = &atlas_conductivity;
    atlas_take_single_reading(CONDUCTIVITY, rd);

    // Fill labels
    char **begins = labels + *array_ix;
    char **ends = begins + nvars;
    zips(begins, ends, "atlas_water_temp", "atlas_dissolved_oxygen",
         "atlas_orp", "atlas_ph", "atlas_ec", "atlas_tds", "atlas_sal",
         "atlas_sg");

    // Fill reading array
    float *beginf = readings + *array_ix;
    float *endf = beginf + nvars;
    zipf(beginf, endf, atlas_water_temp, atlas_do, atlas_orp, atlas_ph,
         atlas_conductivity.ec, atlas_conductivity.tds, atlas_conductivity.sal,
         atlas_conductivity.sg);

    I2C_Sleep();
    CyDelay(100);

    WQ_Power_Write(0u);
    WQ_Power_1_Write(0u);
    WQ_Power_2_Write(0u);
    *array_ix += 8;
    return *array_ix;
}

/* [] END OF FILE */
