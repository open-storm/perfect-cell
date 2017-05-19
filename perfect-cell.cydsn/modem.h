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
uint8 modem_socket_dial(char *socket_dial_str, char* endpoint, int port, int construct_new);
uint8 modem_socket_close();
uint8 modem_send_recv(char* send_str, char* response, uint8 get_response);
uint8 parse_http_status(char* http_status, char* version, char* status_code, char* phrase);
void construct_generic_request(char* send_str, char* body, char* host, char* route,
                               int port, char* method, char* connection_type,
	                           char *extra_headers, int extra_len, char* http_protocol);

//[] END OF FILE
