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
#include "sdcard.h"
//#include "config.h"

// declare variables
int	   iter = 0;
uint8  modem_state, lock_acquired = 0u, ready = 0u;
uint16 uart_string_index = 0u;
uint32 feed_id;
char   modem_received_buffer[MODEM_BUFFER_LENGTH] = {'\0'};
char   modem_time[25u] = {'\0'};
char   request_chunk[CHUNK_SIZE] = {'\0'};
char   modem_apn[50] = "VZWINTERNET";

// modem interrupt declaration
CY_ISR_PROTO(Telit_isr_rx);
void uart_string_reset();

uint8 modem_startup(int *conn_attempts) {
    // Modem is already connected to network
    if (modem_state == MODEM_STATE_READY) {
        return 1u;
    }
    
	// Attempt to connect to network
	for (int i = 0; i < max_conn_attempts; ++i) {
        ++*conn_attempts;
        if (modem_power_on()) {
		    modem_setup();
			modem_connect();
			if (modem_state == MODEM_STATE_READY) {
                return 1u;
            }
		} else {
			modem_reset();
		}
	}
	
	// Failed to connect
	return 0u;
}

uint8 modem_shutdown() {
    modem_disconnect();
	return modem_power_off();
}

// initialize modem
void modem_start(){
    /* Do nothing */
}

// deinitialize modem
void modem_stop(){
    /* Do nothing */
}

uint8 modem_updates_toggle(uint8 updates_enabled){
    /* Do nothing */
    return 1u;
}

uint8 modem_get_time(char *time) {
    /*
    Returns real time clock info from the cell module

    Example Conversation:
    [Board] AT+CCLK?
    [Modem] +CCLK: "19/10/24,00:00:55-00"
    
            OK
    */
    if (at_write_command("AT+CCLK?\r", "OK", 1000u)) {
        strextract(modem_received_buffer, time, "+CCLK: \"", "\"\r\n");
        return time != NULL;
    }

    return 0u;
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
    
    // Back up the modem buffer to the SD card
    uint8 sd_status = 9;
    char debug_label[6] = "MODEM";
    
    sd_status = SD_mkdir(node_id);
    
    sd_status = 9;
    
    //sprintf((char*) SD_filename,"\\%.8s\\%.8s.txt", node_id, debug_label);
    sprintf((char*) SD_filename,"%.8s.txt", debug_label);
    
    sd_status = 10;
    sprintf((char*) SD_body,"%s,%s,%s,%s\r\n",modem_time,debug_label, node_id, uart_string);
    sd_status = SD_write(SD_filename,SD_filemode,SD_body);  
    
    sd_status = 11;
    sprintf((char*) SD_body,"%s,%s,%s,%s\r\n",modem_time,debug_label, node_id, modem_received_buffer);
    sd_status = SD_write(SD_filename,SD_filemode,SD_body);  
    
    return response;
}

uint8 modem_power_on(){
    // Modem is already on
    if (at_write_command("AT\r", "OK", 1000u)) {
        return 1u;
    }
    
    // Initialize UART interrupts
    Telit_UART_Start();
    Telit_isr_rx_StartEx(Telit_isr_rx);
    Telit_ControlReg_Write(1u);
    
    // Start boot sequence
    Telit_PWR_Write(1u);
    Telit_ON_Write(0u);
    CyDelay(100u);
    Telit_ON_Write(1u);
    
    // Wait 30 seconds for boot
    CyDelay(30000u);
	
    // Check modem responds to AT
	if (at_write_command("AT\r", "OK", 1000u)) {    
        modem_state = MODEM_STATE_IDLE;
        return modem_state;
    }
	
	// Failed to turn on
    return 0u;
}

