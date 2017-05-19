#include <device.h>
#include <stdio.h>
#include <string.h>
#include "modem.h"
#include "updater.h"
#include "extern.h"

// declare variables
int	   iter = 0;
uint8  modem_state, lock_acquired = 0u, ready = 0u;
uint16 uart_string_index = 0u;
uint32 feed_id;
char   uart_received_string[1500] = {'\0'};
char*  modem_apn = "epc.tmobile.com";

// prototype modem interrupt
CY_ISR_PROTO(Telit_isr_rx);
void uart_string_reset();

// startup sequence to power on the modem, start modem
// components and prepare for sending/receiving messages
// over the network
//
// returns the status of the modem and stores the number
// of connection attempts made in *conn_attempts
uint8	modem_startup(int *conn_attempts) {
	iter = 0, ready = 0u;

	// Modem is already connected to network
        if( modem_state == MODEM_STATE_READY) {
            return 1u;
        }
	
	modem_start();
	
	while( iter < max_conn_attempts ) {
		iter++;
        *conn_attempts = *conn_attempts + 1;  //alternatively, ++*conn_attempts;

		/* Set up the modem */
		ready = modem_power_on();
		modem_set_flow_control(0u);	
		modem_setup();
		
		if ( ready == 1u ) {
            
			// Connect modem to network
			ready = modem_connect();
			
			if( ready == 3u ) {
                
                iter = max_conn_attempts;//break;	
				return 1u;
			}
		}
		else {
			modem_reset();
		}
	}
	
	// Timed out -- Failed to connect
	return 0u;
}

// shutdown sequence to stop modem components
// and power down the modem
uint8 	modem_shutdown() {
	if (modem_power_off()){
		return 1u;	
	}
	
	return 0u;
}

// initialize modem
void modem_start(){
    Telit_UART_Start();
    Telit_ControlReg_Write(0u);
	Telit_ON_Write(1u);			// Prep modem for "push button"
	Telit_RST_Write(1u);		// Prep modem for "push button"
    Telit_isr_rx_StartEx(Telit_isr_rx);
    modem_state = MODEM_STATE_OFF;
}

// deinitialize modem
void modem_stop(){
    Telit_UART_Stop();
    Telit_ControlReg_Write(0u);
	Telit_ON_Write(0u);			// Save energy by pulling down "push button"
	Telit_RST_Write(0u);		// Save energy by pulling down "push button"
    Telit_isr_rx_Stop();
    modem_state = MODEM_STATE_OFF;
}

// send at-command to modem
uint8 at_write_command(char* uart_string, char* expected_response, uint32 uart_timeout){
    uint8 response = 0u;
    uint32 i = 0u, delay = 100u, interval = uart_timeout/delay;
    
    uart_string_reset();
    Telit_UART_PutString(uart_string);
    
    if( strcmp(expected_response, "") != 0){
        for(i=0;i<interval;i++){
            char* valid = strstr(uart_received_string, expected_response);
            if(valid != NULL){
                response = 1u;
                break;
            }
            CyDelay(delay);
        }
    }
    
    return response;
}

uint8 modem_power_on(){

    if (modem_state != MODEM_STATE_OFF) {
        // Modem is already on
        return 1u;
    }
    
    // Set ON, PWR pins low
    Telit_ON_Write(0u); 
    Telit_RST_Write(0u);
    
    // Provide power to Telit module
    Telit_ControlReg_Write(1u);
    
    // "Push" the ON button for 2 seconds
    Telit_ON_Write(1u); 
    CyDelay(2000u);     // the pad ON# must be tied low for at least 1 second and then released.
    //CyDelay(1500u);     // At least 3 seconds if VBAT < 3.4 for GC 864
    Telit_ON_Write(0u); 
    
    CyDelay(5000u);  
    /* NOTE:
        "To get the desirable stability, 
        CC864-DUAL needs at least 10 seconds 
        after the PWRMON goes HIGH.*/
	
	if(at_write_command("AT\r","OK",1000u) == 1){    
        modem_state = MODEM_STATE_IDLE;
        return modem_state;
    }	
	
	// Failed to turn on
    return 0u;
}

uint8 modem_power_off(){
	
    if (modem_state == MODEM_STATE_OFF) {
        // Modem is already off
        return 1u;
    }
	
    // Try to disconnect the modem.  
    // Continue whether or not disconnect is successful
    modem_disconnect();
    

    if(at_write_command("AT#SHDN","OK",1000u) != 1){  
        // If the command fails, issue a hard reset  
        // "Push" the ON button for 
        Telit_ON_Write(1u);
        CyDelay(2500u);  // To turn the CC864-DUAL off, the ON/OFF Pin must be tied low for 2 second and then released.
        Telit_ON_Write(0u);
        
        CyDelay(5000u); 
        /* wait for module to turn off
            "Normally it will be above 10 seconds later from releasing
            ON/OFF# and DTE should monitor the status of PWRMON to see the
            actual power off."  */
    }   
        
    
    // Book keeping
    Telit_ControlReg_Write(0u);    
    Telit_RST_Write(0u); // Make sure the RESET "button" is not pressed
    
    modem_state = MODEM_STATE_OFF;
    return 1u;
}

