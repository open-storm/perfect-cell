#include "optical_rain.h"

uint8 count = 0;
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




/* [] END OF FILE */
