/**
 * @file atlas_wq_sensor.h
 * @brief Declares functions for Atlas water quality sensors
 * @author Abhiramm Mullapudi and Matt Bartos
 * @version TODO
 * @date 2017-06-19
 */
#ifndef ATLAS_WQ_SENSOR_H
#define ATLAS_WQ_SENSOR_H
#include <stdio.h>
#include <stdlib.h>
#include "project.h"
#include "strlib.h"

typedef struct {
    float ec;
    float tds;
    float sal;
    float sg;
} con_reading_t;

typedef union {
    float *generic_reading; con_reading_t *co_reading;
} reading_ptr_u;

/**
 * @brief Put Atlas I2C sensor into sleep mode
 *
 * @param sensor_address I2C slave address of sensor
 *
 * @return 1 on success
 */
uint8_t atlas_sensor_sleep(uint8_t sensor_address);

/**
 * @brief Calibrate Atlas sensor using single-point method
 *
 * @param sensor_address I2C slave address of sensor
 *
 * @return 1 on success
 */
uint8_t atlas_sensor_calibrate(uint8_t sensor_address);

/**
 * @brief Take a single reading from Atlas sensor in I2C mode.
 * Used for Temperature, DO, ORP and PH
 *
 * @param sensor_address I2C slave address of sensor
 * @param reading Value of reading (to be written to)
 *
 * @return 1u on success, and 0u otherwise.
 */
uint8_t atlas_take_single_reading(uint8_t sensor_address, reading_ptr_u reading);

/**
 * @brief Inserts current values of water quality measurements into labels and
 * readings arrays.
 *
 * @param labels Array to store labels corresponding to each sensor reading
 * @param readings Array to store sensor readings as floating point values
 * @param array_ix Array index to label and readings
 * @param max_size Maximum size of label and reading arrays (number of entries)
 *
 * @return (*array_ix) + number of entries filled
 */
uint8_t zip_atlas_wq(char *labels[], float readings[], uint8_t *array_ix,
                     uint8_t max_size);

#endif
