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

int construct_data_body(char *data_packet, char *labels[], float readings[],
                        int nvars, int service_flag);
void construct_data_post_request(char* body, char* socket_dial_str,
	                             char* send_str, int service_flag);

/* [] END OF FILE */
