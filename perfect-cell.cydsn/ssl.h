/**
 * @file ssl.h
 * @brief Implements functions for configuring SSL/TLS
 * @author Matt Bartos and Ivan Mondragon
 * @version TODO
 * @date 2017-06-01
 */

#ifndef SSL_H
#define SSL_H
#include <project.h>

// Certificates
#define CLIENT_CERT "-----\032\0"
#define PRIVATE_KEY "-----\032\0"
#define SERVER_CERT "-----BEGIN CERTIFICATE-----\n"\
"MIIEMTCCAxmgAwIBAgIJAM099V95yUdhMA0GCSqGSIb3DQEBBQUAMIGuMQswCQYD\n"\
"VQQGEwJVUzELMAkGA1UECAwCTUkxEjAQBgNVBAcMCUFubiBBcmJvcjEfMB0GA1UE\n"\
"CgwWVW5pdmVyc2l0eSBvZiBNaWNoaWdhbjEkMCIGA1UECwwbUmVhbC10aW1lIFdh\n"\
"dGVyIFN5c3RlbXMgTGFiMRQwEgYDVQQDDAtNYXR0IEJhcnRvczEhMB8GCSqGSIb3\n"\
"DQEJARYSbWRiYXJ0b3NAdW1pY2guZWR1MB4XDTE3MDUyOTAzMjAwOVoXDTE4MDIw\n"\
"ODAzMjAwOVowga4xCzAJBgNVBAYTAlVTMQswCQYDVQQIDAJNSTESMBAGA1UEBwwJ\n"\
"QW5uIEFyYm9yMR8wHQYDVQQKDBZVbml2ZXJzaXR5IG9mIE1pY2hpZ2FuMSQwIgYD\n"\
"VQQLDBtSZWFsLXRpbWUgV2F0ZXIgU3lzdGVtcyBMYWIxFDASBgNVBAMMC01hdHQg\n"\
"QmFydG9zMSEwHwYJKoZIhvcNAQkBFhJtZGJhcnRvc0B1bWljaC5lZHUwggEiMA0G\n"\
"CSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCcdIs+Nt2CwskByoqSUJi9l+H/6y20\n"\
"bZQXDu99v69JXCUGltyss5akBPtbQHWq+hVQSwCXphnYVl2ZsqwKdiz4kuEc/GhT\n"\
"Ng5XqPRWomWC8x3L0xvblvSqYK90tLz0FmzU8zVq6f/OLlTPJZwAhYC8i0mnqbS0\n"\
"KMDvXPA4FfayBhDX9bOUUQos7WoGFQmfT/K/xWlIPmQs2QOFdx6Tp4669JaxnpzZ\n"\
"wSWe7EUidblUbOzCQtKb/XeVQfuW2xdXxQQRr740mY+/w2dHVl0132lypP60nUbk\n"\
"NEVziu4s/C3Lwfb296t4HUfOg460uyzkdDWDZ6NBtSFRNXDhQjeUSs2pAgMBAAGj\n"\
"UDBOMB0GA1UdDgQWBBSJJFBLv/J+ysjgjtHN+FnfTyjIHzAfBgNVHSMEGDAWgBSJ\n"\
"JFBLv/J+ysjgjtHN+FnfTyjIHzAMBgNVHRMEBTADAQH/MA0GCSqGSIb3DQEBBQUA\n"\
"A4IBAQBLGrM/RZKPXyQ2f6ZFA9vugU3KzVgCmV1Z62Io8jOfq8Mhrf0j6s65Yxoh\n"\
"KFqvkrozcG+I44Daz9IZbxU04AsYxhpKN5qO5W2PdS9xOXJWugAciVMrTg510WZd\n"\
"JgYRiYdCk4L72GxvdJ4UKnH1N+t6ix0vAT4e6f/CoLQg6CIhQNjOojR9wz6BkpNn\n"\
"Ra81H5kG3lfajk+o/KPbP4L6CexDnkWiYrkeKPU6SSC7RJ/KqxDHScBfqtmz9OjT\n"\
"xtvtzVID7V7pKFM3j3dON81fLrbDtQtu6XPsBpilfl78rl+hess/tMqnAEHaaqpb\n"\
"54+PblnOoiwRzbySIqKde+lDMCVb\n"\
"-----END CERTIFICATE-----\n"\
"\032\0"
// Make sure to end cert with escape char    
    
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

#endif
/* [] END OF FILE */
