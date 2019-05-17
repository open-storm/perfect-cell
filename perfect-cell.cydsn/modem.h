/**
 * @file modem.h
 * @brief Functions for connecting to the cellular network, and sending/receiving packets.
 * @author Brandon Wong and Matt Bartos
 * @version TODO
 * @date 2017-06-01
 */
#ifndef MODEM_H
#define MODEM_H
#include <project.h>
#include "sdcard.h"    

#define MODEM_STATE_OFF         0
#define MODEM_STATE_IDLE        1
#define MODEM_STATE_READY       3
#define MAX_GET_ATTEMPTS        2
#define MAX_SEND_ATTEMPTS       1//2
#define MAX_SEND_LENGTH         5000
#define MAX_RECV_LENGTH         5000
#define CHUNK_SIZE              800
#define CHUNK_STRING_LENGTH     10

extern uint8	modem_state;
extern uint8	lock_acquired;
extern uint32	feed_id;
extern char*	api_key;

/**
 * @brief Enable/disable Sprint updates. Execution command is used to enable or disable updates.
 *
 * @param updates_enabled enable flag:
 * - 0 = disable updates
 * - 1 = enable updates
 *
 * @return 1u on successful toggle, 0u otherwise
 */
uint8 modem_updates_toggle(uint8 updates_enabled);
    
/**
 * @brief Startup sequence to power on the modem, start modem components and
 * prepare for sending/receiving messages over the network.
 *
 * @param conn_attempts Buffer to store the number of attempted connections
 *
 * @return Modem status.
 */
uint8 modem_startup(int *conn_attempts);

/**
 * @brief Shutdown sequence to stop/power down modem components
 *
 * @return 1u on success, 0u otherwise.
 */
uint8 modem_shutdown();

/**
 * @brief Initialize modem.
 */
void modem_start();

/**
 * @brief Deinitialize modem.
 */
void modem_stop();

/**
 * @brief Write AT command to cell module
 *
 * @param uart_string AT command to write to buffer
 * @param expected_response Substring that is expected in the response
 * @param uart_timeout Timeout in milliseconds
 *
 * @return 1u if @p expected response is found; 0u otherwise.
 */
uint8 at_write_command(char* uart_string, char* expected_response, uint32 uart_timeout);

/**
 * @brief Establish modem connection with internet.
 *
 * @return Modem state. {OFF, IDLE, READY}
 */
uint8 modem_connect();

/**
 * @brief Close modem connection to network.
 *
 * @return 1u on success, 0u otherwise.
 */
uint8 modem_disconnect();

/**
 * @brief Initialize configurations for the modem.
 *
 * @return 1u on success, 0u otherwise.
 */
uint8 modem_setup();

/**
 * @brief Network Registration Report. Function wrapper for AT+CREG command.
 *
 * @return 1u on success, 0u otherwise.
 */
uint8 modem_check_network();

/**
 * @brief Socket Status. Function wrapper for AT#SS command.
 *
 * @return Socket status on success. 0 or -1 on failure,
 * where 0 means failure to parse the uart received string.
 */
int	modem_get_socket_status();

/**
 * @brief Mobile Equipment Identifier. Function wrapper for AT#MEID command.
 *
 * @param meid String of size large enough to store an meid of 14 characters.
 * In other words, sizeof(meid) > 14.
 *
 * @return 1u on success, 0u otherwise.
 */
uint8 modem_get_meid(char* meid);

/**
 * @brief Signal Quality Check. Function wrapper for AT+CSQ command.
 *
 * @param rssi  An int* where rssi is to be stored.
 * @param fer   An int* where fer is to be stored.
 *
 * @return 1u on success, 0u otherwise.
 */
uint8 modem_check_signal_quality(int *rssi, int *fer);

/**
 * @brief Set Access Point Name (APN)
 *
 * @return 1u on success, 0u otherwise.
 */
uint8 modem_set_apn();

/**
 * @brief Set Functionality. 
 *
 * @param param One of the following ints:
 * - 0 minimum functionality
 * - 1 full functionality
 * - 2 disable phone transmit RF circuits only
 * - 3 disable phone receive RF circuits only
 * - 4 disable phone both transmit and receive RF circuits
 *
 * @return 1u on success, 0u otherwise.
 */
uint8 modem_set_fun(uint8 param);

/**
 * @brief Set Flow Control. Set command controls the RS232 flow control behaviour.
 *
 * @param param One of the following ints:
 * - 0 = no flow control
 * - 1 = hardware mono-directional flow control (only CTS active)
 * - 2 = software mono-directional flow control (XON/XOFF)
 * - 3 = hardware bi-directional flow control (both RTS/CTS active) (factory default)
 * - 4 = software bi-directional with filtering (XON/XOFF)
 * - 5 = pass through: software bi-directional without filtering (XON/XOF
 *
 * @return 1u on success, 0u otherwise.
 */
