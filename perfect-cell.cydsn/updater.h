/**
 * @file updater.h
 * @brief Declares functions for parsing influxdb responses
 * @author Brandon Wong
 * @version TODO
 * @date 2017-06-19
 */

#include <project.h>

/**
 * @brief TODO: Not implemented
 */
int parse(char *search_str, char *in_str, char *out_str);                     // parse in_str for search_str and write to out_str

/**
 * @brief TODO: Not implemented
 */
int intparse(int *param, char *search_str, char *in_str, char *out_str);      // parse in_str for search_str and write int to param

/**
 * @brief TODO: Not implemented
 */
int uintparse(uint8 *param, char *search_str, char *in_str, char *out_str);    // parse in_str for search_str and write uint to param

/**
 * @brief TODO: Not implemented
 */
int floatparse(float *param, char *search_str, char *in_str, char *out_str);  // parse in_str for search_str and write float to param

/**
 * @brief Searches InfluxDB json packet, "packet", for "name" and
 * writes the associated value to "value" as a char array.
 * This is ported from packet_get_value() from packet.c in kLabUM/IoT
 *
 * @param value Value to be written to
 * @param packet Packet to search
 * @param name Substring to search for in packet
 *
 * @return 1u on success, 0u otherwise
 */
uint8 parse_influxdb(char* value, char* packet, char* name);

/**
 * @brief Searches InfluxDB json-packet, "packet" for "name" and
 * stores the associated value as a string in "param".
 * Removes the quotations that wrap the strings returned in the query.
 *
 * @param param Parameter to be written to
 * @param packet Packet to search
 * @param name Substring to search for in packet
 *
 * @return 1u on success, 0u otherwise
 */
uint8 strparse_influxdb(char* param, char* packet, char* name);

/**
 * @brief Searches InfluxDB json-packet, "packet" for "name" and
 * stores the associated value as an int in "param".
 * Removes the quotations that wrap the strings returned in the query.
 *
 * @param param Parameter to be written to
 * @param packet Packet to search
 * @param name Substring to search for in packet
 *
 * @return 1u on success, 0u otherwise
 */
uint8 intparse_influxdb(int* param, char* packet, char* name);

/**
 * @brief Resets a string to null bytes.
 *
 * @param str String to reset
 *
 * @return 1u on success, 0u otherwise
 */
uint8 clear_str(char* str);

/* [] END OF FILE */
