/**
 * @file data.c
 * @brief Implements the sensor subroutine, metadata updater, trigger updater, and parameter updater.
 * @author Matt Bartos and Brandon Wong
 * @version TODO
 * @date 2017-06-01
 */
#include "data.h"
#include "gps.h"
#include "extern.h"
#include "config.h"

#if (USE_INFLUXDB && USE_CHORDS)
    #error Must select only one of USE_INFLUXDB, USE_CHORDS
#endif

#if USE_INFLUXDB
    #include "influxdb.h"
    // Node ID
    char node_id[20] = DEFAULT_NODE_ID;

    // Default user info
    char user[20] = DEFAULT_HOME_USER;
    char pass[50] = DEFAULT_HOME_PASS;
    char database[20] = DEFAULT_HOME_DB;

    // Meta user info
    char meta_user[20] = DEFAULT_META_USER;
    char meta_pass[50] = DEFAULT_META_PASS;
    char meta_database[20] = DEFAULT_META_DB;
    
    int main_port = DEFAULT_HOME_PORT;
    char main_host[100] = DEFAULT_HOME_HOST;
    char main_tags[200] = DEFAULT_GLOBAL_TAGS;
#endif

#if USE_CHORDS
    #include "chords.h"
    int main_port = CHORDS_PORT;
    char main_host[100] = CHORDS_HOST;
    char main_tags[200] = "";
    int chords_instrument_id = CHORDS_INSTRUMENT_ID;
    int chords_write_key_enabled = CHORDS_WRITE_KEY_ENABLED; 
    char *chords_write_key = CHORDS_WRITE_KEY;
    int chords_is_test = CHORDS_IS_TEST;
#endif

char write_route[MAX_ROUTE_SIZE] = "";
char main_query[MAX_QUERY_SIZE]= {'\0'};

// Sleeptimer
int sleeptimer = SLEEPTIMER; // Number of wakeups before full power: 1172 @ 256ms ~5 min

// Modem
// Number of attempts modem made while trying to establish a connection
// Initialize to -1 to flag when the node has restarted
// (As the code is currently written, this count will always be one reading behind)
int max_conn_attempts = MAX_CONNECTION_ATTEMPTS;
uint8 enable_ssl_config     = ENABLE_SSL_CONFIG;
uint8 enable_ssl_sec_config = ENABLE_SSL_SEC_CONFIG;

// Enable/disable SSL
uint8 ssl_enabled = SSL_ENABLED;

// Flags to activate devices
int connection_flag = CONNECTION_FLAG;
int trigger_flag = TRIGGER_FLAG;
int param_flag = PARAM_FLAG;
int meta_flag  = META_FLAG;
int modem_flag = MODEM_FLAG;
int vbat_flag  = VBAT_FLAG;
int gps_flag = GPS_FLAG;
int ultrasonic_flag   = ULTRASONIC_FLAG;
int ultrasonic_2_flag = ULTRASONIC_2_FLAG;
int senix_flag = SENIX_FLAG;
int optical_rain_flag = OPTICAL_RAIN_FLAG;
int decagon_flag      = DECAGON_FLAG;
int autosampler_flag  = AUTOSAMPLER_FLAG;
int valve_flag   = VALVE_FLAG;
int valve_2_flag = VALVE_2_FLAG;
int atlas_wq_flag = ATLAS_WQ_FLAG;
uint SDI12_flag = SDI12_FLAG;

// Flags to trigger devices
int autosampler_trigger = AUTOSAMPLER_TRIGGER;
int valve_trigger = VALVE_TRIGGER;
int valve_2_trigger = VALVE_2_TRIGGER;
int meta_trigger = META_TRIGGER;
int gps_trigger = GPS_TRIGGER;

// Number of loops for each device
int vbat_loops = VBAT_LOOPS;
int ultrasonic_loops = ULTRASONIC_LOOPS;
int optical_rain_loops = OPTICAL_RAIN_LOOPS;
int decagon_loops = DECAGON_LOOPS;

// Other
int connection_attempt_counter = 0;
int rssi = 0u;
int fer  = 0u;
uint8 bottle_count = 0;
int valve = 0;

// Array index
uint8 array_ix = 0u;

// Functions

