/**
 * @file SDI12.c
 * @brief Implements functions for SDI12 sensors.
 * @author Brandon Wong
 * @version TODO
 * @date 2017-10-28
 */

#include "SDI12.h"
#include "misc.h"

//*========== #BEGIN Define SDI12 sensors ==========//
//----------   SDI12 Array   ----------//
#define DEBUG_SDI12         1
#define N_SDI12_SENSORS     2           // Total number of sensors defined
SDI12_sensor sensors[N_SDI12_SENSORS];  

//!\ WARNING: labels[] and values[] are variable names used in data.c /!\
//----------  (1/2) Solinst  ----------//
char* solinst_labels[] = {"solinst_T","solinst_P"};
float solinst_values[] = {-99.0, -98.0};
SDI12_sensor solinst = { 
    .nvars   = 2,
    .address = "0",
    .labels  = solinst_labels,
    .values  = solinst_values
    // additional metadata should initialize to 0 or '\0' //
}; 

//----------  (2/2) Decagon  ----------//
char* GS3_labels[] = {"GS3_1","GS3_2","GS3_3"};
float GS3_values[] = {-97.0, -95.0, -94.9};
SDI12_sensor GS3 = {
    .nvars   = 3,
    .address = "1",
    .labels  = GS3_labels,
    .values  = GS3_values        
    // additional metadata should initialize to 0 or '\0' //
};      
    
//---------- GOTO zip_SDI12()----------//
    
// ========== #END   Define SDI12 sensors ==========//*/

// The buffer and index are dependent on eachother. This buffer is implemented
// as a circular buffer to avoid overflow.
static char SDI12_uart_buf[257] = {'\0'};
static uint8_t SDI12_buf_idx = 0u;

void SDI12_start(){
    SDI12_UART_Start();
    isr_SDI12_StartEx(isr_SDI12_data); 
}

void SDI12_stop(){
    isr_SDI12_Stop(); 
    SDI12_UART_Stop();
}

void SDI12_sleep(){
    SDI12_UART_Sleep();   
}

void SDI12_wakeup() {
    SDI12_UART_Wakeup();
    SDI12_uart_clear_string();
}

/**
 * @brief 
 */
void SDI12_uart_clear_string() {
    SDI12_UART_ClearRxBuffer();
    memset(SDI12_uart_buf, '\0', sizeof(SDI12_uart_buf));
    SDI12_buf_idx = 0u;
}

char* SDI12_uart_get_string() {
    return SDI12_uart_buf; 
}


/**
 * @brief Send an SDI12-formatted command. A command always starts with
 *        the address of the sensor, followed by the command, and is terminated by 
 *        and exclamation point: <address><command>!
 * 
 * @param command String of the command to send
 *
 */
void SDI12_send_command(char command[]) {
    // Wake up the sensors 
    // Pull the Data pin high to send 12.5 millisecond break
    SDI12_control_reg_Write(1u);
    CyDelayUs(12500u);  // 12.5 milliseconds = 12.5 x 10^3 microseconds

    // Pull the Data pin low for 8.4 milliseconds to indicate marking
    SDI12_control_reg_Write(0u);
    CyDelayUs(8400u);   // 8.4 milliseconds = 8.4 x 10^3 microseconds
        
    // Write the command to the Data pin
    SDI12_UART_PutString(command);
    
    // Release control of the Data line
    SDI12_control_reg_Write(0u);
}

/**
 * @brief Check if an SDI12 sensor is on, given its address
 * 
 * @param sensor SDI12_sensor that we want to check the status of
 *
 * @return 1u if active, 0u if inactive
 */
uint8 SDI12_is_active(SDI12_sensor* sensor) {
    /* 0. Define Local Variables: */
    // string for constructing the command sent to the SDI12 sensor
    char command[100] = {'\0'};

    // string to store and parse SDI12 measurement results
    char value_str[100] = {'\0'};
    
    /* 1. Request if SDI12 sensor is active */
    clear_str(command);
    
    /* 2. Construct the command <address><command><!> */
    sprintf(command,"%s%s%s",(*sensor).address,ACK_ACTIVE,"!");
    SDI12_uart_clear_string();
    
    /* 3. Send the command to the sensor */
    SDI12_send_command(command);
    
    /* 4. Parse the received data, if any */
    /* TODO: This can be shortened using a for-loop like in SDI12_take_measurement() */
    CyDelay(1000u); // Delay for readings to be transmitted
    
    // Copy the SDI12 UART buffer into temporary buffer
    // *TODO: Parse sensor address from the response
    clear_str(value_str);
    if (strextract(SDI12_uart_buf, value_str, "!" ,"\r\n")) {
        // If "\r\n" is in the response, the sensor is active
        return 1u;
    }
    
    return 0u;
}