uint8 modem_reset(){
    if (modem_state != MODEM_STATE_OFF) { // The modem is idle/ready (powered on)
        
        Telit_RST_Write(1u);
        CyDelay(500u);  //   To reset and to reboot the module, 
                        // the RESET pin must be tied low for at least 200 milliseconds and then released.
        Telit_RST_Write(0u);
        
        CyDelay(5000u);
        modem_state = MODEM_STATE_IDLE;
    }
    return modem_state;
}

uint8 modem_setup() {
/* Initialize configurations for the modem */
	// Set Error Reports to verbose mode
	if (modem_set_error_reports(2u) != 1u) {
		return 0u;
	}
	
	return 1u;
}

uint8 modem_connect(){
/* Establish modem connection with the network */	
	uint8 count = 0u, network_status = 0u;
	
	// Check if modem is registered on home network
	// Buffer should return +CREG: 0,1
	// modem_check_network returns 1u if buffer contains ",1"
	network_status = modem_check_network();

	while(network_status == 0u && count < 3*max_conn_attempts) {

        // TODO: Investigate how many iterations we need (10/20/2016)
        // Previously, the delay was 3000 ms, but was this loop was 
        // changed to ping the modem more frequently over the same time period
        network_status = modem_check_network();
		CyDelay(1000u);
        count++;

	}
	
	if (network_status == 1u) {		
		// Try to activate network context
		// #SGACT1,<0,1> is for multisocket
		// For now, use #GPRS, which is from Enhanced Easy IP commands
	    if(at_write_command("AT#SGACT=1,1\r","OK",5000u) == 1){    
		//if(at_write_command("AT#GPRS=1\r","OK",5000u) == 1){    // Used for GSM (ATT, TMobile)
	        modem_state = MODEM_STATE_READY;
	        return modem_state;
	    }		
	}
	

    return modem_state;
        
}

uint8 modem_disconnect(){
/* Close modem connection to network */
	
    // Proceed if modem is not connected to network.  Otherwise, try to disconnect from the network and proceed.
    if(modem_state != MODEM_STATE_READY) {
        /* Can use this statement instead for GSM (ATT, TMobile)
		return (at_write_command("AT#GPRS=0\r","OK",5000u) == 1u);
		*/
		return (at_write_command("AT#SGACT=1,0\r","OK",5000u) == 1u);
			
    }
    return 0u; // failed to disconnect
}

uint8 modem_check_network() {
    if(at_write_command("AT+CREG?\r",",1",1000u) == 1u){      
        return 1u;
    }

    return 0u;      
}

uint8 modem_get_meid(char* meid) {
/*
int modem_get_meid(char* meid)

Return the MEID of the cell module
- Tested for CC864-DUAL and DE910-DUAL

Example CC864-DUAL Conversation:
[Board] AT#MEID?
[Modem] #MEID: A10000,32B9F1C0

        OK

Example DE910-DUAL Conversation:
[Board] AT#MEID?
[Modem] #MEID: A1000049AB9082

        OK
*/
    
    // Check for valid response from cellular module
    if(at_write_command("AT#MEID?\r","OK",1000u) == 1u){ 
        // If successful, parse the string received over uart for the meid
        char *a, *b;

        // Expect the UART to contain something like "\r\n#MEID: A10000,32B9F1C0\r\n\r\nOK"
        // - Search for "#MEID:".  Place the starting pointer, a, at the resulting index
        a = strstr(uart_received_string,"#MEID: ");
        if (a == NULL) {
            //puts("#MEID: not found in uart_received_string");
            return 0u;
        }
        // - Shift the pointer to the beginning of the MEID, i.e. "A10000,32B9F1C0\r\n\r\nOK"
        a += strlen("#MEID: ");
        
        // - Find the carriage return marking after "#MEID: " that follows the MEID
        b = strstr(a,"\r");
        if (b == NULL) {
            //puts("Carriage return not found in uart_received_string");
            return 0u;
        }
        
        // Parse the strings and store the MEID in *meid
        strncpy(meid,a,b-a);
        meid[b-a] = '\0';
        
        // In the case for modules like CC864-DUAL where "," is in the middle of the MEID,
        // remove the comma
        // * Assume only 1 comma needs to be removed
        a = strstr(meid,",");
        if (a != NULL) {
            memmove(a,a+1,strlen(meid)-1);
        } 
        
        return 1u;
    }
    
    return 0u;    
}

