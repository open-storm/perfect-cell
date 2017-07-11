/**
 * @file startup.h
 * @brief Declares functions used on startup (before main loop).
 * @author Matt Bartos
 * @version TODO
 * @date 2017-06-19
 */
#ifndef STARTUP_H
#define STARTUP_H
#include <project.h>

/**
 * @brief TODO: Not implemented
 */
int initial_metadata_update(char* send_str, char* response_str, uint8 *ssl_initialized,
                            uint8 *update_successful, uint8 *socket_status);
/**
 * @brief Initialize counter-type sensors that must remain on during the sleep cycle.
 * Currently only the optical rain sensor requires this behavior
 *
 * @return 1u on success, 0u otherwise
 */
uint8 counter_sensor_initialize();

/**
 * @brief Initialize TLS/SSL for the cell module
 *
 * @param ssl_enabled Desired SSL state:
 * - 0: SSL not enabled
 * - 1: SSL enabled
 * @param ssl_initialized Current SSL initialization state (to be written to):
 * - 0: SSL not successfully initialized
 * - 1: SSL successfully initialized
 * @return @p ssl_initialized
 */
uint8 initialize_ssl(uint8 *ssl_enabled, uint8 *ssl_initialized);

#endif
/* [] END OF FILE */
