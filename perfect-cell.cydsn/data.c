#include "data.h"
#include "extern.h"

// Node ID
char node_id[20] = "example_node";

// Default user info
char user[20] = "home_user";
char pass[50] = "home_pass";
char database[20] = "HOME_DB";

// Meta user info
char meta_user[20] = "meta_user";
char meta_pass[50] = "meta_pass";
char meta_database[20] = "META_DB";

// Defaults if service.c not used
int main_port = 8086;
char main_host[100] = "ec2-52-87-156-130.compute-1.amazonaws.com";
char write_route[60] = "";
char main_query[300]= {'\0'};
char main_tags[200] = "source=node";

// Enable/disable SSL
int ssl_enabled = 1u;

// Set service to use here
int service_flag = 1u;

// Sleeptimer
int sleeptimer = 460u; // Number of wakeups before full power: 1172 @ 256ms ~5 min

// Modem
// Number of attempts modem made while trying to establish a connection
// Initialize to -1 to flag when the node has restarted
// (As the code is currently written, this count will always be one reading behind)
int max_conn_attempts = 5;
int connection_attempt_counter = 0;
int rssi = 0u;
int fer  = 0u;

// Flags to activate devices
int modem_flag = 1u;
int meta_flag  = 1u;
int vbat_flag  = 1u;
int ultrasonic_flag   = 0u;
int ultrasonic_2_flag = 0u;
int optical_rain_flag = 1u;
int decagon_flag      = 0u;
int autosampler_flag  = 0u;
int valve_flag   = 0u;
int valve_2_flag = 0u;

// Flags to trigger devices
int autosampler_trigger = 0u;
int valve_trigger = -1;
int valve_2_trigger = -1;
int meta_trigger = 1;

// Number of loops for each device
int vbat_loops = 1;
int ultrasonic_loops = 5;
int optical_rain_loops = 1;
int decagon_loops = 1;

// Number of vars for each device
int modem_vars = 3;
int vbat_vars = 1;
int ultrasonic_vars = 1;
int ultrasonic_2_vars = 1;
int optical_rain_vars = 1;
int decagon_vars = 3;
int autosampler_vars = 2;
int valve_vars = 2;
int meta_vars  = 1;
// Total vars = 15

// Other
uint8 bottle_count = 0u;
int valve = 0;

// Functions