uint8 modem_check_signal_quality(int *rssi, int *fer){
/* 

uint8 modem_check_signal_quality(uint8 *rssi)

Returns the received signal strength indication (rssi) of the modem
This value ranges from 0-31, or is 99 if unknown/undetectable

Also returns the frame rate error (fer) of the modem.  
This value ranges from 0-7, or is 99 if unknown/undetectable.
(From experience, fer is almost always 99)

Example conversation:
[Board] AT+CSQ
[Modem] +CSQ: 17,99

        OK
*/
    
    // Check for valid response from cellular module
    if(at_write_command("AT+CSQ\r","OK",1000u) == 1u){  
        
        // If successful, parse the string received over uart for the rssi value
        char *a, *b, *c;
        char rssi_str[5] = {'\0'}, fer_str[5] = {'\0'};

        // Expect the UART to contain something like "+CSQ: 17,99\r\n\r\nOK"
        // - Search for "+CSQ: ".  Place the starting pointer, a, at the resulting index
        a = strstr(uart_received_string,"+CSQ: ");
        if (a == NULL) {
            //puts("+CSQ: not found in uart_received_string");
            return 0u;
        }
        // - Shift the pointer to the beginning of the rssi value, i.e. "17,99\r\n\r\n\OK"
        a += strlen("+CSQ: ");
        
        // - Find the comma marking after "+CSQ: " that follows the rssi value
        b = strstr(a,",");
        if (b == NULL) {
            //puts("Comma not found in uart_received_string");
            return 0u;
        }
        

        // - Find the carriage return after the comma that follows the fer value
        c = strstr(b + strlen(","),"\r");
        if (c == NULL) {
            //puts("\r not found in uart_received_string");
            return 0u;
        }
        
        // Parse the strings and store the respective rssi and fer values
        strncpy(rssi_str,a,b-a);
        rssi_str[b-a] = '\0';
        // - Store the rssi value in *rssi
        *rssi = (uint8) strtol(rssi_str,(char **) NULL, 10); // Base 10

        strncpy(fer_str,b+strlen(","),c-b+strlen(","));
        fer_str[c-b+strlen(",")] = '\0';
        // - Store the fer value in *fer
        *fer = (uint8) strtol(fer_str,(char **) NULL, 10); // Base 10

        return 1u;
    }

    return 0u;  
}

int modem_get_socket_status() {
   
    // Check for valid response from cellular module
    if(at_write_command("AT#SS\r","OK",1000u) == 1u){ 
        // If successful, parse the string received over uart for the meid
        char *a, *b;
        char status_str[5];
        int ss;
        
        // Expect the UART to contain something like "\r\n#SS: 1,0\r\n"
        // - Search for "#SS:".  Place the starting pointer, a, at the resulting index
        a = strstr(uart_received_string,"#SS: ");
        if (a == NULL) {
            //puts("#MEID: not found in uart_received_string");
            return 0u;
        }
        // - Shift the pointer to the beginning of the MEID, i.e. "A10000,32B9F1C0\r\n\r\nOK"
        a += strlen("#SS: 1,");
        
        // - Find the carriage return marking after "#MEID: " that follows the MEID
        b = strstr(a,"\r");
        if (b == NULL) {
            //puts("Carriage return not found in uart_received_string");
            return 0u;
        }
        
        // Parse the strings and store the MEID in *meid
        strncpy(status_str,a,b-a);
        status_str[b-a] = '\0';
        ss = (int) strtol(status_str,(char **) NULL, 10);
        return ss;
    }
    
    return -1;    
}

uint8 modem_set_flow_control(uint8 param){
    char cmd[100];
    sprintf(cmd,"AT&K%u\r",param);
    if(at_write_command(cmd,"OK",1000u) == 1u){      
        return 1u;
    }

    return 0u;  
}

uint8 modem_set_error_reports(uint8 param){
	
    char cmd[100];
    sprintf(cmd,"AT+CMEE=%u\r",param);
    if(at_write_command(cmd,"OK",1000u) == 1u){      
        return 1u;
    }

    return 0u;  
}

uint8 modem_socket_dial(char *socket_dial_str, char* endpoint, int port, int construct_new){
	
	if (construct_new){
		// Reset socket dial string if not empty
	    memset(socket_dial_str, '\0', strlen(socket_dial_str));
		sprintf(socket_dial_str, "%s%s%d%s%s%s", socket_dial_str, "AT#SD=1,0,", port, ",\"", endpoint, "\",0,0,1\r\0");
	}
	if( modem_state == MODEM_STATE_READY ){
		// Reset uart for incoming data from modem
        uart_string_reset();
        if(at_write_command(socket_dial_str,"OK",15000u)){      
            return 1u;
        }
	}
    return 0u;  
}

