/**
 * @file modem.c
 * @brief Implements functions for connecting to the cellular network, and sending/receiving packets.
 * @author Brandon Wong and Matt Bartos
 * @version TODO
 * @date 2017-06-01
 */
#include <device.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "modem.h"
#include "updater.h"
#include "extern.h"

char client_cert[] = "-----\032\0";
char private_key[] = "-----\032\0";
char server_cert[] = "-----BEGIN CERTIFICATE-----\n"
"MIIEMTCCAxmgAwIBAgIJAM099V95yUdhMA0GCSqGSIb3DQEBBQUAMIGuMQswCQYD\n"
"VQQGEwJVUzELMAkGA1UECAwCTUkxEjAQBgNVBAcMCUFubiBBcmJvcjEfMB0GA1UE\n"
"CgwWVW5pdmVyc2l0eSBvZiBNaWNoaWdhbjEkMCIGA1UECwwbUmVhbC10aW1lIFdh\n"
"dGVyIFN5c3RlbXMgTGFiMRQwEgYDVQQDDAtNYXR0IEJhcnRvczEhMB8GCSqGSIb3\n"
"DQEJARYSbWRiYXJ0b3NAdW1pY2guZWR1MB4XDTE3MDUyOTAzMjAwOVoXDTE4MDIw\n"
"ODAzMjAwOVowga4xCzAJBgNVBAYTAlVTMQswCQYDVQQIDAJNSTESMBAGA1UEBwwJ\n"
"QW5uIEFyYm9yMR8wHQYDVQQKDBZVbml2ZXJzaXR5IG9mIE1pY2hpZ2FuMSQwIgYD\n"
"VQQLDBtSZWFsLXRpbWUgV2F0ZXIgU3lzdGVtcyBMYWIxFDASBgNVBAMMC01hdHQg\n"
"QmFydG9zMSEwHwYJKoZIhvcNAQkBFhJtZGJhcnRvc0B1bWljaC5lZHUwggEiMA0G\n"
"CSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCcdIs+Nt2CwskByoqSUJi9l+H/6y20\n"
"bZQXDu99v69JXCUGltyss5akBPtbQHWq+hVQSwCXphnYVl2ZsqwKdiz4kuEc/GhT\n"
"Ng5XqPRWomWC8x3L0xvblvSqYK90tLz0FmzU8zVq6f/OLlTPJZwAhYC8i0mnqbS0\n"
"KMDvXPA4FfayBhDX9bOUUQos7WoGFQmfT/K/xWlIPmQs2QOFdx6Tp4669JaxnpzZ\n"
"wSWe7EUidblUbOzCQtKb/XeVQfuW2xdXxQQRr740mY+/w2dHVl0132lypP60nUbk\n"
"NEVziu4s/C3Lwfb296t4HUfOg460uyzkdDWDZ6NBtSFRNXDhQjeUSs2pAgMBAAGj\n"
"UDBOMB0GA1UdDgQWBBSJJFBLv/J+ysjgjtHN+FnfTyjIHzAfBgNVHSMEGDAWgBSJ\n"
"JFBLv/J+ysjgjtHN+FnfTyjIHzAMBgNVHRMEBTADAQH/MA0GCSqGSIb3DQEBBQUA\n"
"A4IBAQBLGrM/RZKPXyQ2f6ZFA9vugU3KzVgCmV1Z62Io8jOfq8Mhrf0j6s65Yxoh\n"
"KFqvkrozcG+I44Daz9IZbxU04AsYxhpKN5qO5W2PdS9xOXJWugAciVMrTg510WZd\n"
"JgYRiYdCk4L72GxvdJ4UKnH1N+t6ix0vAT4e6f/CoLQg6CIhQNjOojR9wz6BkpNn\n"
"Ra81H5kG3lfajk+o/KPbP4L6CexDnkWiYrkeKPU6SSC7RJ/KqxDHScBfqtmz9OjT\n"
"xtvtzVID7V7pKFM3j3dON81fLrbDtQtu6XPsBpilfl78rl+hess/tMqnAEHaaqpb\n"
"54+PblnOoiwRzbySIqKde+lDMCVb\n"
"-----END CERTIFICATE-----\n"
"\032\0"; // Make sure to end cert with escape char

// Parameters for SSLSECCFG
uint8 ssid = 1;
uint8 cipher_suite = 0; // Use 0 to allow server to decide
uint8 auth_mode = 1; // 0: No auth, 1: Server, 2: Server/client
uint8 cert_format = 1; //0: DER, 1: PEM