uint8 modem_power_off(){
    // Modem is already off
    if (!at_write_command("AT\r", "OK", 1000u)) {
        return 1u;
    }
    
    // Start shutdown sequence
    at_write_command("AT^SMSO", "OK", 5000u);
    
    // Deinitialize UART interrupts
    Telit_ControlReg_Write(0u);
    Telit_isr_rx_Stop();
    Telit_UART_Stop();
    
    // Book keeping
    Telit_PWR_Write(0u);  
    Telit_RST_Write(0u);
    Telit_ON_Write(0u);
    
    // Set modem state
    modem_state = MODEM_STATE_OFF;
    return 1u;
}

uint8 modem_reset() {
    if (modem_state == MODEM_STATE_OFF) {
        return modem_state;
    }
        
    Telit_RST_Write(1u);
    CyDelay(200u);
    Telit_RST_Write(0u);
    CyDelay(5000u);
    
    modem_state = MODEM_STATE_IDLE;
    return modem_state;
}

uint8 modem_setup() {
    uint8 status = 0u;
    
    // Set APN
    status |= modem_set_apn();
    // Set modem functionality to full
	status |= modem_set_fun(1u) << 1;
	// Set error message format to verbose
	status |= modem_set_error_reports(2u) << 3;
    // Get latest time from modem
    // sometimes unix epoch start instead
    status |= modem_get_time(modem_time) << 4;

	return status;
}

uint8 modem_connect(){
	uint8 registered = 0u;
    
    // Repeatedly check if modem is registered on network
	for (int i = 0u; i < 3 * max_conn_attempts; ++i) {
        if ((registered = modem_check_network())) break;
		CyDelay(1000u);
	}
	
	if (registered && modem_pdp_context_toggle(1u)) {
	    modem_state = MODEM_STATE_READY;
	}

    return modem_state;   
}

uint8 modem_disconnect(){
    return modem_pdp_context_toggle(0u);
}

uint8 modem_check_network() {
    /*
    Returns whether the cell module is registered to its home network
    
    Example Conversation:
    [Board] AT+CEREG?
    [Modem] +CEREG: 0,1
    
            OK
    */
    return at_write_command("AT+CEREG?\r", ",1", 1000u);      
}

uint8 modem_get_meid(char *meid) {
    /*
    Return the MEID of the cell module
    
    Example Conversation:
    [Board] AT+CCID?
    [Modem] +CCID: "A1000049AB9082",""

            OK
    */
    if (at_write_command("AT+CCID?\r", "OK", 1000u)) {
        char *end = strextract(modem_received_buffer, meid, "+CCID: \"", "\",\"");
        return end != NULL;
    }

    return 0u;
}

uint8 modem_check_signal_quality(int *rsrq, int *rsrp) {
    /*
    Returns the reference signal received quality (rsrq)
    This value ranges from 0-34, or 255 if unknown/undetectable

    Also returns the reference signal received power (rsrp)
    This value ranges from 0-97, or 255 if unknown/undetectable

    Example Conversation:
    [Board] AT+CESQ
    [Modem] +CESQ: 99,99,255,255,27,43

            OK
    */
    if (at_write_command("AT+CESQ\r", "OK", 1000u)) {
        char rsrq_str[4] = {'\0'}, rsrp_str[4] = {'\0'};
        char *comma = strrchr(modem_received_buffer, ',');
        *comma = '\0'; strncpy(rsrp_str, comma + 1, 3);
        comma = strrchr(modem_received_buffer, ',');
        strncpy(rsrq_str, comma + 1, 3);

        *rsrq = atoi(rsrq_str);  // "27" in example
        *rsrp = atoi(rsrp_str);  // "43" in example
        return 1u;
    }

    return 0u;
}

int modem_get_socket_status() {
    /*
    Returns the status of internet service profile 3
    
    Example Conversation:
    [Board] AT^SISI?
    [Modem] ^SISI: 3,6,20,3,3,0
    
            OK
    */
    if (at_write_command("AT^SISI?\r", "OK", 1000u)) {
        char status_str[5] = {'\0'};
        strextract(modem_received_buffer, status_str, "^SISI: ", "\r\n");
        return atoi(status_str); // "6" in example
    }

    return -1;
}

