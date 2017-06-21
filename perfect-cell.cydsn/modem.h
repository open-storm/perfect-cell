/**
 * @file modem.h
 * @brief Functions for connecting to the cellular network, and sending/receiving packets.
 * @author Brandon Wong and Matt Bartos
 * @version TODO
 * @date 2017-06-01
 */
#include <project.h>

#define MODEM_STATE_OFF         0
#define MODEM_STATE_IDLE        1
#define MODEM_STATE_READY       3
#define MAX_GET_ATTEMPTS        2
#define MAX_SEND_ATTEMPTS       1//2
#define MAX_PACKET_LENGTH       1500

extern uint8	modem_state;
extern uint8	lock_acquired;
extern uint32	feed_id;
extern char*	api_key;

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
 * @brief Searches a string "http_status" and attempts to parse the status line.
 * Stores the results in "version", "status_code", and "phrase"
 *
 * @param http_status String to be parsed. Expects Status-Line protocol.
 * @param version Buffer to store the version.
 * @param status_code Buffer to store the status code.
 * @param phrase Buffer to store the phrase.
 *
 * @return 1u on success, 0u otherwise.
 */
uint8 parse_http_status(char* http_status, char* version, char* status_code, char* phrase);

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
 * @brief Finds the leftmost substring that begins with <search_start> and ends
 * with <search_end>, requires that <search_end> appears after <search_start>.
 *
 * @param input_str String to be parsed
 * @param output_str Buffer to store the result of parsing
 * @param search_start Substring start pattern to match, MUST appear before the end string
 * @param search_end Substring end pattern to match, MUST appear after the start string
 *
 * @return 1u on success, 0u otherwise.
 */
uint8 parse_at_command_result(char *input_str, char *output_str,
							  char *search_start, char *search_end);

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

// SSL functions

/**
 * @brief Toggle SSL Socket.
 *
 * @param enable_ssl enable flag:
 * - 0 = Disabled
 * - 1 = Enabled
 *
 * @return  1u on success, 0u otherwise.
 */
uint8 modem_ssl_toggle(int enable_ssl);

/**
 * @brief Stores the security data (certificate(s) and/or private key) into the module’s NVM
 *
 * @param ssid Must be set to 1. It is the only Secure Socket ID available.
 * @param action The action to be performed:
 * - 0 = deleting
 * - 1 = writing
 * - 2 = reading
 * @param datatype identifies the certificate/key to be stored or read:
 * - 0 = Certificate of the client (module). It is needed when the Server/Client authentication
 *   mode has been configured.
 * - 1 = CA Certificate of the remote server, it is used to authenticate the remote server. It is
 *   needed when <auth_mode> parameter of the #SSLSECCFG command is set to 1 or 2.
 * - 2 = RSA private key of the client (module). It is needed if the Server/Client
 *   authentication mode has been configured.
 * @param cert The certificate to be written to the modem.
 * @param output_str The buffer to write the current state of the modem. ONLY USED WHEN ACTION IS READ MODE.
 *
 * @return 1u on success, 0u otherwise.
 */
uint8 modem_ssl_sec_data(uint8 ssid, uint8 action, uint8 datatype,
                         char *cert, char *output_str);

/**
 * @brief configure the communication channel according to the user’s security architecture.
 *
 * @param ssid Must be set to 1. It is the only Secure Socket ID available
 * @param cipher_suite Setting the value 0 all the available cipher suites are proposed to the server.
 * It is responsibility of the remote server to select one of them:
 * - 0 = TLS_RSA_WITH_RC4_128_MD5 + TLS_RSA_WITH_RC4_128_SHA + TLS_RSA_WITH_AES_256_CBC_SHA
 * - 1 = TLS_RSA_WITH_RC4_128_MD5
 * - 2 = TLS_RSA_WITH_RC4_128_SHA
 * - 3 = TLS_RSA_WITH_AES_256_CBC_SHA
 * Warning - the product series HE920 / UE910 V2/ DE910 do not support TLS_RSA_WITH_NULL_SHA and TLS_RSA_WITH_AES_256_CBC_SHA.
 * @param auth_mode Is the authentication mode:
 * - 0 = SSL verify none: no authentication, no security data is needed at all
 * - 1 = Server authentication mode: CA Certificate storage is needed (the most common case)
 * - 2 = Server/Client authentication mode: CA Certificate (server), Certificate (client) and Private Key (client) are needed
 * @param cert_format Is an optional parameter. It selects the format of the certificate to be stored via #SSLSECDATA command.
 * - 0 = DER format
 * - 1 = PEM format
 *
 * @return 1u on success, 0u otherwise.
 */
uint8 modem_ssl_sec_config(uint8 ssid, uint8 cipher_suite, uint8 auth_mode,
                           uint8 cert_format);

/**
 * @brief Before opening the SSL socket, several parameters can be configured via this command:
 *
 * @param ssid Must be set to 1. It is the only Secure Socket ID available.
 * @param cid The PDP Context Identifier, it's value must be set to 1.
 * @param packet_size Is the size of the packet used by the SSL/TCP/IP stack for data sending in online mode.
 * Small <pktSize> values introduce a higher communication overhead.
 * @param max_to Is the socket inactivity timeout. In online mode: if there’s no data exchange within
 * this timeout period the connection is closed. Increment it if it is needed a longer idle time period.
 * @param def_to Timeout value used as default value by other SSL commands whenever their
 * Timeout parameters are not set.
 * @param tx_to Is the time period after which data is sent even if <pktSize> is not reached (only in online mode).
 * The parameter value must be tuned with user’s application requirements. Small <txTo> values introduce a higher communication overhead.
 * @param ssl_ring_mode Is the presentation mode of the SSLSRING unsolicited indication, which
 * informs the user about new incoming data that can be read in command mode. It can be
 * disabled using value 0.
 *
 * @return 1u on success, 0u otherwise.
 */
