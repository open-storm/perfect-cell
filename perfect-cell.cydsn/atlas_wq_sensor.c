/**
 * @file atlas_wq_sensor.c
 * @brief Implements functions for Atlas water quality sensors
 * @author Abhiramm Mullapudi and Matt Bartos
 * @version TODO
 * @date 2017-06-19
 */
#include "atlas_wq_sensor.h"

// I2C Sensors' default addresses
const uint8_t CONDUCTIVITY = 100u;
const uint8_t TEMPERATURE = 102u;
const uint8_t DO = 97u;
const uint8_t ORP = 98u;
const uint8_t PH = 99u;

static uint8_t i2c_write(const uint8_t addr, uint8_t msg[],
                         const uint8_t msg_sz) {
    // Clear any existing buffers
    I2C_MasterClearStatus();
    I2C_MasterClearReadBuf();

    // Send I2C command
    I2C_MasterWriteBuf(addr, msg, msg_sz, I2C_MODE_COMPLETE_XFER);

    // Wait for complete transfer
    while (!(I2C_MasterStatus() & I2C_MSTAT_WR_CMPLT));
    return !(I2C_MasterStatus() & I2C_MSTAT_ERR_XFER); // return 0u on error
}

static uint8_t i2c_read(const uint8_t addr, uint8_t buf[],
                        const uint8_t buf_sz) {
    // Clear any existing buffers
    I2C_MasterClearStatus();
    I2C_MasterClearReadBuf();

    // Send I2C read command
    I2C_MasterReadBuf(addr, buf, buf_sz, I2C_MODE_COMPLETE_XFER);

    // Wait until read is complete
    while (!(I2C_MasterStatus() & I2C_MSTAT_RD_CMPLT));
    return !(I2C_MasterStatus() & I2C_MSTAT_ERR_XFER); // return 0u on error
}

uint8_t atlas_sensor_sleep(uint8_t sensor_address) {
    uint8_t command[] = "Sleep";
    return i2c_write(sensor_address, command, sizeof(command));
}

uint8_t atlas_sensor_calibrate(uint8_t sensor_address) {
    uint8_t command[] = "Cal,1";
    return i2c_write(sensor_address, command, sizeof(command));
}

// Parses string received by a CONDUCTIVITY sensor
// MODIFIES: str
static void parse_conductivity_string(con_reading_t *reading, char *str) {
    const char *ec = strtok(str, ",");
    const char *tds = strtok(NULL, ",");
    const char *sal = strtok(NULL, ",");
    const char *sg = strtok(NULL, ",");

    reading->ec = atof(ec);
    reading->tds = atof(tds);
    reading->sal = atof(sal);
    reading->sg = atof(sg);
}

uint8_t atlas_take_single_reading(uint8_t sensor_address,
                                  reading_ptr_u reading) {
    uint8_t raw_reading[20] = {0u};
    uint8_t buf_sz = sensor_address == CONDUCTIVITY ? 19u : 7u;
    char *reading_start;

    // Send command to read
    uint8_t command[] = "R";
    i2c_write(sensor_address, command, 2u);

    // Wait for sensor to take readings
    CyDelay(1000u);

    // Read result
    i2c_read(sensor_address, raw_reading, buf_sz);

    // Parse the buffer if it is valid
    reading_start = strchr((const char *) raw_reading, 1u);
    if (reading_start == NULL) {
        return 0u;
    }

    // Move ptr to first digit of actual reading
    ++reading_start;

    // Parse based on sensor address and type of reading that we get
    if (sensor_address == CONDUCTIVITY) {
        parse_conductivity_string(reading.co_reading, reading_start);
    } else {
        // Parse generic reading
        *reading.generic_reading = strtof(reading_start, NULL);
    }
    return 1u;
}

static void wq_power_on_off(uint8_t val) {
    WQ_Power_Write(val);
    WQ_Power_1_Write(val);
    WQ_Power_2_Write(val);
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

    wq_power_on_off(1u);
    CyDelay(1000);

    I2C_Wakeup();
    I2C_Start();
    CyDelay(500);

    // Execute readings
    reading_ptr_u rd;

    rd.generic_reading = &atlas_water_temp;
    atlas_take_single_reading(TEMPERATURE, rd);

    rd.generic_reading = &atlas_do;
    atlas_take_single_reading(DO, rd);

    rd.generic_reading = &atlas_orp;
    atlas_take_single_reading(ORP, rd);

    rd.generic_reading = &atlas_ph;
    atlas_take_single_reading(PH, rd);

    rd.co_reading = &atlas_conductivity;
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
    wq_power_on_off(0u);

    *array_ix += 8;
    return *array_ix;
}

/* [] END OF FILE */