// Parameters for SSLCFG
uint8 cid = 1u;
uint packet_size = 1000u;
uint max_to = 90u;
uint def_to = 100u;
uint tx_to = 50u;
uint8 ssl_ring_mode = 1u;

// declare variables
int	   iter = 0;
uint8  modem_state, lock_acquired = 0u, ready = 0u;
uint16 uart_string_index = 0u;
uint32 feed_id;
char   uart_received_string[1500] = {'\0'};
char   request_chunk[CHUNK_SIZE] = {'\0'};
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
            strextract(uart_received_string, meid, "#MEID: ", "\r\n");

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
        char *comma = strextract(uart_received_string, rssi_str, "+CSQ: ", ",");
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
        strextract(uart_received_string, status_str, "#SS: ", "\r\n");

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
        strextract(uart_received_string, pdp_state, "SGACT: 1,", "\r\n");

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

uint8 modem_ssl_toggle(int enable_ssl) {
    char cmd[20];
    char ssl_state[5] = {'\0'};

    // Send AT read command to determine if SSL is already enabled
    if (at_write_command("AT#SSLEN?\r", "OK", 1000u)) {
        // Extract current ssl state into ssl_state
        strextract(uart_received_string, ssl_state, "SSLEN: 1,", "\r\n");

        // If current state matches desired state, do nothing
        if (atoi(ssl_state) != enable_ssl) {
            // Construct AT command
            sprintf(cmd, "AT#SSLEN=1,%u\r", enable_ssl);

            // Enable/disable SSL
            return at_write_command(cmd, "OK", 1000u);
        } else {
            return 1u;
        }
    }

    return 0u;
}

uint8 modem_gps_power_toggle(uint8 gps_power_on) {
    char cmd[20];
    char gps_power[2] = {'\0'};

    // Send AT read command to determine if GPS is already powered
    if (at_write_command("AT$GPSP?\r", "OK", 1000u)) {
        // Extract current gps state into gps_power
        strextract(uart_received_string, gps_power, "GPSP: ", "\r\n");

        // If current state matches desired state, do nothing
        if (atoi(gps_power) != gps_power_on) {
            // Construct AT command
            sprintf(cmd, "AT$GPSP=%u\r", gps_power_on);

            // Enable/disable GPS power
            return at_write_command(cmd, "OK", 5000u);
        } else {
            return 1u;
        }
    }

    return 0u;
}

uint8 modem_get_gps_position(float *lat, float *lon, float *hdop, 
              float *altitude, uint8 *gps_fix, float *cog, 
              float *spkm, float *spkn, uint8 *nsat, uint8 min_satellites, uint8 max_tries){
    uint8 status;
    int gps_iter;
    
        for(gps_iter=0; gps_iter < max_tries; gps_iter++){
            status = at_write_command("AT$GPSACP\r", "OK", 10000u);
            if (status){
                gps_parse(uart_received_string, lat, lon, hdop, altitude, gps_fix, cog, spkm, spkn, nsat);
                clear_str(uart_received_string);
                if (*nsat >= min_satellites){
                    return 1u;
                }
                CyDelay(5000u);
            }
        }
    return 0u;
}
                            
uint8 gps_parse(char *gps_string, float *lat, float *lon, float *hdop, 
              float *altitude, uint8 *gps_fix, float *cog, 
              float *spkm, float *spkn, uint8 *nsat){
    
    char substring[100];
    const char delim[2] = ",";
    char *output_array[11];
    int i;
    char latdeg[3] = {'\0'};
    char latmin[8] = {'\0'};
    char londeg[4] = {'\0'};
    char lonmin[8] = {'\0'};
    
    if (strextract(gps_string, substring, "GPSACP: ", "\r\n")){
        output_array[0] = strtok(substring, delim);
        
        if(output_array[0] == NULL){
            return 0u;
        }
        
        for(i=1; i<11; i++){
            output_array[i] = strtok(NULL, delim);
            if (output_array[i] == NULL){
                return 0u;
            }
        }

        strncpy(latdeg, output_array[1], 2);
        strncpy(latmin, output_array[1] + 2, 7);
        strncpy(londeg, output_array[2], 3);
        strncpy(lonmin, output_array[2] + 3, 7);
        // STILL NEED TO ACCOUNT FOR W/E and N/S
        
        *lat = strtof(latdeg, NULL) + (strtof(latmin, NULL) / 60.0);
        *lon = strtof(londeg, NULL) + (strtof(lonmin, NULL) / 60.0);
        *hdop = strtof(output_array[3], NULL);
        *altitude = strtof(output_array[4], NULL);
        *gps_fix = strtod(output_array[5], NULL);
        *cog = strtof(output_array[6], NULL);
        *spkm = strtof(output_array[7], NULL);
        *spkn = strtof(output_array[8], NULL);
        *nsat = strtod(output_array[10], NULL);
        if (strstr(output_array[1], "S") != NULL){
            (*lat) *= -1.0;
        }
        if (strstr(output_array[2], "W") != NULL){
            (*lon) *= -1.0;
        }        
    return 1u;
    }
    return 0u;
}

