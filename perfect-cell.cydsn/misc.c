#include "misc.h"

uint8 blink_LED(uint8 n_blinks) {
	uint8 k;
	for (k = 0; k < n_blinks; k++) {
		LED_Write(1u);
		CyDelay(200u);
		LED_Write(0u);
		CyDelay(200u);
	}	
	return 1u;
}

float32 read_vbat() {
	int32 reading;
	float32 v_bat;
	/* flip on the ADC pin */
	VBAT_READ_EN_Write(1u);
	CyDelay(100u);	
	/* Start the ADC */
	VBAT_ADC_Wakeup();
	VBAT_ADC_Start(); 
    
    AMux_Start();
    AMux_Select(0u);
	/* Read the voltage */
    reading = VBAT_ADC_Read32();
	v_bat = 11.0 * VBAT_ADC_CountsTo_Volts(reading);
	/* Stop the conversion */
	VBAT_ADC_Sleep();
	/* flip off the ADC pin */
	VBAT_READ_EN_Write(0u);	
    
    AMux_Stop();
	return v_bat;
}

uint8 zip_vbat(char *labels[], float readings[], uint8 *array_ix, uint8 max_size){
    // Ensure we don't access nonexistent array index
    uint8 nvars = 1;
    if(*array_ix + nvars >= max_size){
        return *array_ix;
    }
    float32 v_bat = -9999;
    v_bat = read_vbat();
    labels[*array_ix] = "v_bat";
    readings[*array_ix] = v_bat;
    (*array_ix)++;
    return *array_ix;
}

void init_pins(){
    VBAT_READ_EN_Write(0u);  // VBAT pin
	//Pin2_Write(0u);
    //Pin37_Write(0u);
    //Pin38_Write(0u);
    mux_controller_Write(0u);
}
/* [] END OF FILE */
