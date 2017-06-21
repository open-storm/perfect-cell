/**
 * @file services.h
 * @brief Declares functions for sending requests to various services including Xively, Thingspeak and CHORDS
 * @author Matt Bartos and Brandon Wong
 * @version TODO
 * @date 2017-06-19
 */

// Xively parameters
extern int XIVELY_FEED_ID;
extern const char *xively_api_key;
extern const char *xively_endpoint;
extern const char *xively_port;

// Influxdb parameters
extern int INFLUXDB_AUTHENTICATION;
extern const char *influxdb_endpoint;
extern const char *influxdb_port;
extern const char *influxdb_db;
extern const char *influxdb_user;
extern const char *influxdb_pw;

// Thingspeak parameters
extern const char *thingspeak_endpoint;
extern const char *thingspeak_port;
extern const char *thingspeak_api_key;

// Chords parameters
extern const char *chords_endpoint;
extern const char *chords_port;
extern const char *chords_write_key;

/**
 * @brief Constructs body of POST request for various services
 *
 * @param data_packet Empty buffer to store POST request body
 * @param labels Array to store labels corresponding to each trigger result
 * @param readings Array to store trigger results as floating point values
 * @param nvars Maximum size of label and reading arrays (number of entries)
 * @param service_flag Service to use:
 * - 1: Xively
 * - 2: Influxdb
 * - 3: Thingspeak
 * - 4: CHORDS
 *
 * @return number of entries filled
 */
int construct_data_body(char *data_packet, char *labels[], float readings[],
                        int nvars, int service_flag);

/**
 * @brief Constructs full POST request for various services
 *
 * @param body Empty buffer to store POST request body
 * @param socket_dial_str Buffer to store socket dial string
 * @param send_str Buffer to store entire request, including headers
 * @param service_flag Service to use:
 * - 1: Xively
 * - 2: Influxdb
 * - 3: Thingspeak
 * - 4: CHORDS
 *
 * @return null
 */
void construct_data_post_request(char* body, char* socket_dial_str,
	                             char* send_str, int service_flag);

/* [] END OF FILE */