uint8 modem_set_apn(){
    char cmd[100];
    sprintf(cmd, "AT+CGDCONT=3,\"IPV4V6\",\"%s\"\r",modem_apn);
    return at_write_command(cmd,"OK",1000u);
}

uint8 modem_set_fun(uint8 param){
    char cmd[100];
    sprintf(cmd, "AT+CFUN=%u\r", param);
    return at_write_command(cmd,"OK",1000u);
}

uint8 modem_set_flow_control(uint8 param){
    /* Do nothing */
}

uint8 modem_set_error_reports(uint8 param){
    char cmd[100];
    sprintf(cmd, "AT+CMEE=%u\r", param);
    return at_write_command(cmd,"OK",1000u);
}

uint8 modem_pdp_context_toggle(uint8 activate_pdp) {
    /*
    Activate/deactivate internet service using PDP context 3
    
    Example Conversation:
    [Board] AT^SICA?
    [Modem] ^SICA: 1,1
            ^SICA: 3,0
    
            OK
    */
    char cmd[20];
    char pdp_state[5] = {'\0'};

    if (at_write_command("AT^SICA?\r", "OK", 1000u)) {
        // PDP context 3 state already matches
        strextract(modem_received_buffer, pdp_state, "^SICA: 3,", "\r\n");
        if (atoi(pdp_state) == activate_pdp) return 1u;
        
        // Set context 3 state to activate_pdp, wait 15 seconds
        sprintf(cmd, "AT^SICA=%u,3\r", activate_pdp);
        return at_write_command(cmd, "OK", 60000u);
    }

    return 0u;
}             

uint8 modem_socket_dial(char *socket_dial_str, char* endpoint, int port, 
                        int construct_new, int ssl_enabled){
	// Create new socket_dial_Str with endpoint and port
	if (construct_new){
	    memset(socket_dial_str, '\0', strlen(socket_dial_str));
		sprintf(socket_dial_str, "AT^SISS=0,\"address\",\"socktcp://%s:%d\"\r", endpoint, port);
	}
    
	if( modem_state == MODEM_STATE_READY ){
        uart_string_reset();
        return (at_write_command("AT^SISS=0,srvType,\"Socket\"\r", "OK", 2000u) &&
                at_write_command("AT^SISS=0,conID,\"3\"\r", "OK", 2000u) &&
                at_write_command(socket_dial_str, "OK", 2000u) &&
                at_write_command("AT^SISO=0\r", "^SISW: 0,1", 30000u));
	}
    return 0u;
}

uint8 modem_socket_close(int ssl_enabled){
    return at_write_command("AT^SISC=0\r", "OK", 1000u);  
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
    
    //char tmp_buffer[4000];
    //sprintf(tmp_buffer, "%s", send_str);
    
    return;
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
        status = at_write_command(send_cmd,"^SISW: ",10000u);
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
            status = at_write_command(chunk, "OK", 10000u);
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
    int data_pending = at_write_command(recv_cmd, "OK", 10000u);
    
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
    /*if (data_pending){
        // Throw out headers for now
        uart_string_reset();
        // Get the next modem buffer
        status = at_write_command(recv_cmd, "OK", 10000u);
        // Set the starting pointer to the modem buffer start
        response_start = modem_received_buffer;
    }*/
    // If no data pending, set the starting pointer
    /*else{
        // If chunked, set the starting pointer right before the first chunk size
        // If fixed, set the starting pointer at the start of the message body
            response_start = strstr(modem_received_buffer, "\r\n\r\n");
    }*/
    response_start = strstr(modem_received_buffer, "\r\n\r\n");

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
        buffer_start = 0;
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
    char send_cmd[25] = {0};
    char recv_cmd[] = "AT^SISR=0,1500\r";
    char ring_cmd[] = "^SISW: 0,1";
    sprintf(send_cmd, "AT^SISW=0,%u\r", strlen(send_str));
    
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
