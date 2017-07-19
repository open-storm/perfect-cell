/**
 * @file gps.c
 * @brief Implements Telit's GPS geolocation functions
 * @author Matt Bartos and Ivan Mondragon
 * @version TODO
 * @date 2017-06-01
 */

#include <device.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "modem.h"
#include "strlib.h"
#include "extern.h"
#include "gps.h"

uint8 modem_gps_power_toggle(uint8 gps_power_on) {
    char cmd[20];
    char gps_power[2] = {'\0'};

    // Send AT read command to determine if GPS is already powered
    if (at_write_command("AT$GPSP?\r", "OK", 1000u)) {
        // Extract current gps state into gps_power
        strextract(modem_received_buffer, gps_power, "GPSP: ", "\r\n");

        // If current state matches desired state, do nothing
        if (atoi(gps_power) != gps_power_on) {
            // Construct AT command
            sprintf(cmd, "AT$GPSP=%u\r", gps_power_on);

            // Enable/disable GPS power
            return at_write_command(cmd, "OK", 5000u);
        } else {
            return 1u;
        }
    }

    return 0u;
}

uint8 modem_get_gps_position(float *lat, float *lon, float *hdop, 
              float *altitude, uint8 *gps_fix, float *cog, 
              float *spkm, float *spkn, uint8 *nsat, uint8 min_satellites, uint8 max_tries){
    uint8 status;
    int gps_iter;
    
        for(gps_iter=0; gps_iter < max_tries; gps_iter++){
            status = at_write_command("AT$GPSACP\r", "OK", 10000u);
            if (status){
                gps_parse(modem_received_buffer, lat, lon, hdop, altitude, gps_fix, cog, spkm, spkn, nsat);
                clear_str(modem_received_buffer);
                if (*nsat >= min_satellites){
                    return 1u;
                }
                CyDelay(5000u);
            }
        }
    return 0u;
}

uint8 run_gps_routine(int *gps_trigger, float *lat, float *lon, float *hdop, 
              float *altitude, uint8 *gps_fix, float *cog, 
              float *spkm, float *spkn, uint8 *nsat, uint8 min_satellites, uint8 max_tries){

    uint8 status;
    // Make sure you start with power to GPS off
    modem_gps_power_toggle(0u);    
    // Unlock GPS
    status = at_write_command("AT$GPSLOCK=0\r", "OK", 1000u);
    if (!status){ return 0u;}
    CyDelay(100u);
    // Set antenna path to GPS
    status = at_write_command("AT$GPSAT=1\r", "OK", 1000u);
    if (!status){ return 0u;}
    // Turn on power to GPS
    modem_gps_power_toggle(1u);
    CyDelay(10000u);
    // Get GPS Position
    status = modem_get_gps_position(lat, lon, hdop, 
                                    altitude, gps_fix, cog, 
                                    spkm, spkn, nsat, min_satellites, max_tries);
    // Turn off power to GPS
    modem_gps_power_toggle(0u);
    // Reset GPS Settings
    at_write_command("AT$GPSRST\r", "OK", 1000u);
    // For now, always shut gps trigger off
    (*gps_trigger) = 0u;
    return status;
}

uint8 zip_gps(char *labels[], float readings[], uint8 *array_ix,
              int *gps_trigger, uint8 min_satellites, uint8 max_tries,
              uint8 max_size) {
    // Ensure we don't access nonexistent array index
    uint8 nvars = 10;
    if (*array_ix + nvars >= max_size) {
        return *array_ix;
    }

    float lat = -9999;
    float lon = -9999;
    float hdop = -9999;
    float altitude = -9999;
    uint8 gps_fix = 0u;
    float cog = -9999;
    float spkm = -9999;
    float spkn = -9999;
    uint8 nsat = 0u;

    // with the begin/end paradigm, end must always be `one past the end`
    char **begins = labels + *array_ix;
    char **ends = begins + nvars;
    zips(begins, ends, "gps_latitude", "gps_longitude", "gps_hdop",
         "gps_altitude", "gps_fix", "gps_cog", "gps_spkm", "gps_spkn",
         "gps_nsat", "gps_trigger");

    run_gps_routine(gps_trigger, &lat, &lon, &hdop, &altitude, &gps_fix, &cog,
                    &spkm, &spkn, &nsat, min_satellites, max_tries);

    // with the begin/end paradigm, end must always be `one past the end`
    float *beginf = readings + *array_ix;
    float *endf = beginf + nvars;
    zipf(beginf, endf, lat, lon, hdop, altitude, (float) gps_fix, cog, spkm,
         spkn, (float) nsat, *gps_trigger);

    (*array_ix) += nvars;

    return *array_ix;
}    

/* [] END OF FILE */
