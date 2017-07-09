/**
 * @file data.h
 * @brief Declares the sensor subroutine, metadata updater, trigger updater, and parameter updater.
 * @author Matt Bartos and Brandon Wong
 * @version TODO
 * @date 2017-06-19
 */
#ifndef DATA_H
#define DATA_H
#include <device.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "decagon.h"
#include "ultrasonic.h"
#include "optical_rain.h"
#include "atlas_wq_sensor.h"
#include "autosampler.h"
#include "valve.h"
#include "misc.h"
#include "modem.h"
#include "updater.h"
#include "commit.h"

// Function prototypes

/**
 * @brief Takes sensor readings and stores results into an array of labels
 * and an array of readings
 *
 * @param labels Array to store labels corresponding to each reading
 * @param readings Array to store readings as floating point values
 * @param array_ix Array index to label and readings
 * @param take_average Whether to take average or not
 * - 0: Take first valid reading 
 * - 1: Take average of valid readings
 * @param max_size Maximum size of label and reading arrays (number of entries) in buffer.
 *
 * @return (*array_ix) + number of entries filled
 */
int take_readings(char *labels[], float readings[], uint8 *array_ix, uint8 take_average, uint8 max_size);

/**
 * @brief Executes triggers and stores results into an array of labels
 * and an array of readings
 *
 * @param labels Array to store labels corresponding to each trigger result
 * @param readings Array to store trigger results as floating point values
 * @param array_ix Array index to label and readings
 * @param max_size Maximum size of label and reading arrays (number of entries)
 *
 * @return (*array_ix) + number of entries filled
 */
uint8 execute_triggers(char *labels[], float readings[], uint8 *array_ix, uint8 max_size);

/**
 * @brief Inserts current value of @p meta_trigger into readings array 
 *
 * @param labels Array to store labels corresponding to each trigger result
 * @param readings Array to store trigger results as floating point values
 * @param array_ix Array index to label and readings
 * @param max_size Maximum size of label and reading arrays (number of entries)
 *
 * @return (*array_ix) + number of entries filled
 */
uint8 zip_meta(char *labels[], float readings[], uint8 *array_ix, uint8 max_size);

/**
 * @brief Inserts current values of @p connection_attempt_counter @p rssi and @p fer into 
 * readings array
 *
 * @param labels Array to store labels corresponding to each trigger result
 * @param readings Array to store trigger results as floating point values
 * @param array_ix Array index to label and readings
 * @param max_size Maximum size of label and reading arrays (number of entries)
 *
 * @return (*array_ix) + number of entries filled
 */
uint8 zip_modem(char *labels[], float readings[], uint8 *array_ix, uint8 max_size);

/**
 * @brief Runs @p update_meta and stores the results in an array of readings and labels
 *
 * @param meid The mobile equipment identificiation number for the cell module
 * @param send_str Empty buffer to place request string
 * @param response_str Empty buffer to store server response
 * @param get_device_meid Whether to re-read meid from cell module.
 * - 0: Use current value of meid in memory
 * - 1: Read meid from cell module and store in memory
 *
 * @return 1 if metadata updated successfully, and 0 otherwise
 */
uint8 run_meta_subroutine(char* meid, char* send_str, char* response_str, uint8 get_device_meid);

/**
 * @brief Update values of @p node_id, @p node_user, @p node_pass, @p node_db on device
 * based on current values stored on server.
 *
 * @param meid The mobile equipment identificiation number for the cell module
 * @param send_str Empty buffer to place request string
 * @param response_str Empty buffer to store server response
 * @param get_device_meid Whether to re-read meid from cell module.
 * - 0: Use current value of meid in memory
 * - 1: Read meid from cell module and store in memory
 *
 * @return (response_code - true_response_code). The value of response code is a sum of 
 * the following powers of two:
 * - 1: @p node_id successfully updated
 * - 2: @p user successfully updated
 * - 4: @p pass successfully updated
 * - 8: @p database successfully updated
 *   A response code of 15 indicates that all parameters were successfully updated.
 *   The @p true_reponse_code is 15. Thus, if all parameters are updated successfully,
 *   the function will return 0.
 */
int update_meta(char* meid, char* send_str, char* response_str);

/**
 * @brief Update values of triggers, such as @p valve_trigger, @p autosampler_trigger 
 * and @p gps_trigger based on current values stored on server.
 *
 * @param body Empty buffer to store request body
 * @param send_str Empty buffer to store request string
 * @param response_str Empty buffer to store server response
 *
 * @return (response_code - true_response_code). The value of response code is a sum of 
 * the following powers of two:
 * - 1: @p meta_trigger successfully updated
 * - 2: @p autosampler_trigger successfully updated
 * - 4: @p valve_trigger successfully updated
 * - 8: @p valve_2_trigger successfully updated
 * - 16: @p gps_trigger successfully updated
 *   A response code of 15 indicates that all parameters were successfully updated.
 *   The @p true_reponse_code is 31. Thus, if all parameters are updated successfully,
 *   the function will return 0.
 */
int update_triggers(char* body, char* send_str, char* response_str);

/**
 * @brief Update values of onboard parameters, such as @p sleeptimer and flags such as 
 * @p ultrasonic_flag or @p valve_flag
 *
 * @param body Empty buffer to store request body
 * @param send_str Empty buffer to store request string
 * @param response_str Empty buffer to store server response
 *
 * @return null
 */
void update_params(char* body, char* send_str, char* response_str);

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
void construct_default_body(char *data_packet, char *labels[], float readings[],
                        int nvars);

/**
 * @brief Send readings to influxdb.
 *
 * @param body Empty buffer to hold message body 
 * @param send_str Empty buffer to hold request string
 * @param response_str Empty buffer to hold server response
 * @param socket_dial_str Empty buffer to hold string used in socket dial
 * @param labels Array containing reading labels
 * @param readings Array containing readings as floating point values
 * @param nvars Number of variables (entries) in the readings array
 *
 * @return 1 if data transmission was successful, otherwise 0.
 */
uint8 send_readings(char* body, char* send_str, char* response_str, char* socket_dial_str,
                    char *labels[], float readings[], uint8 nvars);

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

/**
 * @brief Return a URL-encoded version of the input string.
 *
 * @param str String to URL-encode
 *
 * @return URL-encoded string.
 */
char *url_encode(char *str);

#endif
/* [] END OF FILE */