/** 
 * @brief Change the address of an SDI12 sensor
 *
 * @param sensor SDI12_sensor that we want to change the address of
 * @param new_address String (single char, either 0-9, a-z, A-Z) of the desired address
 *
 * @return 1u if successful, 0u if unsuccessful
 */
uint8 SDI12_change_address(SDI12_sensor* sensor, char new_address[]) {
    /* 0. Define Local Variables: */
    // string for constructing the command sent to the SDI12 sensor
    char command[100] = {'\0'};

    // string to store and parse SDI12 measurement results
    char value_str[100] = {'\0'};
    
    // unsigned ints for taking measurements
    uint8         i = 0u; // for iterating through sensor measurements
        
    /* 1. Request measurement from addressed SDI12 sensor */
    clear_str(command);
    sprintf(command,"%s%s%s%s",(*sensor).address,CHANGE_ADDR,new_address,"!");
    
    SDI12_uart_clear_string();
    SDI12_send_command(command);    
        
    // Delay max 1000 ms
    // Look for <new address><CR><LF> in the buffer
    clear_str(value_str);
    sprintf(value_str, "%s\r\n", new_address);
    for (i = 0; i < 200; i++) { 
        CyDelay(5u);
        if ( strstr(SDI12_uart_buf, value_str) ) {
            (*sensor).address = new_address;
            return 1u;
        }
    }
    
    return 0u;
}


/**
 * @brief Take a measurement for an SDI12 sensor, given its address
 * 
 * @param sensor SDI12_sensor that we want to request measurements from
 *
 * @ return 1u if measurements were successfully taken, 0u if there was a communication error
 */
uint8 SDI12_take_measurement(SDI12_sensor* sensor) {
    /* 0. Define Local Variables: */
    // string for constructing the command sent to the SDI12 sensor
    char command[100] = {'\0'};

    // string and pointer to store and parse SDI12 measurement results
    char value_str[100] = {'\0'}, delay_str[10] = {'\0'}, *ptr_end, *sign; 
    
    // unsigned ints for taking measurements
    uint8 valid = 0u;
    uint8 inner = 0u, outer = 0u, MAX_INNER = 5u, MAX_OUTER = 5u;    
    uint8         i = 0u; // for iterating through sensor measurements
    uint32 ms_delay = 0u; // for delaying the code while measurements are being taken    


    // <http://www.sdi-12.org/current_specification/SDI-12_version-1_4-Dec-1-2017.pdf>
    // See Sect 7.0 and Appendix B-1 for more details on retries and flow control
    // (BW)   2017/12/04 - Solinst Levelogger 3000 can still fail
    for (outer = 0u; outer < MAX_OUTER; outer++) {
        /* 1. Request measurement from addressed SDI12 sensor */    
        clear_str(command);
        clear_str(value_str); 
        clear_str(delay_str);      
        sprintf(command,"%s%s%s",(*sensor).address,TAKE_MEASUREMENT,"!");
        
        SDI12_uart_clear_string();
        CyDelay(100u);
        SDI12_send_command(command);
        
        /* 2. Delay for measurement to be taken */
        /*
         * The address sensor responds within 15.0 ms, returning the
         * maximum time until the measurement data will be ready and the
         * number of data values it will return
         */
        
        // supposedly response time is 15 ms, but delaying max 1000 ms to be safe
        // TODO: When taking concurrent measurements, look for <address><CR><LF>
        for (i = 0; i < 200; i++) { 
            CyDelay(5u);
            if ( strextract(SDI12_uart_buf, value_str, "!","\r\n") ) {
                break;
            }
        }
        
        /* Delay while measurements are being prepared */
        /*
         * The returned format for <address>M! is atttn<CR><LF>
         * (NOTE: The returned format for <address>C! is atttnn<CR><LF> -- this will be useful when requesting 
         *        concurrent measurements in the future)
         * Where,
         * -    a: sensor address
         * -  ttt: maximum time until measurement is ready
         * - n(n): number of data values that will be returned
         */
        strncpy(delay_str, value_str+strlen((*sensor).address), 3); // shift ptr by strlen(...) to skip address byte
        ms_delay = ( (uint32) strtod(delay_str,(char**) NULL) ) * 1000u;
        CyDelay(ms_delay);
        
        for (inner = 0u; inner < MAX_INNER; inner++) {
            /* 3. Request data from SDI12 sensor */
            clear_str(command);    
            clear_str(value_str);
            sprintf(command,"%s%s%s",(*sensor).address,READ_MEASUREMENT,"!");   
            
            SDI12_uart_clear_string(); 
            SDI12_send_command(command);
            
            // Delay a maximum of 200 ms as readings are transmitted
            for (i = 0; i < 10; i++) {
                CyDelay(20u);
                
                // Copy the SDI12 UART buffer into temporary buffer, value_str
                // and break once "\r\n" is returned
                if ( strextract(SDI12_uart_buf, value_str, "!","\r\n") ) {
                    break;
                }
            }
            
            
            /* 4. Parse the received data */    
            // Check for "+" or "-" signs to ensure data was sent
            // First check for "+"
            sign = strstr(value_str,"+");
            if (sign == NULL) {
                // If there is no + sign, check for "-"
                sign = strstr(value_str,"-");
                
                // The absence of "+" or "-" indicates no data was sent yet
                // This has particularly been an issue with Solinst Levelogger 3001
                // Leaving notes below to potentially help future fix
                //
                // Notes:
                // <http://www.sdi-12.org/current_specification/SDI-12_version-1_4-Dec-1-2017.pdf>
                // (Page 15, Sect 4.4.8, after Table 11)
                // If the response to a D command is valid, but no data are returned, 
                // the sensor has aborted the measurement. 
                // To obtain data the recorder must issue another M, C, or V command.
                //
                // <https://www.solinst.com/products/dataloggers-and-telemetry/3001-levelogger-series/operating-instructions/sdi-12-user-guide/6-trouble-shooting/6-trouble-shooting.php>
                // **Recorder Receives Invalid Response to D command e.g. 0000<CR><LF>**
                //
                // Retry the Command. The Levelogger may have been busy or 
                // the Levelogger is not connected to the SDI-12 Interface Cable.
                // If your recorder supports automatic retries, consider enabling that function.
                // If necessary, check all connections in the field.
                //
                if (sign == NULL) {
                    for (i = 0; i < (*sensor).nvars; i++) {
                        (*sensor).values[i] = -9999.0f;
                    }
                    valid = 0u;
                    
                }
            }
            
            if (sign != NULL) {
                // Set the pointer to the start of the returned results
                // and shift forward past the address byte
                ptr_end = value_str + strlen((*sensor).address);
                
                // Parse and store all values returned from the sensor
                for (i = 0; i < (*sensor).nvars; i++) {
                    (*sensor).values[i] = (float) strtod(ptr_end, &ptr_end);
                }
                
                valid = 1u;
                return valid;
            }
        }
    }
    return valid;
}

