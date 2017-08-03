/**
 * @file modem.c
 * @brief Implements functions for connecting to the cellular network, and sending/receiving packets.
 * @author Brandon Wong, Matt Bartos and Ivan Mondragon
 * @version TODO
 * @date 2017-06-01
 */
#include <device.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "modem.h"
#include "strlib.h"
#include "extern.h"

// declare variables
int iter = 0;
uint8 modem_state, lock_acquired = 0u, ready = 0u;
static volatile uint16 uart_string_index = 0u;
uint32 feed_id;
volatile char modem_received_buffer[MODEM_BUFFER_LENGTH] = {'\0'};
char request_chunk[CHUNK_SIZE] = {'\0'};
char *modem_apn = "epc.tmobile.com";

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
            char* valid = strstr(modem_received_buffer, expected_response);
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
	    if(modem_pdp_context_toggle(1u)){    
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
		return (modem_pdp_context_toggle(0u));
			
    }
    return 0u; // failed to disconnect
}

uint8 modem_check_network() {
    if(at_write_command("AT+CREG?\r",",1",1000u) == 1u){      
        return 1u;
    }

    return 0u;      
}

uint8 modem_get_meid(char *meid) {
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
    if (at_write_command("AT#MEID?\r", "OK", 1000u) == 1u) {
        // Expect the UART to contain something like
        // "\r\n#MEID: A10000,32B9F1C0\r\n\r\nOK"
        char *terminator =
            strextract(modem_received_buffer, meid, "#MEID: ", "\r\n");

        // In the case for modules like CC864-DUAL where "," is in the middle of
        // the MEID, remove the comma
        // * Assume only 1 comma needs to be removed
        char *comma = strstr(meid, ",");
        if (comma != NULL) {
            // +1 to include the null terminating character of a c string
            memmove(comma, comma + 1, strlen(comma) + 1);
        }

        return terminator != NULL;
    }

    return 0u;
}