int take_readings(char* labels[], float readings[], uint8* array_ix,
                  uint8 take_average, uint8 max_size) {
    /*
    // Check if the signal strength and number of attempts modem took
    //  to connect during the last transmission is to be reported
    if ( modem_flag == 1u ){
        zip_modem(labels, readings, array_ix, max_size);
    }
    */

    // Take battery voltage measurement
    if (vbat_flag == 1u) {
        zip_vbat(labels, readings, array_ix, max_size);
    }

    // Take ultrasonic measurement
    if (ultrasonic_flag == 1u) {
        zip_ultrasonic(labels, readings, array_ix, 0u, take_average,
                       ultrasonic_loops, max_size);
    }

    // Take ultrasonic 2 measurement
    if (ultrasonic_2_flag == 1u) {
        zip_ultrasonic(labels, readings, array_ix, 1u, take_average,
                       ultrasonic_loops, max_size);
    }

    // Take toughsonic measurement
    if (senix_flag == 1u) {
        zip_ultrasonic(labels, readings, array_ix, 2u, take_average,
                       ultrasonic_loops, max_size);
    }

    // Take optical rain measurement
    if (optical_rain_flag == 1u) {
        zip_optical_rain(labels, readings, array_ix, max_size);
    }

    // Take soil moisture measurement
    if (decagon_flag == 1u) {
        zip_decagon(labels, readings, array_ix, take_average, decagon_loops,
                    max_size);
    }

    // Take water quality measurement
    if (atlas_wq_flag == 1u) {
        zip_atlas_wq(labels, readings, array_ix, max_size);
    }
    
    // Take SDI-12 measurements
    // In the future, SDI_flag can be used to say which addresses to read from
    //   by assigning each bit to an address (e.g. '0' = 0b1 = 1, '9' = 0b0100000000 = 512, 'a' = 0b100000000 = 1024
    //   and to read from all three, SDI12_flag would be 1537 = 0b11000000001 = 1 + 512 + 1024)
    //   Then, each bit would serve as a "flag" for each SDI12 sensor
    // In effect, one could blindly call zip_SDI12() -- if SDI12_flag is 0, it should just go
    //   through the function without taking measurements from any sensors
    if (SDI12_flag > 0u) {
        zip_SDI12(labels, readings, array_ix, max_size, SDI12_flag);
        //zip_SDI12(labels, readings, array_ix, max_size, SDI12_flag);
    }
    return (*array_ix);
}

uint8 execute_triggers(char *labels[], float readings[], uint8 *array_ix, uint8 max_size){
    #if USE_INFLUXDB
    //// Execute triggers
	// Check if autosampler measurement is to be taken
	if ((autosampler_flag == 1u) && (autosampler_trigger > 0)){
        zip_autosampler(labels, readings, array_ix, &autosampler_trigger, &bottle_count, max_size);
		
	}
    // TODO: Valve 1 and Valve 2 should probably handled using a mux
	if ((valve_flag == 1u) && (valve_trigger >= 0)){
		zip_valve(labels, readings, array_ix, &valve_trigger, max_size);
	}
    
    // Similar case for the moment for valve_2
	if ((valve_2_flag == 1u) && (valve_2_trigger >= 0)){
        zip_valve_2(labels, readings, array_ix, &valve_2_trigger, max_size);
	}
    
    if ((gps_flag == 1u) && (gps_trigger == 1u)){
        zip_gps(labels, readings, array_ix, &gps_trigger, MIN_SATELLITES, MAX_GPS_TRIES, max_size);
    }
    
    // Report meta updater status
    if ( meta_flag == 1u ){
        zip_meta(labels, readings, array_ix, max_size);
    }
    #endif
    return (*array_ix);
}

uint8 zip_meta(char *labels[], float readings[], uint8 *array_ix, uint8 max_size){
    // Ensure we don't access nonexistent array index
    uint8 nvars = 1;
    if(*array_ix + nvars >= max_size){
        return *array_ix;
    }
    labels[*array_ix] = "meta_trigger";
    readings[*array_ix] = meta_trigger;
    (*array_ix) += nvars;
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
    (*array_ix) += nvars;
    return *array_ix;
}

uint8 send_readings(char* body, char* send_str, char* response_str, char* socket_dial_str,
                    char *labels[], float readings[], uint8 nvars){
    uint8 data_sent = 0u;
    #if USE_INFLUXDB
	// Construct the data body
    construct_influxdb_body(body, labels, readings, nvars);
	// Construct POST request
	construct_generic_request(send_str, body, main_host, write_route,
		                      main_port, "POST", "Close",
				              "", 0, "1.1");
    #endif
    
    #if USE_CHORDS
        construct_chords_route(write_route, labels, readings,
                        nvars, chords_instrument_id, chords_write_key_enabled, 
                        chords_write_key, chords_is_test);
        construct_generic_request(send_str, body, main_host, write_route,
                               main_port, "GET", "Close",
	                           "", 0, "1.1");
    #endif
				
    // This sends the data
    modem_socket_dial(socket_dial_str, main_host, main_port, 1, ssl_enabled);
	data_sent = modem_send_recv(send_str, response_str, 0, ssl_enabled);
    modem_socket_close(ssl_enabled);
    return data_sent;
}