uint8 modem_set_flow_control(uint8 param);

/**
 * @brief Report Mobile Equipment Error. Set command enables/disables the report of result code.
 *
 * @param param enable flag:
 * - 0 : disable +CME ERROR:<err> reports, use only ERROR report.
 * - 1 : enable +CME ERROR:<err> reports, with <err> in numeric format
 * - 2 : enable +CME ERROR: <err> reports, with <err> in verbose format
 *
 * @return 1u on success, 0u otherwise.
 */
uint8 modem_set_error_reports(uint8 param);

// Removed functions
//uint8 modem_send_packet(char* socket_dial_str, char* send_str);
//uint8 modem_get_packet(char* packet, char* csv);
//uint8 modem_get_state();

/**
 * @brief Resets the modem.
 *
 * @return Modem state. {OFF, IDLE, READY}
 */
uint8 modem_reset();

/**
 * @brief Powers on the modem.
 *
 * @return Modem state {OFF, IDLE, READY} or 0u on failure to power on.
 */
uint8 modem_power_on();

/**
 * @brief Powers off the modem.
 *
 * @return 1u on success.
 */
uint8 modem_power_off();

/**
 * @brief Opens a remote connection via socket.
 *
 * @param socket_dial_str
 * @param endpoint Endpoint
 * @param port Port
 * @param construct_new Contruct new socket dial string flag
 * @param ssl_enabled SSL enabled flag
 *
 * @return 1u on success, 0u otherwise.
 */
uint8 modem_socket_dial(char *socket_dial_str, char* endpoint, int port,
                        int construct_new, int ssl_enabled);

/**
 * @brief Closes the socket on the modem.
 *
 * @param ssl_enabled Needs to know if ssl is enabled to close the socket properly.
 *
 * @return 1u on success, 0u otherwise.
 */
uint8 modem_socket_close(int ssl_enabled);

/**
 * @brief Breaks request string into packets and sends them.
 *
 * @param send_str The request string to be sent
 * @param chunk Empty buffer for holding string chunks
 * @param chenk_len The size of the chunks to send
 * @param send_cmd The AT command used to send data: SSEND, SSENDEXT or SSLSEND
 * @param ring_cmd The ring command to look for: SRING or SSLRING
 * @param term_char The termination character to use: '\032' for Telit
 *
 * @return 1u on success, 0u otherwise.
 */
int send_chunked_request(char* send_str, char *chunk, int chunk_len, char *send_cmd, char *ring_cmd, char *term_char);

/**
 * @brief Reads a HTTP response with a chunked of fixed-length transfer encoding over multiple modem buffers
 *
 * @param message The message buffer to write to
 * @param recv_cmd The Telit receive command to use: AT#SRECV=1,<BYTES>\r or AT#SSLRECV=1,<BYTES>\r
 * @param ring_cmd The Telit ring URC to seek: SRING: 1 or SSLRING: 1
 * @param get_response 1u if the response is to be downloaded to the response buffer; 0u otherwise 
 * @param max_loops The maximum number of times to loop; used as a safeguard
 * @param max_message_size The maximum size of the @p message
 *
 * @return 1u on success, 0u otherwise.
 */
int read_response(char message[], char *recv_cmd, char *ring_cmd, uint8 get_response, 
                  int max_loops, int max_message_size);

/**
 * @brief TODO
 *
 * @param send_str
 * @param response
 * @param get_response
 * @param ssl_enabled
 *
 * @return 1u on success, 0u otherwise.
 */
uint8 modem_send_recv(char* send_str, char* response, uint8 get_response, int ssl_enabled);

/**
 * @brief Constructs a generic HTTP request.
 *
 * @param send_str Buffer to store generated HTTP request.
 * @param body HTTP Body.
 * @param host Host.
 * @param route Route.
 * @param port Port.
 * @param method HTTP Method.
 * @param connection_type HTTP Connection.
 * @param extra_headers Any extra HTTP headers.
 * @param extra_len Length of the extra headers.
 * @param http_protocol HTTP protocol.
 */
void construct_generic_request(char* send_str, char* body, char* host, char* route,
                               int port, char* method, char* connection_type,
							   char *extra_headers, int extra_len, char* http_protocol);

/**
 * @brief Context Activation. Execution command is used to activate or deactivate the specified PDP context.
 *
 * @param activate_pdp enable flag:
 * - 0 = deactivate the context
 * - 1 = activate the context
 *
 * @return 1u on successful toggle, 0u otherwise
 */
uint8 modem_pdp_context_toggle(uint8 activate_pdp);

/**
 * @brief Reset modem_received_buffer, clear RX buffer, and reset string index
 *
 * @return NULL
 */
void uart_string_reset();

#endif
//[] END OF FILE

