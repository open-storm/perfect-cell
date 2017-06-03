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
int optical_rain_flag = 0u;
int decagon_flag      = 0u;
int autosampler_flag  = 0u;
int valve_flag   = 0u;
int valve_2_flag = 0u;
int atlas_wq_flag = 1u;

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
/*
int modem_vars = 3;
int vbat_vars = 1;
int ultrasonic_vars = 1;
int ultrasonic_2_vars = 1;
int optical_rain_vars = 1;
int decagon_vars = 3;
int autosampler_vars = 2;
int valve_vars = 2;
int meta_vars  = 1;
int wq_vars = 8;
// Total vars = 23
*/

// Other
uint8 bottle_count = 0;
int valve = 0;

// Functions

int take_readings(char *labels[], float readings[], uint8 take_average, uint8 max_size){
    int read_iter = 0;
    uint8 array_ix = 0;

    // Check if the signal strength and number of attempts modem took
    //  to connect during the last transmission is to be reported
    if ( modem_flag == 1u ){
        zip_modem(labels, readings, &array_ix, max_size);
    }

    // Take battery voltage measurement
    if ( vbat_flag == 1u ){
        zip_vbat(labels, readings, &array_ix, max_size);
    }
    
    // Take ultrasonic measurement
    if ( ultrasonic_flag == 1u ) {
        zip_ultrasonic(labels, readings, &array_ix, 0u, take_average, ultrasonic_loops, max_size);
    }    

    // Take ultrasonic 2 measurement     
    if ( ultrasonic_2_flag == 1u ) {
        zip_ultrasonic(labels, readings, &array_ix, 1u, take_average, ultrasonic_loops, max_size);
    }    
    
    // Take optical rain measurement
    if ( optical_rain_flag == 1u ) {
        zip_optical_rain(labels, readings, &array_ix, max_size);
    }

    // Take soil moisture measurement
    if ( decagon_flag == 1u ) {
        zip_decagon(labels, readings, &array_ix, take_average, decagon_loops, max_size);
    }
    
    if (atlas_wq_flag == 1u){
        zip_atlas_wq(labels, readings, &array_ix, max_size);
    }
    
    //// Execute triggers
	// Check if autosampler measurement is to be taken
	if ((autosampler_flag == 1u) && (autosampler_trigger > 0)){
        zip_autosampler(labels, readings, &array_ix, &autosampler_trigger, &bottle_count, max_size);
		
	}
    // TODO: Valve 1 and Valve 2 should probably handled using a mux
	if ((valve_flag == 1u) && (valve_trigger >= 0)){
		zip_valve(labels, readings, &array_ix, &valve_trigger, max_size);
	}
    
    // Similar case for the moment for valve_2
	if ((valve_2_flag == 1u) && (valve_2_trigger >= 0)){
        zip_valve_2(labels, readings, &array_ix, &valve_2_trigger, max_size);
	}
    
    // Report meta updater status
    if ( meta_flag == 1u ){
        zip_meta(labels, readings, &array_ix, max_size);
    }
    return array_ix;
}

uint8 zip_meta(char *labels[], float readings[], uint8 *array_ix, uint8 max_size){
    // Ensure we don't access nonexistent array index
    uint8 nvars = 1;
    if(*array_ix + nvars >= max_size){
        return *array_ix;
    }
    labels[*array_ix] = "meta_trigger";
    readings[*array_ix] = meta_trigger;
    (*array_ix)++;
    return *array_ix;
}

uint8 zip_modem(char *labels[], float readings[], uint8 *array_ix, uint8 max_size){
    // Ensure we don't access nonexistent array index
    uint8 nvars = 3;
    if(*array_ix + nvars >= max_size){
        return *array_ix;
    }
    labels[*array_ix] = "conn_attempts";
    labels[*array_ix + 1] = "cell_strength";
    labels[*array_ix + 2] = "cell_fer";
    readings[*array_ix] = connection_attempt_counter;
    readings[*array_ix + 1] = rssi;
    readings[*array_ix + 2] = fer;
    (*array_ix) += 3;
    return *array_ix;
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