uint8 run_gps_routine(int *gps_trigger, float *lat, float *lon, float *hdop, 
              float *altitude, uint8 *gps_fix, float *cog, 
              float *spkm, float *spkn, uint8 *nsat, uint8 min_satellites, uint8 max_tries){

    uint8 status;
    // Make sure you start with power to GPS off
    modem_gps_power_toggle(0u);    
    // Unlock GPS
    status = at_write_command("AT$GPSLOCK=0\r", "OK", 1000u);
    if (!status){ return 0u;}
    CyDelay(100u);
    // Set antenna path to GPS
    status = at_write_command("AT$GPSAT=1\r", "OK", 1000u);
    if (!status){ return 0u;}
    // Turn on power to GPS
    modem_gps_power_toggle(1u);
    CyDelay(10000u);
    // Get GPS Position
    status = modem_get_gps_position(lat, lon, hdop, 
                                    altitude, gps_fix, cog, 
                                    spkm, spkn, nsat, min_satellites, max_tries);
    // Turn off power to GPS
    modem_gps_power_toggle(0u);
    // Reset GPS Settings
    at_write_command("AT$GPSRST\r", "OK", 1000u);
    // For now, always shut gps trigger off
    (*gps_trigger) = 0u;
    return status;
}

uint8 zip_gps(char *labels[], float readings[], uint8 *array_ix,
              int *gps_trigger, uint8 min_satellites, uint8 max_tries,
              uint8 max_size) {
    // Ensure we don't access nonexistent array index
    uint8 nvars = 10;
    if (*array_ix + nvars >= max_size) {
        return *array_ix;
    }

    float lat = -9999;
    float lon = -9999;
    float hdop = -9999;
    float altitude = -9999;
    uint8 gps_fix = 0u;
    float cog = -9999;
    float spkm = -9999;
    float spkn = -9999;
    uint8 nsat = 0u;

    // with the begin/end paradigm, end must always be `one past the end`
    char **begins = labels + *array_ix;
    char **ends = begins + nvars;
    zips(begins, ends, "gps_latitude", "gps_longitude", "gps_hdop",
         "gps_altitude", "gps_fix", "gps_cog", "gps_spkm", "gps_spkn",
         "gps_nsat", "gps_trigger");

    run_gps_routine(gps_trigger, &lat, &lon, &hdop, &altitude, &gps_fix, &cog,
                    &spkm, &spkn, &nsat, min_satellites, max_tries);

    // with the begin/end paradigm, end must always be `one past the end`
    float *beginf = readings + *array_ix;
    float *endf = beginf + nvars;
    zipf(beginf, endf, lat, lon, hdop, altitude, gps_fix, cog, spkm, spkn, nsat,
         *gps_trigger);

    (*array_ix) += nvars;

    return *array_ix;
}

uint8 modem_ssl_sec_data(uint8 ssid, uint8 action, uint8 datatype, 
                         char *cert, char *output_str){
    char at_command[100] = {'\0'};
    int certsize;
    // Construct common portion of AT command
    sprintf(at_command, "AT#SSLSECDATA=%u,%u,%u", ssid, action, datatype);
    // Delete mode
    if (action == 0u){
        sprintf(at_command, "%s\r", at_command);
        if (at_write_command(at_command,"OK",1000u)){
            return 1u;
        }
    }
    // Read mode
    else if (action == 2u){
        sprintf(at_command, "%s\r", at_command);
        if (at_write_command(at_command,"OK",1000u)){
            strextract(uart_received_string, output_str, "SSLSECDATA: ", "\r\n");
            return 1u;
        }
    }
    // Write mode
    else if (action == 1u){
        // Why is this showing 4?
        //certsize = sizeof(cert);
        certsize = strlen(cert) - 1;
        sprintf(at_command, "%s,%d\r", at_command, certsize);
        if (at_write_command(at_command,">",1000u)){
            uart_string_reset();
		    if(at_write_command(cert,"OK",10000u)){
                return 1u;
            }
        }
    }
    return 0u;
}
                        
