/**
 * @file valve.c
 * @brief Implements functions for operating valve
 * @author Brandon Wong
 * @version TODO
 * @date 2017-06-19
 */

#include "valve.h"

int test_valve(){
	int p_count = 0;
	
	for (p_count = 0; p_count < 2; p_count++){
		blink_LED(5u);
		Valve_OUT_Write(1u);
		CyDelay(200u);
		Valve_OUT_Write(0u);
		
		Valve_IN_Write(1u);
		CyDelay(200u);
		Valve_IN_Write(0u);
        
		Valve_2_OUT_Write(1u);
		CyDelay(200u);
		Valve_2_OUT_Write(0u);
		
		Valve_2_IN_Write(1u);
		CyDelay(200u);
		Valve_2_IN_Write(0u);
                
	}
    return 1;
}

float32 read_Valve_POS() {
	
	uint32 reading;
	float32 valve_pos, v_bat;
	
	/* Get the battery voltage */
	v_bat = read_vbat();
    
    /* Start the Analog MUX and select second input */
    AMux_Start();
    AMux_Select(1u);
	
	/* flip on the ADC pin */
	Valve_POS_EN_Write(1u);
	CyDelay(100u);
	
	/* Restore User configuration & start the ADC */
	VBAT_ADC_Wakeup();
	VBAT_ADC_Start(); 
	
	/* Set up the multiplexer */
	AMux_Select(1u);
	
	/* Read the potentiometer voltage in bits */
    reading = VBAT_ADC_Read32();
	
	/* Calculate ratio of the potentiometer voltage to the battery voltage 
	   This returns the percentage that the valve is open
	*/
	valve_pos = VBAT_ADC_CountsTo_Volts(reading) / v_bat;
	
	/* Stop the conversion & save user configuration */
	VBAT_ADC_Sleep();
	
	
	/* flip off the ADC pin */
	Valve_POS_EN_Write(0u);	
	
    /* Turn of the Analog MUX */
    AMux_Stop();
    
	return valve_pos;
}

int move_valve(int valve){
	int iter;
	int valve_out;
	float32 valve_pos, d_valve;
	
	if (valve == 0){
		// Push the actuator out and fully close the valve 
		for(iter = 0; iter < 24; iter++) {
			valve_pos = read_Valve_POS();
								
			// valve_pos maxes out at 0.88 but we'll leave
			//   this at 0.90 to ensure the valve fully closes
			//
			//if(valve_pos > 0.90) {									
			//	break;
			//}
								
			Valve_OUT_Write(1u);
			CyDelay(1000u);
			Valve_OUT_Write(0u);
			}
														
		// Acknowledge the command
		valve_out = -1*iter;
		return valve_out;
		}
	else if (valve == 100){
		// Push the actuator out and fully close the valve 
		for(iter = 0; iter < 23; iter++) {
			valve_pos = read_Valve_POS();
								
			// say limit for open valve is 0.05
			//if(valve_pos > 0.05) {									
			//	break;
			//}
								
			Valve_IN_Write(1u);
			CyDelay(1000u);
			Valve_IN_Write(0u);
		}
														
		// Acknowledge the command
		valve_out = -1*iter;
		return valve_out;
	}
	else if(valve > 0 && valve < 100){	
														
	    // Derivative controller
	    // move the actuator within 5 percent of the desired pos
		for(iter = 0; iter < 20; iter++){
			valve_pos = 100 * read_Valve_POS();
			d_valve = valve - valve_pos;
								
			// pull the actuator in if desired pos > current pos
			// to open up the valve
			// (e.g. desired is 95% open and valve is currently 50% open)
			if(d_valve >= 2.5){
				Valve_IN_Write(1u);
				CyDelay(1000u);
				Valve_IN_Write(0u);	
			}
		    // push the actuator out if desired pos < current pos
			// to close the valve
			// (e.g. desired is 15% open and valve is currently 40% open)
			if (d_valve <= -2.5){
				Valve_OUT_Write(1u);
				CyDelay(1000u); 
				Valve_OUT_Write(0u);
			}
								
			// reached objective of getting within 1 percent
			if (d_valve < 2.5 && d_valve > -2.5){
				break;
			}
		}							
		// Acknowledge the command
		valve_out = -1*iter;
		return valve_out;						
		}
    return valve_out;
}

uint8 zip_valve(char *labels[], float readings[], uint8 *array_ix, int *valve_trigger, uint8 max_size){
    // Ensure we don't access nonexistent array index
    uint8 nvars = 1;
    if(*array_ix + nvars >= max_size){
        return *array_ix;
    }
        // Ellsworth does not have a potentiometer installed
        // Simply flip the pins for now and come back to implement
        // a pulse counter
        /*        
        float32 valve_pos;
		valve = move_valve(valve);
		valve_pos = 100. * read_Valve_POS();
		labels[array_ix] = "valve_cmd";
		labels[array_ix + 1] = "valve_pos";
		readings[array_ix] = valve;
		readings[array_ix + 1] = valve_pos;
		array_ix += 2;
        */
        
        // If zero, open the valve completely
        // IMPORTANT: If there is a "null" entry,
        //            intparse_influxdb returns 0
        //            Make sure default is to open the valve
        if (*valve_trigger == 0) { 
            Valve_OUT_Write(1u);
            CyDelay(20000u);
            Valve_OUT_Write(0u);
        // Else, if 100, close the valve completely
        } else if(*valve_trigger == 100) {
            Valve_IN_Write(1u);
            CyDelay(20000u);
            Valve_IN_Write(0u);
        } else {
            // For now, do nothing for the other cases
        }
        
        // Acknowledge the trigger by updating it to -1
        // -1, and negative values are reserved for actuator response
        labels[*array_ix] = "valve_trigger";
        readings[*array_ix] = -1;
        (*array_ix) += 1;
        return *array_ix;
}

uint8 zip_valve_2(char *labels[], float readings[], uint8 *array_ix, int *valve_2_trigger, uint8 max_size){
    // Ensure we don't access nonexistent array index
    uint8 nvars = 1;
    if(*array_ix + nvars >= max_size){
        return *array_ix;
    }
        // If zero, open the valve completely
        // IMPORTANT: If there is a "null" entry,
        //            intparse_influxdb returns 0
        //            Make sure default is to open the valve
        if (*valve_2_trigger == 0) { 
            Valve_2_OUT_Write(1u);
            CyDelay(20000u);
            Valve_2_OUT_Write(0u);
        // Else, if 100, close the valve completely
        } else if(*valve_2_trigger == 100) {
            Valve_2_IN_Write(1u);
            CyDelay(20000u);
            Valve_2_IN_Write(0u);
        } else {
            // For now, do nothing for the other cases
        }
        
        // Acknowledge the trigger by updating it to -1
        // -1, and negative values are reserved for actuator response
        labels[*array_ix] = "valve_2_trigger";
        readings[*array_ix] = -1;
        (*array_ix) += 1;
        return *array_ix;
}

/* [] END OF FILE */
