/**
 * @file strlib.c
 * @brief Functions for parsing c strings.
 * @author Brandon Wong, Ivan Mondragon
 * @version TODO
 * @date 2017-06-19
 */

#include "strlib.h"
#include <stdarg.h> /* va_list, va_start, va_arg, va_end */
#include <stdlib.h>
#include <string.h>

/*
Searches InfluxDB json packet, "packet", for name and
returns the associated value as a char array.
Returns 1 if successful, and 0 if there was an error.

This is ported from packet_get_value() from packet.c in kLabUM/IoT
*/
uint8_t parse_influxdb(char *value, char *packet, char *name) {
    char *a, *b;

    // Find the first occurence of "name" in "packet"
    a = strstr(packet, name);
    if (a == NULL) {
        return 0u;
    }

    // Move the pointer to the timestamp after the first occurence of "name"
    // This marks where the value is
    a = strstr(a, "Z\",") + strlen("Z\",");
    if (a == NULL) {
        return 0u;
    }

    // Find the closing bracket that follows the timestamp, marking the end of
    // the value
    b = strstr(a, "]");
    if (b == NULL) {
        return 0u;
    }

    strncpy(value, a, b - a);
    value[b - a] = '\0';
    return 1u;
}

/*
Searches InfluxDB json-packet, "packet" for "name" and
stores the associated value as string in "param".
Removes the quotations that wrap the strings returned in the query.
Returns 1 if successful, and 0 if there was an error.
*/
uint8_t strparse_influxdb(char *param, char *packet, char *name) {
    // assume a string no longer than 100 bytes is stored
    char value_str[100] = {'\0'};

    if (parse_influxdb(value_str, packet, name) == 1u) {
        if (strstr(value_str, "null") == NULL) {
            // Copy the result into "param"
            strncpy(param, value_str + 1, strlen(value_str) - 2);
            param[strlen(value_str) - 2] = '\0';

            return 1u;
        }
    }

    return 0u;
}

/*
Searches InfluxDB json-packet, "packet" for "name" and
stores the associated value as int in "param".
Returns 1 if successful, and 0 if there was an error.
*/
uint8_t intparse_influxdb(int *param, char *packet, char *name) {
    char value_str[20] = {'\0'};  // note, int is at most 5 digits long

    if (parse_influxdb(value_str, packet, name)) {
        *param = (int) strtol(value_str, (char **) NULL, 10);  // Base 10
        // http://www.cplusplus.com/reference/cstdlib/strtol/
        return 1u;
    }

    return 0u;
}

/**
 * @brief Clear a string by using memset() to replace all characters with '\0' (null char)
 * 
 * @param str String to be cleared
 *
 * @return 1u 
 */
uint8_t clear_str(char *str) {
    memset(str, '\0', strlen(str));
    return 1u;
}

/**
 * @brief Append a variable number of strings to the array of measurement labels 
 *
 * @param begin Array of strings (used as a string pointer) to store labels corresponding to each sensor reading.
 *        begin starts by pointing to where the first string in the variable-length list will be inserted.
 * @param end Array to strings (used as a string pointer) to indicate when to stop inserting labels
 * @param ... comma-separated list of strings. The list is of variable length
 *
 * @return none
 */
void zips(char *begin[], char *end[], ...) {
    va_list args;
    va_start(args, end);

    while (begin < end) {
        *begin++ = va_arg(args, char *);
    }

    va_end(args);
}

/** Possible edit, where n_labels is the number of labels to be added
 * (Uses a for-loop to replace while-loop)
 * void zips(char *labels[], int n_labels, ...)
 * va_list args;
 * va_start(args, n_labels);
 * int i;
 *
 * for(i = 0; i < n_labels; i++) {
 *      *labels++ = va_arg(args, char *);
 * }
 *
 * va_end(args);
 */


/**
 * @brief Append a variable number of floats to the array of measurement values 
 *
 * @param begin Array of floats to store values corresponding to each sensor reading.
 *        begin starts by pointing to where the first value in the variable-length list will be inserted.
 * @param end   Array to floats (used as a pointer) to indicate when to stop inserting values
 * @param ... comma-separated list of values. The list is of variable length
 *
 * @return none
 */
