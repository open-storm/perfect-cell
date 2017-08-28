/**
 * @file ssl.c
 * @brief Implements functions for configuring SSL/TLS
 * @author Matt Bartos and Ivan Mondragon
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
#include "ssl.h"

// Certificates
char client_cert[] = CLIENT_CERT;
char private_key[] = PRIVATE_KEY;
char server_cert[] = SERVER_CERT;

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


uint8 modem_ssl_toggle(int enable_ssl) {
    char cmd[20];
    char ssl_state[5] = {'\0'};

    // Send AT read command to determine if SSL is already enabled
    if (at_write_command("AT#SSLEN?\r", "OK", 1000u)) {
        // Extract current ssl state into ssl_state
        strextract(modem_received_buffer, ssl_state, "SSLEN: 1,", "\r\n");

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
            strextract(modem_received_buffer, output_str, "SSLSECDATA: ", "\r\n");
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

/* [] END OF FILE */