uint8 run_meta_subroutine(char* meid, char* send_str, char* response_str, uint8 get_device_meid){
    #if USE_INFLUXDB    
    uint8 status;
    // Only run if meta_trigger and meta_flag are both active
    if ((meta_trigger == 0u) || (meta_flag == 0u)) {
        return 0u;
    }
    if (modem_startup(&connection_attempt_counter)) {
        if (get_device_meid){
            modem_get_meid(meid);
        }
        status = update_meta(meid, send_str, response_str);
        if (status == 1u){
            construct_route(write_route, "write", user, pass, database);
            return 1u;
        }
    }
    #endif
return 0u;
}

int update_meta(char* meid, char* send_str, char* response_str){
    int result = 0;
    #if USE_INFLUXDB
	if (meta_flag == 0u){
        return 0u;
    }
	// Assumes that send_str and response_str are empty        
    int response_code = 0;
    int true_response_code = 15;
    if (modem_startup(&connection_attempt_counter)) {
        if (modem_socket_dial(send_str, main_host, main_port, 1, ssl_enabled)){
            // Build the GET request
            sprintf(main_query,"query?u=%s&p=%s&db=%s&q="
                    "SELECT last(value) from node_id,node_user,node_pass,node_db "
                    "where meid=\'%s\' AND time < now()",
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
    #endif
    return result;
}

int update_triggers(char* body, char* send_str, char* response_str){
    int result = 0u;
    #if USE_INFLUXDB
	if (trigger_flag == 0u){
        return 0u;
    }
	// Assumes that body, send_str and response_str are empty
    int response_code = 0, true_response_code = 0; 
    if (modem_startup(&connection_attempt_counter)) {
        // Dial socket
        if (modem_socket_dial(send_str, main_host, main_port, 1, ssl_enabled)){
            memset(send_str, '\0', sizeof(*send_str));
            memset(response_str, '\0', sizeof(*send_str));
    
            // Determine which triggers to include in the request string
            if (meta_flag){
                sprintf(body, "%s%s,", body, "meta_trigger");
                true_response_code += 1;
            }
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
            if (gps_flag){
                sprintf(body, "%s%s,", body, "gps_trigger");
                true_response_code += 16;
            }

            
            // Remove the trailing "," 
            body[strlen(body)-1] = '\0'; 
            
            // Build the GET request
            sprintf(main_query,"query?u=%s&p=%s&db=%s&q="
                "SELECT last(value) from %s "
                "where node_id=\'%s\' AND time < now()",
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
                response_code += 1*intparse_influxdb(&meta_trigger, response_str, "meta_trigger");
                response_code += 2*intparse_influxdb(&autosampler_trigger, response_str, "autosampler_trigger");
    	        response_code += 4*intparse_influxdb(&valve_trigger, response_str, "valve_trigger");
                response_code += 8*intparse_influxdb(&valve_2_trigger, response_str, "valve_2_trigger");
                response_code += 16*intparse_influxdb(&gps_trigger, response_str, "gps_trigger");
                // TO DO: Check if metatrigger updates to 0 if the response is null from the server 
                // Check if node updates correctly. Currently, params must be defined on db before values are written 
                
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
    #endif
    return result;
}
							
int update_params(char* body, char* send_str, char* response_str){
    #if USE_INFLUXDB
	if (param_flag == 0u){
        return 0u;
    }	
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
            sprintf(body, "%s%s,",  body, "autosampler_flag");
            sprintf(body, "%s%s,",  body, "atlas_wq_flag");
            sprintf(body, "%s%s",  body, "senix_flag");

            // Build the GET request
            sprintf(main_query,"query?u=%s&p=%s&db=%s&q="
                    "SELECT last(value) from %s "
                    "where node_id=\'%s\' AND time < now()",
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
                intparse_influxdb(&atlas_wq_flag, response_str, "atlas_wq_flag");
                intparse_influxdb(&senix_flag, response_str, "senix_flag");
            }
		}
        modem_socket_close(ssl_enabled);
	}
    #endif
    return 1u;
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
