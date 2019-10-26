#include "data.h"
#include "modem.h"
#include "extern.h"
#include "startup.h"
#include "ssl.h"
#include "config.h"

uint8 initialize_modem_params(char *send_str, char *response_str, 
    uint8 ssl_enabled, uint8 ssl_initialized){
    int status = 0u;
    if (modem_startup(&connection_attempt_counter)) {
        // Enable/disable sprint updates
        status = modem_updates_toggle(ALLOW_DM_UPDATES);
        // Initialize SSL if enabled
        status = initialize_ssl(&ssl_enabled, &ssl_initialized);
        // Update metadata if enabled
        status = run_meta_subroutine(meid, send_str, response_str, 1u);
        modem_shutdown();
    }
    return 1u;
}

uint8 counter_sensor_initialize(){
    // Turn on optical rain sensor if needed
    if ( (!optical_rain_pwr_Read()) && (optical_rain_flag) ) {
	    optical_rain_start();
        return 1u;
    }
    else if ( (optical_rain_pwr_Read()) && (!optical_rain_flag) ) {
	    optical_rain_stop();
        return 1u;
    }
    return 0u;
}

uint8 initialize_ssl(uint8 *ssl_enabled, uint8 *ssl_initialized){
    // TODO: This requires the modem to connect to the network
    // But really all we need to do is flip power and activate PDP context
    /*if (modem_startup(&connection_attempt_counter)){
        modem_ssl_toggle(*ssl_enabled);
        // Initialize SSL if enabled
        if (*ssl_enabled){
            (*ssl_initialized) = ssl_init(enable_ssl_sec_config, 
                                          enable_ssl_config);
            // If SSL initialization fails, fall back to unsecured connection
            if (!(*ssl_initialized)){
                (*ssl_enabled) = 0;
                return *ssl_initialized;
            }
        }
    }
    return *ssl_initialized;*/
    return 1u;
}

/* [] END OF FILE */