uint8 modem_socket_close(){
    if(at_write_command("AT#SH=1\r","OK",1000u) == 1u){      
        return 1u;
    }
    return 0u;  
}

void construct_generic_request(char* send_str, char* body, char* host, char* route,
                               int port, char* method, char* connection_type,
	                           char *extra_headers, int extra_len, char* http_protocol){

    sprintf(send_str,"%s /%s HTTP/%s\r\n", method, route, http_protocol);
    sprintf(send_str,"%s%s%s%s%d%s%s%s%s",
            send_str, // 1
            "Host: ", host, ":", port, "\r\n", // 2 3 4 5 6
            "Connection: ", connection_type, "\r\n"); // 7 8 9
	if (extra_headers && strlen(extra_headers) > 0){
		sprintf(send_str, "%s%s", send_str, extra_headers);
	}
	if (strcmp(method, "GET") != 0){
		sprintf(send_str, "%s%s%s%d%s%s",
			send_str,
            "Content-Type: text/plain\r\n", // 10
            "Content-Length: ", (extra_len + strlen(body)), //11 12 (Extra len should be 2 for flask server)
            "\r\n\r\n", body); // 13 14 15
	}
	sprintf(send_str, "%s%s", send_str, "\r\n\032"); 
}

uint8 modem_send_recv(char* send_str, char* response, uint8 get_response)
{
    if(at_write_command("AT#SSEND=1\r",">",10000u)){ 
		// Reset uart for incoming data from modem
	    uart_string_reset();
		if(at_write_command(send_str,"SRING: 1",10000u) != 0){
            // Read HTTP response from the buffer
            uart_string_reset();
            uint8 data_pending = at_write_command("AT#SRECV=1,1500\r","SRING: 1",10000u);
            
            // Check the HTTP response for valid status (200 or 204)
            // Create array, "status_code" to temporarily hold the result
            char status_code[5] = {"\0"};
            parse_http_status(uart_received_string, (char*) NULL, status_code, (char*) NULL);

            // InfluxDB sends chunked data, so check again to see if there is an
            // unsolicited message indicated a suspended connection
            //
            // If so, then the query results are stored in the buffer, so issue
            // AT#SRECV to read the buffer
            if (data_pending == 1) {
                uart_string_reset();
                uint8 success = at_write_command("AT#SRECV=1,1500\r","NO CARRIER",10000u);
            }
			if (get_response){
                strcpy(response, uart_received_string);
			}
            
             // return 1 if status code indicates Success, 2xx
            if( status_code[0] == '2' ){
                return 1u;
            }
		}
    }   
    return 0u;  
}

/*
Searches a string "http_status" for and attempts to parse the status line.
Leverages Status-Line protocol for HTTP/1.0 and HTTP/1.1

    Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF

(See https://www.w3.org/Protocols/HTTP/1.0/spec.html#Status-Line.
 SP is a space, CRLF is carriage return line feed)

Stores the results in "version", "status_code", and "phrase"
*/
uint8 parse_http_status(char* http_status, char* version, char* status_code, char* phrase) {
    char *a, *b;
    
    // Find the version and copy to "version"
    a = strstr(http_status,"HTTP/");
    if (a == NULL) {
        return 0u;
    }
    a += strlen("HTTP/");
    b = strstr(a," ");
    if (b == NULL) {
        return 0u;
    }
    strncpy(version,a,b-a);
    
    // Find the status code and copy to "status_code"
    a = b+1;
    b = strstr(a," ");
    if (b == NULL) {
        return 0u;
    }
    strncpy(status_code,a,b-a);
    //*status_code = (int) strtol(tmp_code,(char **) NULL, 10); 
    
    // Find the status phrase and copy to "phrase"
    a = b+1;
    b = strstr(a,"\r\n");
    if (b == NULL) {
        return 0u;
    }
    strncpy(phrase,a,b-a);
    
    return 1u;
}

void uart_string_reset(){
    // reset uart_received_string to zero
    memset(&uart_received_string[0],0,sizeof(uart_received_string));
    uart_string_index = 0;
    Telit_UART_ClearRxBuffer();
}

// this function fires when uart rx receives bytes (when modem is sending bytes)
CY_ISR(Telit_isr_rx){
    while (Telit_UART_GetRxBufferSize()){
        // hold the next char in the rx register as a temporary variable
        char rx_char_hold = Telit_UART_GetChar();
        
        // store the char in uart_received_string
        if(rx_char_hold){
            uart_received_string[uart_string_index] = rx_char_hold;
            uart_string_index++;
        }
    }
}

///* [] END OF FILE */
//