int take_readings(char *labels[], float readings[], uint8 take_average){
    int read_iter = 0;
    int array_ix = 0;

    // Check if the signal strength and number of attempts modem took
    //  to connect during the last transmission is to be reported
    if ( modem_flag == 1u ){
    	labels[array_ix] = "conn_attempts";
    	labels[array_ix + 1] = "cell_strength";
    	labels[array_ix + 2] = "cell_fer";
    	readings[array_ix] = connection_attempt_counter;
    	readings[array_ix + 1] = rssi;
    	readings[array_ix + 2] = fer;
    	array_ix += 3;
    }

    // Check if battery voltage measurement is to be taken
    if ( vbat_flag == 1u ){
        float32 vbat = 9999;
        vbat = read_Vbat();
        labels[array_ix] = "v_bat";
        readings[array_ix] = vbat;
        array_ix++;
    }
    
    // Check if ultrasonic measurement is to be taken
    if ( ultrasonic_flag == 1u ) {
        
        // Start the MUX 
        mux_controller_Wakeup();
        
        // Set MUX to read from 1st input
        mux_controller_Write(0u);
        
        // Add to labels
		labels[array_ix] = "maxbotix_depth";
        UltrasonicReading ultrasonic_reading = {0u, 0u, 0u};
        uint8 valid;
        float valid_iter = 0.0;
        for( read_iter = 0; read_iter < ultrasonic_loops; read_iter++){
            valid = ultrasonic_get_reading(&ultrasonic_reading);
            if ( ultrasonic_reading.valid == 1u){
                valid_iter++;
                readings[array_ix] += ultrasonic_reading.depth;
                // If not taking the average, break the loop at the first valid reading
                if ( take_average == 0u ) {
                    array_ix++;
				    break;
                }
            }            
        }
        // If taking the average, divide by the number of valid readings
        if ( take_average == 1u ) {
			if ( valid_iter > 0 ) {
                readings[array_ix] = readings[array_ix] / valid_iter;
                array_ix++;
			}
        }
        
        // 2017 02 05: Send -1 instead of 9999 to avoid confusion
        // TODO: Test and then check in this update on GitHub
		// If there are no valid readings, send 9999
		if (valid_iter == 0.0) {
			readings[array_ix] = -1;
			array_ix++;
		}
        
        // Save MUX configuration + put MUX to sleep
        mux_controller_Sleep();
        
    }    

    // Check if ultrasonic measurement is to be taken      
    if ( ultrasonic_2_flag == 1u ) {
        
        // Start the MUX
        mux_controller_Wakeup();
        
        // Set MUX to read from 2nd input
        mux_controller_Write(1u);
        
        // Add to labels
		labels[array_ix] = "maxbotix_2_depth";
        
        // Take reading(s)
        UltrasonicReading ultrasonic_2_reading = {0u, 0u, 0u};
        uint8 valid;
        float valid_iter = 0.0;
        for( read_iter = 0; read_iter < ultrasonic_loops; read_iter++){
            valid = ultrasonic_get_reading(&ultrasonic_2_reading);
            if ( ultrasonic_2_reading.valid == 1u){
                valid_iter++;
                readings[array_ix] += ultrasonic_2_reading.depth;
                // If not taking the average, break the loop at the first valid reading
                if ( take_average == 0u ) {
                    array_ix++;
				    break;
                }
            }            
        }
        // If taking the average, divide by the number of valid readings
        if ( take_average == 1u ) {
			if ( valid_iter > 0 ) {
                readings[array_ix] = readings[array_ix] / valid_iter;
                array_ix++;
			}
        }
		// If there are no valid readings, send 9999
		if (valid_iter == 0.0) {
			readings[array_ix] = 9999;
			array_ix++;
		}
        
        // Save MUX configuration + put MUX to sleep
        mux_controller_Sleep();
    }    
    
    // Check if optical rain measurement is to be taken
    if ( optical_rain_flag == 1u ) {
        float optical_rain_reading = 0;
        optical_rain_reading = 0.01 * optical_rain_get_count();
        labels[array_ix] = "hydreon_prcp";
        readings[array_ix] = optical_rain_reading;
		array_ix++;
    }

    // Check if soil moisture measurement is to be taken
    if ( decagon_flag == 1u ) {
		labels[array_ix] = "decagon_soil_conduct";
		labels[array_ix + 1] = "decagon_soil_temp";
		labels[array_ix + 2] = "decagon_soil_dielec";
        DecagonGS3 decagon_reading = {0u, 0u, 0u, 0u, 0u};
        float valid_iter = 0.0;
        for( read_iter = 0; read_iter < decagon_loops; read_iter++){
            decagon_reading = Decagon_Take_Reading();
            if ( decagon_reading.valid == 1u){
                valid_iter++;
                readings[array_ix] += decagon_reading.conductivity;
                readings[array_ix + 1] += decagon_reading.temp;
                readings[array_ix + 2] += decagon_reading.dielectric;
                if ( take_average == 0u ) {
                    array_ix += 3;
				    break;
                    }
                }
            }
        if ( take_average == 1u ) {
			if (valid_iter > 0){
                readings[array_ix] = readings[array_ix] / valid_iter;
                readings[array_ix + 1] = readings[array_ix + 1] / valid_iter;
                readings[array_ix + 2] = readings[array_ix + 2] / valid_iter;
                array_ix += 3;
			}
        }
		if (valid_iter == 0.0) {
			readings[array_ix] = 9999;
			readings[array_ix + 1] = 9999;
			readings[array_ix + 2] = 9999;
			array_ix += 3;
		}
    }
	// Check if autosampler measurement is to be taken
	if ((autosampler_flag == 1u) && (autosampler_trigger > 0)){
		autosampler_trigger = 0u; // Update the value locally
        labels[array_ix] = "autosampler_trigger"; // Update the database
        readings[array_ix] = 0;
		array_ix++;
        
        if (bottle_count < MAX_BOTTLE_COUNT) {
			labels[array_ix] = "isco_bottle";
            autosampler_start();
            autosampler_power_on();                                    
            if (autosampler_take_sample(&bottle_count) ) {
				readings[array_ix] = bottle_count;
			}
			else {
				// Use -1 to flag when a triggered sample failed
                readings[array_ix] = -1;
			}                        
            autosampler_power_off(); 
            autosampler_stop();
			array_ix++;
		}
		else {
            //debug_write("bottle_count >= MAX_BOTTLE_COUNT");
			}
		
	}
	if ((valve_flag == 1u) && (valve_trigger >= 0)){
		
        // Ellsworth does not have a potentiometer installed
        // Simply flip the pins for now and come back to implement
        // a pulse counter
        /*        
        float32 valve_pos;
		valve = move_valve(valve);
		valve_pos = 100. * read_Valve_POS();
		labels[array_ix] = "valve_cmd";
		labels[array_ix + 1] = "valve_pos";
		readings[array_ix] = valve;
		readings[array_ix + 1] = valve_pos;
		array_ix += 2;
        */
        
        // If zero, open the valve completely
        // IMPORTANT: If there is a "null" entry,
        //            intparse_influxdb returns 0
        //            Make sure default is to open the valve
        if (valve_trigger == 0) { 
            Valve_OUT_Write(1u);
            CyDelay(20000u);
            Valve_OUT_Write(0u);
        // Else, if 100, close the valve completely
        } else if(valve_trigger == 100) {
            Valve_IN_Write(1u);
            CyDelay(20000u);
            Valve_IN_Write(0u);
        } else {
            // For now, do nothing for the other cases
        }
        
        // Acknowledge the trigger by updating it to -1
        // -1, and negative values are reserved for actuator response
        labels[array_ix] = "valve_trigger";
        readings[array_ix] = -1;
        array_ix += 1;
	}
    // Similar case for the moment for valve_2
	if ((valve_2_flag == 1u) && (valve_2_trigger >= 0)){

        
        // If zero, open the valve completely
        // IMPORTANT: If there is a "null" entry,
        //            intparse_influxdb returns 0
        //            Make sure default is to open the valve
        if (valve_2_trigger == 0) { 
            Valve_2_OUT_Write(1u);
            CyDelay(20000u);
            Valve_2_OUT_Write(0u);
        // Else, if 100, close the valve completely
        } else if(valve_2_trigger == 100) {
            Valve_2_IN_Write(1u);
            CyDelay(20000u);
            Valve_2_IN_Write(0u);
        } else {
            // For now, do nothing for the other cases
        }
        
        // Acknowledge the trigger by updating it to -1
        // -1, and negative values are reserved for actuator response
        labels[array_ix] = "valve_2_trigger";
        readings[array_ix] = -1;
        array_ix += 1;
	}
    if ( meta_flag == 1u ){
        labels[array_ix] = "meta_trigger";
        readings[array_ix] = meta_trigger;
        array_ix++;
    }
    return array_ix;
}