uint8 modem_ssl_sec_config(uint8 ssid, uint8 cipher_suite, uint8 auth_mode,
                           uint8 cert_format){
    char at_command[100] = {'\0'};
    // Construct AT command
    sprintf(at_command, "AT#SSLSECCFG=%u,%u,%u,%u\r", ssid, cipher_suite,
            auth_mode, cert_format);
    if (at_write_command(at_command,"OK",1000u)){
            return 1u;
        }
    return 0u;    
}
                        
uint8 modem_ssl_config(uint8 ssid, uint8 cid, int packet_size,
                           int max_to, int def_to, int tx_to, uint8 ssl_ring_mode){
    char at_command[100] = {'\0'};
    // Construct AT command
    sprintf(at_command, "AT#SSLCFG=%u,%u,%u,%u,%u,%u,%u\r", ssid, cid,
            packet_size, max_to, def_to, tx_to, ssl_ring_mode);
    if (at_write_command(at_command,"OK",1000u)){
            return 1u;
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
            strncpy(chunk, a, str_end - a);
            sprintf(chunk, "%s%s", chunk, term_char);
            status = at_write_command(chunk, ring_cmd, 10000u);
            uart_string_reset();
            return status;
        }
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
        // Read HTTP response from the buffer
        uart_string_reset();
        uint8 data_pending = at_write_command(recv_cmd,ring_cmd,10000u);
            
        // Check the HTTP response for valid status (200 or 204)
        // Create array, "status_code" to temporarily hold the result
        char status_code[5] = {"\0"};
        parse_http_status(uart_received_string, (char*) NULL, status_code, (char*) NULL);

        // TODO: Generalize this for N chunks
        // TODO: Parse for content length
        // InfluxDB sends chunked data, so check again to see if there is an
        // unsolicited message indicated a suspended connection
        //
        // If so, then the query results are stored in the buffer, so issue
        // AT#SRECV to read the buffer
        if (data_pending == 1) {
            uart_string_reset();
            status = at_write_command(recv_cmd,"NO CARRIER",10000u);
        }
		if (get_response){
            strcpy(response, uart_received_string);
		}
            
        // return 1 if status code indicates Success, 2xx
        if( status_code[0] == '2' ){
            return 1u;
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
    /*
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
    */

    char *space = strextract(http_status, version, "HTTP/", " ");
    char *next_space = NULL;
    char *terminator = NULL;

    if (space != NULL) {
        next_space = strextract(space, status_code, " ", " ");
    }

    if (next_space != NULL) {
        terminator = strextract(next_space, phrase, " ", "\r\n");
    }

    return terminator != NULL;
}

void uart_string_reset(){
    // reset uart_received_string to zero
    memset(&uart_received_string[0],0,sizeof(uart_received_string));
    uart_string_index = 0;
    Telit_UART_ClearRxBuffer();
}

uint8 ssl_init(uint8 edit_ssl_sec_config, uint8 edit_ssl_config){
    int response_code = 0;
    // Enable SSL
    if(modem_ssl_toggle(1u)){
        // Edit ssl security settings (SSLSECCFG) if desired
        if(edit_ssl_sec_config){
            response_code = modem_ssl_sec_config(ssid, cipher_suite, 
                                auth_mode, cert_format);
            if (!response_code){
                return 0u;
            }
        }
        // Edit general ssl configuration (SSLCFG) if desired
        if (edit_ssl_config){
            response_code = modem_ssl_config(ssid, cid, packet_size, max_to, 
                             def_to, tx_to, ssl_ring_mode);
            if (!response_code){
                return 0u;
            }                            
        }       
        // If authorization enabled, store cert data
        if (auth_mode){
            response_code = 0;
            // Delete existing data
            response_code += modem_ssl_sec_data(ssid, 0, 1, (char*) NULL, (char*) NULL);
            // Write certificate to NVM
            response_code += 2*modem_ssl_sec_data(ssid, 1, 1, server_cert, (char*) NULL);
            if (response_code != 3){
                return 0u;
            }
            if (auth_mode == 2u){
                // Delete existing data
                response_code += 4*modem_ssl_sec_data(ssid, 0, 0, (char*) NULL, (char*) NULL);
                response_code += 8*modem_ssl_sec_data(ssid, 0, 2, (char*) NULL, (char*) NULL);
                // Write certificate to NVM
                response_code += 16*modem_ssl_sec_data(ssid, 1, 0, client_cert, (char*) NULL);
                response_code += 32*modem_ssl_sec_data(ssid, 1, 2, private_key, (char*) NULL);
                if (response_code != 63){
                    return 0u;
                }
            }
        }
        return 1u;
    }
    return 0u;
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
