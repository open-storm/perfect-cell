/**
 * @file updater.h
 * @brief Declares functions for parsing influxdb responses
 * @author Brandon Wong
 * @version TODO
 * @date 2017-06-19
 */

#ifndef UPDATER_H
#define UPDATER_H
#include <stddef.h>
#include <stdint.h>

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
int uintparse(uint8_t *param, char *search_str, char *in_str, char *out_str);    // parse in_str for search_str and write uint to param

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
uint8_t parse_influxdb(char* value, char* packet, char* name);

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
uint8_t strparse_influxdb(char* param, char* packet, char* name);

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
uint8_t intparse_influxdb(int* param, char* packet, char* name);

/**
 * @brief Resets a string to null bytes.
 *
 * @param str String to reset
 *
 * @return 1u on success, 0u otherwise
 */
uint8_t clear_str(char* str);

/**
 * @brief Variadic function that zips c strings.
 *
 * @param begin Buffer to store the c strings.
 * @param end Pointer to one past the end of the buffer.
 * @param ... Variadic number of c strings.
 */
void zips(char *begin[], char *end[], ...);

/**
 * @brief Variadic function that zips floats.
 *
 * @param begin Buffer to store the floats.
 * @param end Pointer to one past the end of the buffer.
 * @param ... Variadic number of floats.
 */
void zipf(float begin[], float end[], ...);

/**
 * @brief Searches for the left-most c string in between @p search_start and @p
 * search_end and stores it in @p output_str
 *
 * @param input_str[] C string to be searched
 * @param output_str[] Buffer to store the resulting c string
 * @param search_start[] Leading c string in the query
 * @param search_end[] Ending c string in the query
 *
 * @return Pointer to first occurance of @p search_end after @p search_start
 */
char *strextract(const char input_str[], char output_str[],
                 const char search_start[], const char search_end[]);

/**
 * @brief Applies a function over a range of values;
 *
 * @param begin[] Pointer to beginning of range.
 * @param end[] Pointer to `one past the end` of the range.
 * @param sz Size of each element in bytes.
 * @param fn function which transforms each value.
 */
void for_each(void *begin, void *end, size_t sz, void (*fn)(void *a));

#endif
/* [] END OF FILE */
