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

float32 read_Vbat() {
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

void init_pins(){
    VBAT_READ_EN_Write(0u);  // VBAT pin
	Pin2_Write(0u);
    Pin37_Write(0u);
    Pin38_Write(0u);
    mux_controller_Write(0u);
}
/* [] END OF FILE */