/**
 * @brief TODO: Take a concurrent measurement for an SDI12 sensor, given its address
 * 
 * @param sensor SDI12_sensor that we want to request measurements from
 *
 * @ return if sensor is active
 */
uint8 SDI12_take_concurrent_measurement(SDI12_sensor* sensor) {
    /* TODO */
    /*
     * Concurrent measurements enable measurements to be triggered for
     * multiple sensors, rather than doing one at a time.
     * 
     * Requires SDI12 V1.2 or greater.
     *
     * Currently returning a dummy response for now.
     */
    return SDI12_is_active(sensor);
}

/**
 * @brief Obtain detailed info about an SDI12 sensor, given its address
 * 
 * @param sensor SDI12_sensor that we want to request info from
 *
 * @ return 1u if successful, 0u if unsuccessful
 */
uint8 SDI12_info(SDI12_sensor* sensor) {
    /* 0. Define Local Variables: */
    // string for constructing the command sent to the SDI12 sensor
    char command[100] = {'\0'};

    // string and pointer to store and parse SDI12 measurement results
    char value_str[100] = {'\0'}, *ptr_end; 
    
    // unsigned ints for taking measurements
    uint8         i = 0u; // for iterating through sensor measurements
    
    /* 1. Request measurement from addressed SDI12 sensor */
    clear_str(command);
    sprintf(command,"%s%s%s",(*sensor).address,INFO,"!");
    
    SDI12_uart_clear_string();
    SDI12_send_command(command);    

    clear_str(value_str);
    
    // Look for <CR><LF> in the buffer since <CR><LR> terminates all responses, if there is one 
    // Delay max 1000 ms while doing so
    for (i = 0; i < 200; i++) { 
        CyDelay(5u);
        if (strextract(SDI12_uart_buf, value_str, "!" ,"\r\n")) {
        // If "\r\n" is in the response, then the sensor is active
            
            ptr_end = value_str;
            // Attempt to parse the information and store in "sensor"
            // Move the pointer past the address 
            ptr_end += strlen((*sensor).address);
            
            // SDI-12 specification (2 chars)
            strncpy((*sensor).v_SDI12, ptr_end, 2);
            ptr_end += 2;
            
            // Vendor identification (8 chars)
            strncpy((*sensor).vendor, ptr_end, 8);
            ptr_end += 8;
            
            // Sensor Model (6 chars)
            strncpy((*sensor).model, ptr_end, 6);
            ptr_end += 6;
            
            // Sensor Version (3 chars)
            strncpy((*sensor).version, ptr_end, 3);
            ptr_end += 3;
            
            // Sensor Serial number / misc info (up to 13 chars)
            strncpy((*sensor).serial, ptr_end, 13);
            
            // Assume successful if we reached this point
            return 1u;
        }
    }
    
    return 0u;
}

