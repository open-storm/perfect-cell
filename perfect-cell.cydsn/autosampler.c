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

uint8 autosampler_take_sample(uint8* count){
    
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

CY_ISR(isr_SampleCounter){
    SampleCount1++;
}

/* [] END OF FILE */