int update_meta(char* meid, char* send_str, char* response_str){
	// Assumes that send_str and response_str are empty
    
    int result = 0, response_code, true_response_code = 15;
    if (modem_startup(&connection_attempt_counter)) {
        if (modem_socket_dial(send_str, main_host, main_port, 1, ssl_enabled)){
            // Build the GET request
            sprintf(main_query,"query?u=%s&p=%s&db=%s&q="
                    "SELECT last(value) from node_id,node_user,node_pass,node_db "
                    "where meid=\'%s\'",
                    meta_user, meta_pass, meta_database,
                    meid
                    );
        
            char *url_encoded_query = url_encode(main_query);
                    		
    	    construct_generic_request(send_str, "", main_host, url_encoded_query,
                                      main_port, "GET", "Close", "", 0, "1.1");

            if(modem_send_recv(send_str, response_str, 1u, ssl_enabled)){
                memset(send_str, '\0', sizeof(*send_str));
    	
        	    // Update variables
                // - Note: USER, PASSWORD, DATABASE are keywords that result
                //         in parsing errors with InfluxDB
                // - For now, use: u, p, db to store these variables on InfluxDB
                clear_str(node_id); clear_str(user); clear_str(pass); clear_str(database);
                
                // !!!need a failsafe in case the response is null.
                // TODO: Look into bit comparison/assignment &=
    	        response_code += 1*strparse_influxdb(node_id, response_str, "node_id");
                response_code += 2*strparse_influxdb(user, response_str, "node_user"); 
                response_code += 4*strparse_influxdb(pass, response_str, "node_pass"); 
                response_code += 8*strparse_influxdb(database, response_str, "node_db"); 

    		}
            
            // Mandatory free() after using malloc() in url_encode()
            free(url_encoded_query);
            // TODO: This only seems to be resetting the first char of main_query
            memset(main_query, '\0', sizeof(*main_query));
        }
        modem_socket_close(ssl_enabled);
    }
    
    result = response_code - true_response_code;
    if (result == 0) {
        meta_trigger = 0;
        return 1u;
    }
    
    // If we get to here, "result" will be an integer less than 0
    // Based on the value of "result", we should be able to find 
    // which parameter didn't parse
    return result;
}

