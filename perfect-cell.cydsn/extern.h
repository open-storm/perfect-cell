#ifndef EXTERN_H
#define EXTERN_H
#include <project.h>

#define MODEM_BUFFER_LENGTH     1600
    
extern char node_id[20];
extern char site_name[10];

// Defaults if service.c not used

extern char user[20];
extern char pass[50];
extern char database[20];

extern int main_port;
extern char main_host[100];
extern char main_method[5];
extern char main_connection_type[11];
extern int main_extra_len;
extern char main_extra_headers[200];
extern char main_tags[200];
extern char main_http_protocol[4];
extern char meid[20];
extern char write_route[60];

extern uint8 array_ix;

// Buffer for cell module
extern char modem_received_buffer[MODEM_BUFFER_LENGTH];

// SSL/TLS enabled?
extern uint8 ssl_enabled;
extern uint8 enable_ssl_config;
extern uint8 enable_ssl_sec_config;

// Set service to use here
extern int service_flag;

// Sleeptimer
extern int sleeptimer; // Number of wakeups before full power: 1172 @ 256ms ~5 min

// Modem
// Number of attempts modem made while trying to establish a connection
extern int max_conn_attempts;
extern int connection_attempt_counter;
extern int rssi; 
extern int fer;

// Flags to trigger devices
extern int modem_flag;
extern int meta_flag;
extern int vbat_flag;
extern int ultrasonic_flag;
extern int ultrasonic_2_flag;
extern int optical_rain_flag;
extern int decagon_flag;
extern int atlas_wq_flag;
extern int autosampler_flag;
extern int valve_flag;
extern int gps_flag;

extern int autosampler_trigger;
extern int valve_trigger;
extern int valve_2_trigger;
extern int meta_trigger;
extern int gps_trigger;

// Number of loops for each device
extern int vbat_loops;
extern int ultrasonic_loops;
extern int optical_rain_loops;
extern int decagon_loops;

// Number of vars for each device
extern int modem_vars;
extern int vbat_vars;
extern int ultrasonic_vars;
extern int optical_rain_vars;
extern int decagon_vars;
extern int autosampler_vars;
extern int valve_vars;

// Server parameters
extern char *storm_endpoint;
extern int storm_port;

#endif
/* [] END OF FILE */
