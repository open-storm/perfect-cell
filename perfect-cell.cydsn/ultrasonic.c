#include "ultrasonic.h"
#include <stdio.h>

#define MAX_STRING_LENGTH       128
#define DEPTH_STRING_LENGTH     4

// prototype modem interrupt
CY_ISR_PROTO(isr_byte_ultrasonic_rx);

// Declare variables
char uart_ultrasonic_received_string[MAX_STRING_LENGTH];
uint8 uart_ultrasonic_string_index = 0;

// Start the UART
void ultrasonic_start(){
    uart_ultrasonic_Start();
}
// Stop the UART
void ultrasonic_stop(){
    uart_ultrasonic_Stop();
}

// Provide power to the ultrasonic sensor
uint8 ultrasonic_power_on(uint8 which_ultrasonic){
    if (which_ultrasonic == 0u){
        Ultrasonic_ON_Write(1u);
    }
    else if (which_ultrasonic == 1u){
        Ultrasonic_2_ON_Write(1u);
    }
    return 1u;
}

// Cut power to the ultrasonic sensor
uint8 ultrasonic_power_off(uint8 which_ultrasonic){
    if (which_ultrasonic == 0u){
        Ultrasonic_ON_Write(0u);
    }
    else if (which_ultrasonic == 1u){
        Ultrasonic_2_ON_Write(0u);
    }
    return 0u;
}

// Start ISR, Fill array, Return array, Stop ISR
uint8 ultrasonic_get_reading(UltrasonicReading *reading) {
    
    int i, j;
    char depth[DEPTH_STRING_LENGTH] = {'\0'};
    char s[MAX_STRING_LENGTH] = {'\0'};
    
    uint8 which_ultrasonic;
    which_ultrasonic = mux_controller_Read();
    
    // Reset variables
    (*reading).valid = 0u;
    (*reading).temp = -9999;    
    (*reading).depth = -9999;
	
	ultrasonic_start();
    uart_ultrasonic_string_reset();

    isr_byte_ultrasonic_rx_StartEx(isr_byte_ultrasonic_rx); // Start the ISR to read the UART
    ultrasonic_power_on(which_ultrasonic);                                  // Power on the sensor
    CyDelay(1500u);                                         // Wait for UART to get readings from sensor
    
    isr_byte_ultrasonic_rx_Stop();                          // Stop the ISR to read the UART
    ultrasonic_power_off(which_ultrasonic);                                 // Power off the sensor
	ultrasonic_stop();

    // Store relevant strings to ultrasonic_packet
    strcpy(s,uart_ultrasonic_received_string);
    uart_ultrasonic_string_reset();

    for(i = 0; i < MAX_STRING_LENGTH-1-DEPTH_STRING_LENGTH; i++){
        if( s[i] == 'R' && s[i+1+DEPTH_STRING_LENGTH] == '\r' ) {
            for(j = 0; j < DEPTH_STRING_LENGTH; j++) {
                depth[j] = s[i+j+1];
            }
            (*reading).valid = 1u;
            break;
        }
    }
    
    if ((*reading).valid) {
        (*reading).depth = strtof(depth,(char **) NULL);
		
		if ((*reading).depth == 9999) {			
			(*reading).valid = 0u;
			(*reading).depth = -9999;
		}        
    }
	
    return (*reading).valid;
}

uint8 zip_ultrasonic(char *labels[], float readings[], uint8 *array_ix, uint8 which_ultrasonic, uint8 take_average, int ultrasonic_loops, uint8 max_size){

    // Ensure we don't access nonexistent array index
    uint8 nvars = 1;
    if(*array_ix + nvars >= max_size){
        return *array_ix;
    }
    
    uint8 valid = 0u;
    float valid_iter = 0.0;
    int read_iter = 0;
    UltrasonicReading ultrasonic_reading = {0u, 0u, 0u};
    char maxbotix_label[20] = {'\0'};
        
    // TODO: This should probably be generalized
    // May need to include string.h
    if (which_ultrasonic == 0u){
        sprintf(maxbotix_label, "maxbotix_depth");
    }
    if (which_ultrasonic == 1u){
        sprintf(maxbotix_label, "maxbotix_2_depth");
    }
        
    // Start the MUX 
    mux_controller_Wakeup();
        
    // Set MUX to read from 1st input
    mux_controller_Write(which_ultrasonic);
        
    // Add to labels
	labels[*array_ix] = maxbotix_label;
        
    for( read_iter = 0; read_iter < ultrasonic_loops; read_iter++){
        valid = ultrasonic_get_reading(&ultrasonic_reading);
        if ( ultrasonic_reading.valid == 1u){
            valid_iter++;
            readings[*array_ix] += ultrasonic_reading.depth;
            // If not taking the average, break the loop at the first valid reading
            if ( take_average == 0u ) {
                (*array_ix)++;
				break;
            }
        }            
    }
    // If taking the average, divide by the number of valid readings
    if ( take_average == 1u ) {
		if ( valid_iter > 0 ) {
            readings[*array_ix] = readings[*array_ix] / valid_iter;
            (*array_ix)++;
		}
    }
        
    // 2017 02 05: Send -1 instead of 9999 to avoid confusion
    // TODO: Test and then check in this update on GitHub
	// If there are no valid readings, send 9999
	if (valid_iter == 0.0) {
		readings[*array_ix] = -1;
		(*array_ix)++;
	}
        
    // Save MUX configuration + put MUX to sleep
    mux_controller_Sleep();
    return *array_ix;
}

void uart_ultrasonic_string_reset(){
    // reset uart_received_string to zero
       
    memset(&uart_ultrasonic_received_string[0],0,sizeof(uart_ultrasonic_received_string));
    uart_ultrasonic_string_index = 0;
    uart_ultrasonic_ClearRxBuffer();
}


CY_ISR(isr_byte_ultrasonic_rx){
    // hold the next char in the rx register as a temporary variable
    uint8 rx_char_hold = uart_ultrasonic_GetChar();
    
    // store the char in uart_received_string
    if(rx_char_hold) {
        uart_ultrasonic_received_string[uart_ultrasonic_string_index] = rx_char_hold;
        uart_ultrasonic_string_index++;
        
        if (uart_ultrasonic_string_index >= MAX_STRING_LENGTH) {
            uart_ultrasonic_string_index = 0; // reset the index to prevent an overflow
        }
    }   
}

/* [] END OF FILE */
