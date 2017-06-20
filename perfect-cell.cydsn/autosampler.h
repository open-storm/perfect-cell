/**
 * @file autosampler.h
 * @brief Declares functions for ISCO autosampler
 * @author Brandon Wong and Matt Bartos
 * @version TODO
 * @date 2017-06-19
 */
#ifndef AUTOSAMPLER_H
#define AUTOSAMPLER_H
#endif

#include <project.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

#define AUTOSAMPLER_STATE_OFF       0
#define AUTOSAMPLER_STATE_IDLE      1
#define AUTOSAMPLER_STATE_BUSY      3
#define MAX_BOTTLE_COUNT            24
#define PULSE_COUNT                 20

extern uint8    autosampler_state;
extern uint8    SampleCount;

/**
 * @brief Starts the autosampler UART
 *
 * @return 1u on success
 */
uint8 autosampler_start();

/**
 * @brief Stops the autosampler UART
 *
 * @return 1u on success
 */
uint8 autosampler_stop();

/**
 * @brief Powers on the autosampler
 *
 * @return 1u on success
 */
uint8 autosampler_power_on();

/**
 * @brief Powers off the autosampler
 *
 * @return 1u on success
 */
uint8 autosampler_power_off();

/**
 * @brief Takes a sample with the autosampler and writes current bottle count to @p count
 *
 * @param count The current bottle count to be updated
 *
 * @return 1u on success
 */
uint8 autosampler_take_sample(uint8 *count);

/**
 * @brief Inserts current values of @p autosampler_trigger and @p bottle_count into labels and readings arrays.
 *
 * @param labels Array to store labels corresponding to each trigger result
 * @param readings Array to store trigger results as floating point values
 * @param array_ix Array index to label and readings
 * @param autosampler_trigger Current value of autosampler trigger
 * @param bottle_count Current bottle count
 * @param max_size Maximum size of label and reading arrays (number of entries)
 *
 * @return (*array_ix) + number of entries filled
 */
uint8 zip_autosampler(char *labels[], float readings[], uint8 *array_ix, int *autosampler_trigger, uint8 *bottle_count, uint8 max_size);

/* [] END OF FILE */
