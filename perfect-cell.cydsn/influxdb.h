#include "device.h"

/**
 * @brief Constructs the route (URL) following the base endpoint.
 *
 * @param route Empty buffer to store route information
 * @param base Base endpoint
 * @param user Influxdb username
 * @param pass Influxdb password
 * @param database Influxdb database
 *
 * @return null
 */
void construct_route(char* route, char* base, char* user, char* pass, char* database);

/**
 * @brief Parses label and readings arrays into an influxdb line protocol body. 
 *
 * @param data_packet Empty buffer to store POST request body
 * @param labels Buffer to store labels corresponding to each trigger result
 * @param readings Buffer to store trigger results as floating point values
 * @param nvars Number of variables (entries) in the readings array
 *
 * @return null
 */
void construct_influxdb_body(char *data_packet, char *labels[], float readings[],
                        int nvars);

/**
 * @brief Append new tags to existing influxdb tags.
 *
 * @param main_tags Existing tag string (comma-separated) 
 * @param appended_label Tag label to append
 * @param appended_value Tag value to append
 *
 * @return 1
 */
uint8 append_tags(char *main_tags, char *appended_label, char *appended_value);

/* [] END OF FILE */
