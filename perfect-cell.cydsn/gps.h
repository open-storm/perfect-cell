/**
 * @file gps.h
 * @brief Implements Telit's GPS geolocation functions
 * @author Matt Bartos and Ivan Mondragon
 * @version TODO
 * @date 2017-06-01
 */

#ifndef GPS_H
#define GPS_H
#include <project.h>

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
    
    
#endif
/* [] END OF FILE */