uint8 modem_check_signal_quality(int *rssi, int *fer) {
    /*
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
    if (at_write_command("AT+CSQ\r", "OK", 1000u) == 1u) {
        char rssi_str[4] = {'\0'};
        char fer_str[4] = {'\0'};

        // Expect the UART to contain something like "+CSQ: 17,99\r\n\r\nOK"
        char *comma = strextract(modem_received_buffer, rssi_str, "+CSQ: ", ",");
        char *terminator = strextract(comma, fer_str, ",", "\r\n");

        *rssi = atoi(rssi_str);  // Containing "17"
        *fer = atoi(fer_str);   // Containing "99"

        return terminator != NULL;
    }

    return 0u;
}

int modem_get_socket_status() {
    // Check for valid response from cellular module
    if (at_write_command("AT#SS\r", "OK", 1000u) == 1u) {
        char status_str[5] = {'\0'};

        // Expect the UART to contain something like "\r\n#SS: 1,0\r\n"
        strextract(modem_received_buffer, status_str, "#SS: ", "\r\n");

        // status_str should contain something like "1,0"
        // So increment ptr by 2;
        return atoi(status_str + 2);
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

uint8 modem_pdp_context_toggle(uint8 activate_pdp) {
    char cmd[20];
    char pdp_state[5] = {'\0'};

    // Send AT read command to determine if context is already enabled
    if (at_write_command("AT#SGACT?\r", "OK", 1000u)) {
        // Extract current pdp state into pdp_state
        strextract(modem_received_buffer, pdp_state, "SGACT: 1,", "\r\n");

        // If current state matches desired state, do nothing
        if (atoi(pdp_state) != activate_pdp) {
            // Construct AT command
            sprintf(cmd, "AT#SGACT=1,%u\r", activate_pdp);

            // Enable/disable context
            return at_write_command(cmd, "OK", 5000u);
        } else {
            return 1u;
        }
    }

    return 0u;
}                    

uint8 modem_socket_dial(char *socket_dial_str, char* endpoint, int port, 
                        int construct_new, int ssl_enabled){
	
	if (construct_new){
		// Reset socket dial string if not empty
	    memset(socket_dial_str, '\0', strlen(socket_dial_str));
        if (ssl_enabled){
            sprintf(socket_dial_str, "%s%s%d%s%s%s", socket_dial_str, "AT#SSLD=1,",
                port, ",\"", endpoint, "\",0,1,1000\r\0");
        }
        else {
		    sprintf(socket_dial_str, "%s%s%d%s%s%s", socket_dial_str, "AT#SD=1,0,", port, ",\"", endpoint, "\",0,0,1\r\0");
        }
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

uint8 modem_socket_close(int ssl_enabled){
    if (ssl_enabled){
        if(at_write_command("AT#SSLH=1\r","OK",1000u) == 1u){      
        return 1u;
        }   
    }
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
	sprintf(send_str, "%s%s", send_str, "\r\n"); 
}

int send_chunked_request(char* send_str, char *chunk, int chunk_len, char *send_cmd, char *ring_cmd, char *term_char){
    int i;
    int status = 0u;
    char *a;
    char *b;
    
    int str_len = strlen(send_str);
    int n_chunks = str_len/chunk_len + (str_len % chunk_len != 0);
    char *str_end = send_str + str_len;
    
    for (i = 0; i < n_chunks; i++){
        status = at_write_command(send_cmd,">",10000u);
        if ( status == 0u ) {
            return status;
        }
        uart_string_reset();
        memset(chunk, '\0', chunk_len + 1);
        a = send_str + i*chunk_len;
        b = send_str + (i+1)*chunk_len;
        if (b >= str_end){
            // TODO: Reimplement as strncat to avoid unnecessary copying
            strncpy(chunk, a, str_end - a);
            sprintf(chunk, "%s%s", chunk, term_char);
            status = at_write_command(chunk, ring_cmd, 10000u);
            uart_string_reset();
            return status;
        }
        // TODO: Reimplement as strncat to avoid unnecessary copying
        strncpy(chunk, a, b - a);
        sprintf(chunk, "%s%s", chunk, term_char);
        status = at_write_command(chunk, "OK", 10000u);
        uart_string_reset();
        if ( status == 0u ) {
            return status;
        }        
    }
    return status;
}

int read_response(char message[], char *recv_cmd, char *ring_cmd, uint8 get_response, 
                  int max_loops, int max_message_size){

    char *chunked_header = "Transfer-Encoding: chunked";
    char *fixed_length_header = "Content-Length: ";
    int message_free_space = max_message_size - strlen(message);
    int chunk_remainder = 0;
    int buffer_start = 1;
    int chunk_start = 1;
    int chunk_size = 0;
    int status = 0;
    char *chunked = NULL;
    char *fixed_length = NULL;
    char *error = NULL;
    char *message_end = NULL;
    char *ending_buffer = NULL;
    char *response_start = NULL;
    char length_string[CHUNK_STRING_LENGTH] = {'\0'};
    char status_code[5] = {"\0"};
    int iter;
    
    char *a = NULL;
    char *b = NULL;

    // Download the first buffer of data
    uart_string_reset();
    int data_pending = at_write_command(recv_cmd, ring_cmd, 10000u);
    
    // Check the HTTP response for valid status (200 or 204)
    parse_http_status(modem_received_buffer, (char*) NULL, status_code, (char*) NULL);

    // If response is not desired, report whether request was successful
    if ( !get_response){
        if( status_code[0] == '2' ){
            return 1u;
        }
        return 0u;
    }
    // Determine the transfer encoding
    fixed_length = strstr(modem_received_buffer, fixed_length_header);
    chunked = strstr(modem_received_buffer, chunked_header);

    // If neither (or both) transfer encodings are included, abort
    // TODO: This may not always be the case
    //if (!fixed_length == !chunked ){ // consider this logical equivalent
    if ((!fixed_length && !chunked) || (fixed_length && chunked)){
        return 0u;
    }

    // If fixed length, get the content length
    if (fixed_length){
        a = fixed_length + strlen(fixed_length_header);
        b = strstr(a, "\r\n");
        if (!b) {return 0u;}
        // Buffer overflow check
        if ( (b - a) >= CHUNK_STRING_LENGTH){
            return 0u;
        }
        strncpy(length_string, a, b-a);
        chunk_size = (int)strtol(length_string, NULL, 10);
        memset(length_string, '\0', sizeof(length_string));
        chunk_remainder = chunk_size;
    }

    // InfluxDB sends chunked data, so check again to see if there is an
    // unsolicited message indicated a suspended connection
    //
    // If so, then the query results are stored in the buffer, so issue
    // AT#SRECV to read the buffer
    if (data_pending){
        // Throw out headers for now
        uart_string_reset();
        // Get the next modem buffer
        status = at_write_command(recv_cmd, "OK", 10000u);
        // Set the starting pointer to the modem buffer start
        response_start = modem_received_buffer;
    }
    // If no data pending, set the starting pointer
    else{
        // If chunked, set the starting pointer right before the first chunk size
        // If fixed, set the starting pointer at the start of the message body
            response_start = strstr(modem_received_buffer, "\r\n\r\n");
    }

    // Make sure response start is not a null pointer
    if (!response_start){
        return 0u;
    }
    // Set the start and end pointers to the beginning of the message
    a = response_start;
    b = response_start;

    // Iterate until all content is downloaded
    for (iter=0; iter < max_loops; iter++){
        // Check for CMEE error
        error = strstr(response_start, "CMEE ERROR");
        // Get pointer to end of Telit buffer
        message_end = strstr(response_start, "\r\n\r\nOK\r\n");
        // Check if this buffer contains the terminating chunk and get pointer
        ending_buffer = strstr(response_start, "\r\n0\r\n");

        // Break if CMEE error or message truncated
        if (error || !message_end){
            return 0u;
        }

        // Move the beginning pointer to the start of the next line
        a = strstr(b, "\r\n") + strlen("\r\n");
        if (!a) {return 0u;}
        // If we are at the beginning of the Telit buffer, skip another line
        if (buffer_start){
            a = strstr(a, "\r\n") + strlen("\r\n");
            if (!a) {return 0u;}
            // Reset buffer start flag
            buffer_start = 0;
        }
        // Move the end pointer to the end of the line
        b = strstr(a, "\r\n");
        if (!b) {return 0u;}

        // If we're positioned at the start of a new chunk, get the chunk size
        if (chunked){
            if (chunk_start){
                // Buffer overflow check
                if ( (b - a) >= CHUNK_STRING_LENGTH){
                    return 0u;
                }
                strncpy(length_string, a, b-a);
                chunk_size = (int)strtol(length_string, NULL, 16);
                memset(length_string, '\0', sizeof(length_string));
                chunk_remainder = chunk_size;
                // Move the beginning pointer to the start of the next line
                a = b + strlen("\r\n");
                if (!a) {return 0u;}
                // Move the end pointer to the end of the line
                b = strstr(a, "\r\n");
                if (!b) {return 0u;}
                // Reset chunk start flag
                chunk_start = 0;
            }
        }

        // Add the current selection to the message buffer
        // First, check for buffer overflow
        message_free_space = max_message_size - strlen(message);
        if ((b - a) >= message_free_space){
            strncat(message, a, message_free_space);
            return 0u;
        }
        // Concatenate the latest selection to the message
        strncat(message, a, b - a);
        // Compute the remaining bytes in the chunk
        chunk_remainder -= (b - a);

        // If we've reached the terminal chunk, exit the function
        if (!chunk_size){
            // return 1 if status code indicates Success, 2xx
            if( status_code[0] == '2' ){
                return 1u;
            }
            return 0u;
        }

        // Check if current Telit buffer is exhausted
        if (b >= message_end){
            // Make sure that we haven't gone over the end of the buffer
            if (b > message_end){
                return 0u;
            }
        // If exhausted, get the next Telit buffer
        status = at_write_command(recv_cmd, "OK", 10000u);
        // Set the flag to indicate we're at the start of a Telit buffer
        buffer_start = 1;
        // Reset pointers
        response_start = modem_received_buffer;
        a = response_start;
        b = response_start;
        }

        // Check chunk status
        if (chunk_remainder <= 0){
            // Check to see if we've overflowed the end of the chunk
            if (chunk_remainder < 0){
                return 0u;
            }
            // For chunked responses:
            // If chunk is exhausted, set flag to indicate we're at the 
            // start of a new chunk
            if (chunked){
                if (chunk_size != 0){
                    chunk_start = 1;
                }
            }
            // For fixed-length responses:
            // If chunk is exhausted, exit the function
            else if (fixed_length){
                // return 1 if status code indicates Success, 2xx
                if( status_code[0] == '2' ){
                    return 1u;
                }
                return 0u;
            }
        }
    }
    return 0u;
}
                            
uint8 modem_send_recv(char* send_str, char* response, uint8 get_response, int ssl_enabled)
{
    int status = 0u;
    char send_cmd[30] = {"\0"};
    char recv_cmd[30] = {"\0"};
    char ring_cmd[30] = {"\0"};
    
    if (ssl_enabled){
        sprintf(send_cmd, "AT#SSLSEND=1\r");
        // numbytes > 1000 throws a CMEE ERROR: Operation not supported
        sprintf(recv_cmd, "AT#SSLRECV=1,1000\r");
        sprintf(ring_cmd, "SSLSRING: 1");
    }
    else{
        sprintf(send_cmd, "AT#SSEND=1\r");
        sprintf(recv_cmd, "AT#SRECV=1,1500\r");
        sprintf(ring_cmd, "SRING: 1");
    }
    
    // TODO: Should request_chunk be passed in, or global?
    status = send_chunked_request(send_str, request_chunk, CHUNK_SIZE, send_cmd, ring_cmd, "\032");
    
    if( status ){
        status = read_response(response, recv_cmd, ring_cmd, get_response, 100, MAX_RECV_LENGTH);
        return status;
    }
    
    return 0u;  
}

void uart_string_reset(){
    // reset uart_received_string to zero
    memset(&modem_received_buffer[0],0,sizeof(modem_received_buffer));
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
            modem_received_buffer[uart_string_index] = rx_char_hold;
            uart_string_index++;
        }
    }
}

///* [] END OF FILE */
//