int update_triggers(char* body, char* send_str, char* response_str){
	
	// Assumes that body, send_str and response_str are empty

    int response_code = 0, true_response_code = 0, result = 0; 
    if (modem_startup(&connection_attempt_counter)) {
        // Dial socket
        if (modem_socket_dial(send_str, main_host, main_port, 1, ssl_enabled)){
            memset(send_str, '\0', sizeof(*send_str));
            memset(response_str, '\0', sizeof(*send_str));
    
            // Determine which triggers to include in the request string
	        if (autosampler_flag){
	            sprintf(body, "%s%s,", body, "autosampler_trigger");
                true_response_code += 2;
	        }
	        if (valve_flag){
	            sprintf(body, "%s%s,", body, "valve_trigger");
                true_response_code += 4;
	        }
            if (valve_2_flag){
                sprintf(body, "%s%s,", body, "valve_2_trigger");
                true_response_code += 8;
            }
            sprintf(body, "%s%s,", body, "meta_trigger");
            true_response_code += 1;
            
            // Remove the trailing "," 
            body[strlen(body)-1] = '\0'; 
            
            // Build the GET request
            sprintf(main_query,"query?u=%s&p=%s&db=%s&q="
                "SELECT last(value) from %s "
                "where node_id=\'%s\'",
                user, pass, database,
                body,
                node_id
                );
            
            // Convert whitespace to %20
            char *url_encoded_query = url_encode(main_query);
            		
            construct_generic_request( send_str, "", main_host, url_encoded_query,
                                  main_port, "GET", "Close", "", 0, "1.1");
                            
            //// url_query[] is used to view strings in the watch while debugging
            
            // MANDATORY free() after using malloc() in url_encode()
            free(url_encoded_query);
            memset(main_query, '\0', sizeof(*main_query));
	
            if(modem_send_recv(send_str, response_str, 1u, ssl_enabled)){
                memset(send_str, '\0', sizeof(*send_str));

                // Update variables
                response_code += 1*intparse_influxdb(&autosampler_trigger, response_str, "autosampler_trigger");
    	        response_code += 2*intparse_influxdb(&valve_trigger, response_str, "valve_trigger");
                response_code += 4*intparse_influxdb(&valve_2_trigger, response_str, "valve_2_trigger");
                
                // TO DO: Check if metatrigger updates to 0 if the response is null from the server 
                // Check if node updates correctly. Currently, params must be defined on db before values are written 
                response_code += 8*intparse_influxdb(&meta_trigger, response_str, "meta_trigger");
	        }
	    }
        modem_socket_close(ssl_enabled);
    }
    
    result = response_code - true_response_code;
    if (result == 0) {
        return 1u;
    }
    
    // If we get to here, "result" will be an integer less than 0
    // Based on the value of "result", we should be able to find 
    // which trigger didn't parse    
    return result;
    
}
							
