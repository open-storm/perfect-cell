/**
 * @file strlib.h
 * @brief Functions for parsing c strings.
 * @author Brandon Wong, Ivan Mondragon
 * @version TODO
 * @date 2017-06-19
 */

#ifndef STRLIB_H
#define STRLIB_H
#include <stddef.h>
#include <stdint.h>

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
void zips(char* begin[], char* end[], ...);

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
 * @return Pointer to first occurance of @p search_end after @p search_start or
 * NULL if extraction fails.
 */
char* strextract(const char input_str[], char output_str[],
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
uint8_t gps_parse(const char* gps_string, float* lat, float* lon, float* hdop,
                  float* altitude, uint8_t* gps_fix, float* cog, float* spkm,
                  float* spkn, uint8_t* nsat);

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
uint8_t parse_http_status(char* http_status, char* version, char* status_code,
                          char* phrase);

#endif
/* [] END OF FILE */