uint8 modem_ssl_config(uint8 ssid, uint8 cid, int packet_size,
                           int max_to, int def_to, int tx_to, uint8 ssl_ring_mode);

/**
 * @brief Initialize SSL features.
 *
 * @param edit_ssl_sec_config Enable flag: edit SSL security settings (SSLSECCFG) if desired.
 * @param edit_ssl_config Enable flag: edit general SSL configuration (SSLCFG) if desired.
 *
 * @return 1u on success, 0u otherwise.
 */
uint8 ssl_init(uint8 edit_ssl_sec_config, uint8 edit_ssl_config);

/**
 * @brief Toggle power to the GPS module.
 *
 * @param gps_power_on Enable flag:
 * - 0 = Power off
 * - 1 = Power on
 *
 * @return 1u on success, 0u otherwise.
 */
uint8 modem_gps_power_toggle(uint8 gps_power_on);

/**
 * @brief Retrieves the current GPS position by attempting to connect to the GPS
 * network.
 *
 * @param lat Buffer to store the Latitude.
 * @param lon Buffer to store the Longitude.
 * @param hdop Buffer to store the Horizontal Diluition of Precision.
 * @param altitude Buffer to store the Altitude: mean-sea-level (geoid).
 * @param gps_fix Buffer to store the flag:
 * - 0 = Invalid Fix
 * - 2 = 2D fix
 * - 3 = 3D fix
 * @param cog Buffer to store the Course over ground.
 * @param spkm Buffer to store the Speed over ground (km/hr).
 * @param spkn Buffer to store the Speed over ground (knots).
 * @param nsat Buffer to store the number of satellites in use [0..12]
 * @param min_satellites Minimum number of satellits the module
 * should connect to.
 * @param max_tries Maximum number of attempts.
 *
 * @return 1u if the gps module was able to connect to the minimum number of satellites,
 * 0u otherwise.
 */
uint8 modem_get_gps_position(float *lat, float *lon, float *hdop,
              float *altitude, uint8 *gps_fix, float *cog,
              float *spkm, float *spkn, uint8 *nsat, uint8 min_satellites, uint8 max_tries);

/**
 * @brief Parses the given gps string into the given buffers.
 *
 * @param gps_string The string to be parsed
 * @param lat Buffer to store the Latitude.
 * @param lon Buffer to store the Longitude.
 * @param hdop Buffer to store the Horizontal Diluition of Precision.
 * @param altitude Buffer to store the Altitude: mean-sea-level (geoid).
 * @param gps_fix Buffer to store the flag:
 * - 0 = Invalid Fix
 * - 2 = 2D fix
 * - 3 = 3D fix
 * @param cog Buffer to store the Course over ground.
 * @param spkm Buffer to store the Speed over ground (km/hr).
 * @param spkn Buffer to store the Speed over ground (knots).
 * @param nsat Buffer to store the number of satellites in use [0..12]
 *
 * @return 1u on successful parsing, 0u otherwise.
 */
uint8 gps_parse(char *gps_string, float *lat, float *lon, float *hdop,
              float *altitude, uint8 *gps_fix, float *cog,
              float *spkm, float *spkn, uint8 *nsat);

/**
 * @brief Runs GPS routine to retrieve GPS position.
 *
 * @param gps_trigger gps_trigger flag.
 * @param lat Buffer to store the Latitude.
 * @param lon Buffer to store the Longitude.
 * @param hdop Buffer to store the Horizontal Diluition of Precision.
 * @param altitude Buffer to store the Altitude: mean-sea-level (geoid).
 * @param gps_fix Buffer to store the flag:
 * - 0 = Invalid Fix
 * - 2 = 2D fix
 * - 3 = 3D fix
 * @param cog Buffer to store the Course over ground.
 * @param spkm Buffer to store the Speed over ground (km/hr).
 * @param spkn Buffer to store the Speed over ground (knots).
 * @param nsat Buffer to store the number of satellites in use [0..12]
 * @param min_satellites Minimum number of satellits the module
 * should connect to.
 * @param max_tries Maximum number of attempts.
 *
 * @return 1u if the gps module was able to connect to the minimum number of satellites,
 * 0u otherwise.
 */
uint8 run_gps_routine(int *gps_trigger, float *lat, float *lon, float *hdop,
              float *altitude, uint8 *gps_fix, float *cog,
              float *spkm, float *spkn, uint8 *nsat, uint8 min_satellites, uint8 max_tries);

/**
 * @brief Takes a string array of labels and float array of values and pairs the corresponding values.
 *
 * @param labels[] Array to store the labels.
 * @param readings[] Array to store the corresponding values.
 * @param array_ix  Current array index.
 * @param gps_trigger GPS trigger.
 * @param min_satellites Minimum number of satellits the module
 * should connect to.
 * @param max_tries Maximum number of attempts.
 * @param max_size Max size of the labels and readings arrays.
 *
 * @return (*array_ix) + 10
 */
uint8 zip_gps(char *labels[], float readings[], uint8 *array_ix, int *gps_trigger, uint8 min_satellites, uint8 max_tries, uint8 max_size);
//[] END OF FILE