void update_params(char* body, char* send_str, char* response_str){
	
	// Assumes that body, send_str and response_str are empty    
    
    if (modem_startup(&connection_attempt_counter)) {
	    // Dial socket
	    if (modem_socket_dial(send_str, main_host, main_port, 1, ssl_enabled)){
	        memset(send_str, '\0', sizeof(*send_str));
            memset(response_str, '\0', sizeof(*send_str));

	        // Generate request string
	        sprintf(body, "%s%s,", body, "sleeptimer");
            sprintf(body, "%s%s,", body, "max_conn_attempts");
	        sprintf(body, "%s%s,", body, "vbat_flag");
	        sprintf(body, "%s%s,", body, "ultrasonic_flag");
            sprintf(body, "%s%s,", body, "ultrasonic_2_flag");
	        sprintf(body, "%s%s,", body, "optical_rain_flag");
	        sprintf(body, "%s%s,", body, "decagon_flag");
            sprintf(body, "%s%s,", body, "valve_flag");
            sprintf(body, "%s%s,", body, "valve_2_flag");
            sprintf(body, "%s%s",  body, "autosampler_flag");

            // Build the GET request
            sprintf(main_query,"query?u=%s&p=%s&db=%s&q="
                    "SELECT last(value) from %s "
                    "where node_id=\'%s\'",
                    user, pass, database,
                    body,
                    node_id
                    );
            
            // Convert whitespace to %20
            char *url_encoded_query = url_encode(main_query);
                
	        construct_generic_request(send_str, "", main_host, url_encoded_query,
                                      main_port, "GET", "Close", "", 0, "1.1");
            
            // Mandatory free() after using malloc() in url_encode()
            free(url_encoded_query);
            memset(main_query, '\0', sizeof(*main_query));
                        
	        if(modem_send_recv(send_str, response_str, 1u, ssl_enabled)){
	            //modem_socket_close();
                memset(send_str, '\0', sizeof(*send_str));
                	
	            // Update variables
	            intparse_influxdb(&sleeptimer, response_str, "sleeptimer");
                intparse_influxdb(&max_conn_attempts, response_str, "max_conn_attempts");
        	    intparse_influxdb(&vbat_flag, response_str, "vbat_flag");
        	    intparse_influxdb(&ultrasonic_flag, response_str, "ultrasonic_flag");
                intparse_influxdb(&ultrasonic_2_flag, response_str, "ultrasonic_2_flag");
        	    intparse_influxdb(&optical_rain_flag, response_str, "optical_rain_flag");
        	    intparse_influxdb(&decagon_flag, response_str, "decagon_flag");
                intparse_influxdb(&valve_flag, response_str, "valve_flag");
                intparse_influxdb(&valve_2_flag, response_str, "valve_2_flag");
                intparse_influxdb(&autosampler_flag, response_str, "autosampler_flag");
            }
		}
        modem_socket_close(ssl_enabled);
	}
}

void construct_route(char* route, char* base, char* user, char* pass, char* database){
    memset(route, '\0', sizeof(*route));
    sprintf(route, "%s%s?u=%s&p=%s&db=%s", route, base, user, pass, database); 
}

void construct_default_body(char *data_packet, char *labels[], float readings[],
                        int nvars){
	int i = 0; // iterator through labels and readings
	
	// Default to influxdb line protocol
    for (i = 0; i < nvars; i++){
		if (labels[i] && labels[i][0]){
            sprintf(data_packet, "%s%s,node_id=%s,%s value=%f\n",
                    data_packet, labels[i],
                    node_id, main_tags,
                    readings[i]);
		}
    }
}
						
/* Returns a url-encoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char *url_encode(char *str) {
    char *pstr, *buf = (char*) malloc(strlen(str) * 3 + 1), *pbuf = buf;
    for (pstr = str; *pstr != '\0'; pstr++) {
        if (*pstr == ' ') {
            *pbuf++ = '%', *pbuf++ = '2', *pbuf++ = '0';
        } else {
            *pbuf++ = *pstr;
        }
    }
    *pbuf = '\0';
    return buf;
}                        
/* [] END OF FILE */