uint8 zip_SDI12(char *labels[], float readings[], uint8 *array_ix, uint8 max_size, int SDI12_flag) {
    
    //----------   Create Array  ----------//
    sensors[0] = solinst;
    sensors[1] = GS3;    
    
    // Define local variables     
    uint8 i, j, k, valid;
    //char output[100] = {'\0'}; // For debugging
    
    SDI12_start();
    SDI12_Power_Write(1u);
      
    // Iterate through each SDI12 sensor, using SDI12_flag 
    // where each bit is a flag to an SDI-12 sensor
    //  (This assumes the number of bits < N_SDI12_SENSORS)
    for (k = 0; k < N_SDI12_SENSORS;  k++) {
        if (0x00000001 & SDI12_flag>>k)  {
     
            // Ensure we don't access nonexistent array index
            if(*array_ix + (sensors[k]).nvars >= max_size){
                return *array_ix;
            }    
            //*/
            
            //* ---------- Take measurement(s) from k'th SDI-12 sesnor ---------- //
            valid = 0; // Reinitialize sensor status
            
            // Let the sensor power up max 10000 ms = 50 * 200 ms
            for (i = 0; i < 50; i++) {
                CyDelay(200u);
                
                valid = SDI12_is_active(&sensors[k]); //CyDelay(200u);
                if (valid == 1u){  
                    
                    if (DEBUG_SDI12 == 1u) {
                        LED_Write(0u);
                    }
                    
                    //valid = SDI12_change_address(&sensors[0],"0"); CyDelay(200u);
                    valid = SDI12_info(&sensors[k]); //CyDelay(200u);
                    valid = SDI12_take_measurement(&sensors[k]);

                    if (valid == 1u) {
                        // Successfully took a measurement 
                        if (DEBUG_SDI12 == 1u) {
                            blink_LED(1u);    
                        }
                    } else {
                        /*/ SDI12 sensor powered on, but unable to parse response
                        valid = -2;                    
                        for (j = 0; j < (sensors[k]).nvars; j++) {
                            (sensors[k]).values[j] = -9999.2;
                        }
                        //*/
                        if (DEBUG_SDI12 == 1u) {
                            blink_LED(4u);
                        }
                    }    
                    break;
                                            
                } else {
                    // SDI12 sensor not responding. It may be powered off or have a different address
                    valid = -1;
                    for (j = 0; j < (sensors[k]).nvars; j++) {
                        (sensors[k]).values[j] = -9999.1;
                    }
                    if (DEBUG_SDI12 == 1u) {
                        LED_Write(1u);
                    }
                }
            }
            
            //* ---------- Insert measurement(s) from k'th SDI-12 sensor ---------- //
            // char[] output is used for debugging purposes
            //clear_str(output);
            
            // Insert current values in labels[] and readings[]
            for (j = 0; j < (sensors[k]).nvars; j++) {
                // char[] output is used for debugging purposes
                //sprintf(output,"%s %s %f", output, (sensors[k]).labels[j], (sensors[k]).values[j]);
                
                labels[*array_ix] = (sensors[k]).labels[j];
                readings[*array_ix] = (sensors[k]).values[j];
                (*array_ix) += 1;
            }
        }
    }
                
    SDI12_Power_Write(0u);
    SDI12_stop(); 
    
    return *array_ix;
}

CY_ISR(isr_SDI12_data) {
    // hold the next char in the rx register as a temporary variable
    char rx_char = SDI12_UART_GetChar();

    // store the char in SDI12_uart_buf
    if (rx_char) {
        // unsigned ints don't `overflow`, they reset at 0 if they are
        // incremented one more than it's max value. It will be obvious if an
        // `overflow` occurs
        SDI12_uart_buf[SDI12_buf_idx++] = rx_char;
    }
}

/* [] END OF FILE */
