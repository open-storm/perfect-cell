/**
 * @file optical_rain.c
 * @brief Implements functions for hydreon optical rain sensor
 * @author Brandon Wong and Matt Bartos
 * @version TODO
 * @date 2017-06-19
 */

#include "optical_rain.h"

static volatile uint8 count = 0;
// prototype modem interrupt
CY_ISR_PROTO(isr_optical_rain);

CY_ISR(isr_optical_rain){
    // hold the next char in the rx register as a temporary variable
    count++;  
}

uint8 optical_rain_get_count() {
	return count;
}

void optical_rain_start() {
	isr_optical_rain_StartEx(isr_optical_rain);
	optical_rain_pwr_Write(1u);
	optical_rain_reset_count();
}

void optical_rain_stop() {
	optical_rain_pwr_Write(0u);
	isr_optical_rain_Stop();
}

void optical_rain_reset_count() {
	count = 0u;
}

uint8 zip_optical_rain(char *labels[], float readings[], uint8 *array_ix, uint8 max_size){
    // Ensure we don't access nonexistent array index
    uint8 nvars = 1;
    if(*array_ix + nvars >= max_size){
        return *array_ix;
    }
    float optical_rain_reading = 0;
    optical_rain_reading = 0.01 * optical_rain_get_count();
    labels[*array_ix] = "hydreon_prcp";
    readings[*array_ix] = optical_rain_reading;
	(*array_ix)++;
    return *array_ix;
}


/* [] END OF FILE */
