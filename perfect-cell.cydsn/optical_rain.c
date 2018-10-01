/**
 * @file optical_rain.c
 * @brief Implements functions for hydreon optical rain sensor
 * @author Brandon Wong and Matt Bartos
 * @version TODO
 * @date 2017-06-19
 */

#include "optical_rain.h"

uint8  event = 0; // Keep track of each time the rain gage triggers an interrupt
uint16 count = 0; // Number of "tips" 

// prototype modem interrupt
CY_ISR_PROTO(isr_optical_rain);

CY_ISR(isr_optical_rain){
    // This interrupt is triggered by two events when using a tipping bucket
    // 1) Rising edge caused by an actual tip 
    // 2) A series of rising edges caused by bouncing as the tipping mechanism settles
    //
    // See https://www.allaboutcircuits.com/technical-articles/switch-bounce-how-to-deal-with-it/ 
    // for suggested fixes using an RC circuit or firmware
    
    // Increment each time an interrupt is triggered by the rain gage
    event++;   
    
    // Delay 500 microseconds to allow bouncing signal to pass
    // /!\ NOTE: This is not good practice and should either be fixed 
    //     by keeping track of the time or adding an RC circuit in order
    //     to prevent blocking other processes
    CyDelayUs(500u);
  
    // Update the number of "tips". This code double counts each tip, so 
    // use this if-statement to increment the actual number of tips once every 2 times
    if (event%2 == 0) {
        event = 0;
        count++;
        //LED_Write(!LED_Read());
    }
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
