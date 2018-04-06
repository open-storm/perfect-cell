/*
 * @file four20.c
 * @brief Take an analog reading from a current loop (4-20 mA) sensor
 * @author Brandon Wong
 * @version TODO
 * @date 2018-04-03
 */

#include "four20.h"
#include "device.h"

/**
 * @brief Take a an analog reading from a 4-20 mA sensor across a 150 ohm resistor
 * 
 * @param v_420mA float32 of the voltage across 150 ohm resistor
 *
 */  
uint8 read_420mA(float32 *v_420mA) {
	int32 reading;
    uint8 status = 0u;
    
	/* flip on the ADC pin & 12V pin */
	VBAT_READ_EN_Write(1u);
    SDI12_Power_Write(1u);
	CyDelay(100u);	
    
	/* Start the ADC */
	VBAT_ADC_Wakeup();
	VBAT_ADC_Start(); 
    
    /* Start the Mux and select 4th input */
    AMux_Start();
    AMux_Select(3u);
    
	/* Read the voltage */
    reading = VBAT_ADC_Read32();
	*v_420mA = VBAT_ADC_CountsTo_Volts(reading);
    status = 1u;
    
	/* Stop the conversion */
	VBAT_ADC_Sleep();
    
	/* flip off the ADC pin & 12V pin*/
	VBAT_READ_EN_Write(0u);	
    SDI12_Power_Write(0u);
    
    /* Turn of the Analog MUX */
    AMux_Stop();
    
	return status;
}

/**
 * @brief Inserts current value of 4-20 mA sensor into labels and
 * readings arrays.
 *
 * @param labels Array to store labels corresponding to each sensor reading
 * @param readings Array to store sensor readings as floating point values
 * @param array_ix Array index to label and readings
 * @param max_size Maximum size of label and reading arrays (number of
 * entries)
 *
 * @return (*array_ix) + number of entries filled
 */
uint8 zip_4_20mA(char *labels[], float readings[], uint8 *array_ix, uint8 max_size){
    // Ensure we don't access nonexistent array index
    uint8 nvars = 1;
    if(*array_ix + nvars >= max_size){
        return *array_ix;
    }
    
    /* Take a reading from the sensor */
    float32 v_420mA = -9999;
    read_420mA(&v_420mA);
    
    /* Insert reading into "labels" and "readings" arrays */
    labels[*array_ix] = "v_420mA";
    readings[*array_ix] = v_420mA;
    (*array_ix)++;
    
    return *array_ix;
}


/* [] END OF FILE */