void zipf(float begin[], float end[], ...) {
    va_list args;
    va_start(args, end);

    while (begin < end) {
        *begin++ = va_arg(args, double);
    }

    va_end(args);
}

/** Possible edit is similar to the suggestion above, 
 *  where the number of readings is declared explicitly
 *  allowing us to replace the while-loop with a for-loop
 */

char *strextract(const char input_str[], char output_str[],
                 const char search_start[], const char search_end[]) {
    if (input_str == NULL) return NULL;
    char *begin, *end = NULL;

    if ((begin = strstr(input_str, search_start))) {
        begin += strlen(search_start);
        if ((end = strstr(begin, search_end))) {
            strncpy(output_str, begin, end - begin);
            output_str[end - begin] = '\0';
        }
    }

    return end;
}

void for_each(void *begin, void *end, size_t sz, void (*fn)(void *a)) {
    char *ptr = begin;
    char *last = end;

    while (ptr < last) {
        fn(ptr);
        ptr += sz;
    }
}

uint8_t gps_parse(const char *gps_string, float *lat, float *lon, float *hdop,
                  float *altitude, uint8_t *gps_fix, float *cog, float *spkm,
                  float *spkn, uint8_t *nsat) {
    char substring[100];
    const char delim[2] = ",";
    const char *output_array[11];
    int i;
    char latdeg[3] = {'\0'};
    char latmin[8] = {'\0'};
    char londeg[4] = {'\0'};
    char lonmin[8] = {'\0'};

    if (strextract(gps_string, substring, "GPSACP: ", "\r\n")) {
        // Save UTC to dummy variable
        output_array[0] = strtok(substring, delim);

        if (output_array[0] == NULL) {
            return 0u;
        }

        for (i = 1; i < 11; i++) {
            output_array[i] = strtok(NULL, delim);
            if (output_array[i] == NULL) {
                return 0u;
            }
        }

        strncpy(latdeg, output_array[1], 2);
        strncpy(latmin, output_array[1] + 2, 7);
        strncpy(londeg, output_array[2], 3);
        strncpy(lonmin, output_array[2] + 3, 7);
        // STILL NEED TO ACCOUNT FOR W/E and N/S

        *lat = strtof(latdeg, NULL) + (strtof(latmin, NULL) / 60.0);
        *lon = strtof(londeg, NULL) + (strtof(lonmin, NULL) / 60.0);
        *hdop = strtof(output_array[3], NULL);
        *altitude = strtof(output_array[4], NULL);
        *gps_fix = strtod(output_array[5], NULL);
        *cog = strtof(output_array[6], NULL);
        *spkm = strtof(output_array[7], NULL);
        *spkn = strtof(output_array[8], NULL);
        *nsat = strtod(output_array[10], NULL);
        if (strstr(output_array[1], "S") != NULL) {
            (*lat) *= -1.0;
        }
        if (strstr(output_array[2], "W") != NULL) {
            (*lon) *= -1.0;
        }
        return 1u;
    }
    return 0u;
}

/*
Searches a string "http_status" for and attempts to parse the status line.
Leverages Status-Line protocol for HTTP/1.0 and HTTP/1.1

    Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF

(See https://www.w3.org/Protocols/HTTP/1.0/spec.html#Status-Line.
 SP is a space, CRLF is carriage return line feed)

Stores the results in "version", "status_code", and "phrase"
*/
uint8_t parse_http_status(char *http_status, char *version, char *status_code,
                          char *phrase) {
    const char *space = strextract(http_status, version, "HTTP/", " ");
    const char *next_space = NULL;
    const char *terminator = NULL;

    if (space != NULL) {
        next_space = strextract(space, status_code, " ", " ");
    }

    if (next_space != NULL) {
        terminator = strextract(next_space, phrase, " ", "\r\n");
    }

    return terminator != NULL;
}

/* [] END OF FILE */

