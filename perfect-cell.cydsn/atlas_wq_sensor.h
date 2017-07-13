/**
 * @file atlas_wq_sensor.h
 * @brief Declares functions for Atlas water quality sensors
 * @author Abhiramm Mullapudi and Matt Bartos
 * @version TODO
 * @date 2017-06-19
 */
#ifndef ATLAS_WQ_SENSOR_H
#define ATLAS_WQ_SENSOR_H
#include "project.h"

#define CONDUCTIVITY 101
#define TEMPERATURE 96
#define DO 97
#define ORP 98
#define PH 102

#define ATLAS_MAX_ITER 100

struct{
    float ec;
    float tds;
    float sal;
    float sg;
}typedef con_reading;

/**
 * @brief Put Atlas I2C sensor into sleep mode
 *
 * @param sensor_address I2C slave address of sensor
 *
 * @return 1 on success
 */
int atlas_sensor_sleep(uint8 sensor_address);

/**
 * @brief Calibrate Atlas sensor using single-point method
 *
 * @param sensor_address I2C slave address of sensor
 *
 * @return 1 on success
 */
int atlas_sensor_calibrate(uint8 sensor_address);

/**
 * @brief Take a single reading from Atlas sensor in I2C mode.
 * Used for Temperature, DO, ORP and PH
 *
 * @param sensor_address I2C slave address of sensor
 * @param reading Value of reading (to be written to)
 *
 * @return 1u on success, and 0u otherwise.
 */
uint8 atlas_take_single_reading(uint8 sensor_address, float *reading);

/**
 * @brief Take a single reading from Atlas sensor in I2C mode.
 * Used for conductivity sensor.
 *
 * @param reading Struct for conductivity reading (to be written to)
 *
 * @return 1u on success, and 0u otherwise.
 */
uint8 atlas_take_con_reading(con_reading *reading);

/**
 * @brief Inserts current values of water quality measurements into labels and readings arrays.
 *
 * @param labels Array to store labels corresponding to each sensor reading
 * @param readings Array to store sensor readings as floating point values
 * @param array_ix Array index to label and readings
 * @param max_size Maximum size of label and reading arrays (number of entries)
 *
 * @return (*array_ix) + number of entries filled
 */
uint8 zip_atlas_wq(char *labels[], float readings[], uint8 *array_ix, uint8 max_size);

#endif
// Additional functionality
