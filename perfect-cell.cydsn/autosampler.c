/**
 * @file autosampler.c
 * @brief Implements functions for ISCO autosampler
 * @author Brandon Wong and Matt Bartos
 * @version TODO
 * @date 2017-06-19
 */

#include <device.h>
#include <string.h>
#include "autosampler.h"

// prototype bottle_count interrupt
CY_ISR_PROTO(isr_SampleCounter);

// Declare variables
uint8 SampleCount = 0, SampleCount1 = 0, autosampler_state;;

uint8 autosampler_start(){
    isr_SampleCounter_StartEx(isr_SampleCounter);
    autosampler_state = AUTOSAMPLER_STATE_OFF;
    
    return 1u;
}

uint8 autosampler_stop() {
    return 1u;
}

/*
Start the autosampler.  Then power it on.
*/
uint8 autosampler_power_on() {
    Pin_Sampler_Power_Write(1u);
    CyDelay(1000u);//give the sampler time to boot
    
    autosampler_state = AUTOSAMPLER_STATE_IDLE;
    
    return 1u;
}

uint8 autosampler_power_off() {
    Pin_Sampler_Power_Write(0u);
    autosampler_state = AUTOSAMPLER_STATE_OFF;
    
    return 1u;
}

uint8 autosampler_take_sample(uint8 *count){
    
    if (*count >= MAX_BOTTLE_COUNT) {        
        return 0;
    }
    
    uint32 i = 0u, delay = 100u, interval;
    
    autosampler_state = AUTOSAMPLER_STATE_BUSY;
            
    // Send PULSE_COUNT pulses @ 10Hz to trigger sampler
    for(i=0; i < PULSE_COUNT; i++){
        Pin_Sampler_Trigger_Write(1u);
        CyDelay(100u);
        Pin_Sampler_Trigger_Write(0u);
        CyDelay(100u);
    }
    
    interval =  2u*60*1000/delay;           // Wait Max of 2 Minutes for distributor arm to move
    
    for (i = 0; i < interval ; i++) {  
        CyDelay(delay);
        if (Pin_Sampler_Completed_Sample_Read()!=0) { // Event pin on autosampler is HI
            break;
        }
    }
    
    
    if (Pin_Sampler_Completed_Sample_Read() != 0) {
        
        interval =  4u*60*1000/delay;       // Wait Max of 4 Minutes for pumping to complete
        for (i = 0; i < interval ; i++) { 
            CyDelay(delay);
            if (Pin_Sampler_Completed_Sample_Read()==0) { // Event pin on autosampler is HI
                break;
            }
        }
    }
    *count = BottleCount_Read();
    return 1u;
}

uint8 zip_autosampler(char *labels[], float readings[], uint8 *array_ix, int *autosampler_trigger, uint8 *bottle_count, uint8 max_size){
    // Ensure we don't access nonexistent array index
    uint8 nvars = 2;
    if(*array_ix + nvars >= max_size){
        return *array_ix;
    }
    (*autosampler_trigger) = 0u; // Update the value locally
    labels[*array_ix] = "autosampler_trigger"; // Update the database
    readings[*array_ix] = 0;
    (*array_ix)++;
        
    if (*bottle_count < MAX_BOTTLE_COUNT) {
	    labels[*array_ix] = "isco_bottle";
        autosampler_start();
        autosampler_power_on();                                    
        if (autosampler_take_sample(bottle_count) ) {
			readings[*array_ix] = *bottle_count;
		}
		else {
			// Use -1 to flag when a triggered sample failed
            readings[*array_ix] = -1;
		}                        
        autosampler_power_off(); 
        autosampler_stop();
	    (*array_ix)++;
		}
	else {
        //debug_write("bottle_count >= MAX_BOTTLE_COUNT");
		}
    return *array_ix;
}

CY_ISR(isr_SampleCounter){
    SampleCount1++;
}

/* [] END OF FILE */
