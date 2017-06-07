#include <project.h>

#define MODEM_STATE_OFF         0
#define MODEM_STATE_IDLE        1
#define MODEM_STATE_READY       3
#define MAX_GET_ATTEMPTS        2
#define MAX_SEND_ATTEMPTS       1//2
#define MAX_PACKET_LENGTH       1500

extern uint8    modem_state;
extern uint8    lock_acquired;
extern uint32   feed_id;
extern char*    api_key;

int modem_get_socket_status();
uint8	modem_startup(int *conn_attempts);
uint8 	modem_shutdown();
void    modem_start();
void	modem_stop();
uint8   modem_connect();
uint8   modem_disconnect();
uint8	modem_setup();
uint8   modem_check_network();
uint8   modem_get_meid(char* meid);
uint8   modem_check_signal_quality(int *rssi, int *fer);
uint8   modem_set_flow_control(uint8 param);
uint8 	modem_set_error_reports(uint8 param);
uint8   modem_send_packet(char* socket_dial_str, char* send_str);
uint8   modem_get_packet(char* packet, char* csv);
uint8   modem_get_state();
uint8   modem_reset();
uint8   modem_power_on();
uint8   modem_power_off();
uint8 modem_socket_dial(char *socket_dial_str, char* endpoint, int port, 
                        int construct_new, int ssl_enabled);
uint8 modem_socket_close(int ssl_enabled);
uint8 modem_send_recv(char* send_str, char* response, uint8 get_response, int ssl_enabled);
uint8 parse_http_status(char* http_status, char* version, char* status_code, char* phrase);
void construct_generic_request(char* send_str, char* body, char* host, char* route,
                               int port, char* method, char* connection_type,
	                           char *extra_headers, int extra_len, char* http_protocol);
uint8 parse_at_command_result(char *input_str, char *output_str, 
                            char *search_start, char *search_end);
uint8 modem_pdp_context_toggle(uint8 activate_pdp);

// SSL functions
uint8 modem_ssl_toggle(int enable_ssl);
uint8 modem_ssl_sec_data(uint8 ssid, uint8 action, uint8 datatype, 
                         char *cert, char *output_str);
uint8 modem_ssl_sec_config(uint8 ssid, uint8 cipher_suite, uint8 auth_mode,
                           uint8 cert_format);
uint8 modem_ssl_config(uint8 ssid, uint8 cid, int packet_size,
                           int max_to, int def_to, int tx_to, uint8 ssl_ring_mode);
uint8 ssl_init(uint8 edit_ssl_sec_config, uint8 edit_ssl_config);
uint8 modem_gps_power_toggle(uint8 gps_power_on);
uint8 modem_get_gps_position(float *lat, float *lon, float *hdop, 
              float *altitude, uint8 *gps_fix, float *cog, 
              float *spkm, float *spkn, uint8 *nsat, uint8 min_satellites, uint8 max_tries);
uint8 gps_parse(char *gps_string, float *lat, float *lon, float *hdop, 
              float *altitude, uint8 *gps_fix, float *cog, 
              float *spkm, float *spkn, uint8 *nsat);
uint8 run_gps_routine(int *gps_trigger, float *lat, float *lon, float *hdop, 
              float *altitude, uint8 *gps_fix, float *cog, 
              float *spkm, float *spkn, uint8 *nsat, uint8 min_satellites, uint8 max_tries);
uint8 zip_gps(char *labels[], float readings[], uint8 *array_ix, int *gps_trigger, uint8 min_satellites, uint8 max_tries, uint8 max_size);
//[] END OF FILE
