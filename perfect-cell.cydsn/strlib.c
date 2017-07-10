/**
 * @file strlib.h
 * @brief Implements functions for parsing strings
 * @author Brandon Wong, Ivan Mondragon
 * @version TODO
 * @date 2017-06-19
 */

#include "strlib.h"
#include <stdarg.h>
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

uint8_t clear_str(char *str) {
    memset(str, '\0', strlen(str));
    return 1u;
}

void zips(char *begin[], char *end[], ...) {
    va_list args;
    va_start(args, end);

    while (begin < end) {
        *begin++ = va_arg(args, char *);
    }

    va_end(args);
}

void zipf(float begin[], float end[], ...) {
    va_list args;
    va_start(args, end);

    while (begin < end) {
        *begin++ = va_arg(args, double);
    }

    va_end(args);
}

char *strextract(const char input_str[], char output_str[],
                 const char search_start[], const char search_end[]) {
    if (input_str == NULL) return NULL;
    char *begin, *end = NULL;
    output_str[0] = '\0';

    if (begin = strstr(input_str, search_start)) {
        begin += strlen(search_start);
        if (end = strstr(begin, search_end)) {
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

/* [] END OF FILE */

