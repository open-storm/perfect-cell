/**
 * @file updater.h
 * @brief Implements functions for parsing influxdb responses
 * @author Brandon Wong
 * @version TODO
 * @date 2017-06-19
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "updater.h"

int parse(char *search_str, char *in_str, char *out_str)
{
    char *parsed;
    char *start;
    char *end;
    int str_len;
    
    parsed = strstr(in_str, search_str);
    if (parsed){
        start = parsed + strlen(search_str);
        end = strstr(start, "&");
	    if (start){
	        memset(out_str, '\0', strlen(out_str));
            if (end){
                str_len = end - start;
                strncpy(out_str, start, str_len);
                out_str[str_len] = '\0';
            }
            else {
                strcpy(out_str, start);
                out_str[strlen(start)] = '\0';
            }
		    return 1;
	    }
	    
        else{
            return 0;
	    }
    }
    
    else{
        return 0;
    }
}

int intparse(int *param, char *search_str, char *in_str, char *out_str)
{
    
	if (parse(search_str, in_str, out_str))
	{
	    *param = strtol(out_str, NULL, 10);
		return 1;
	}
   
    else
	{
	return 0;
	}
}

int uintparse(uint8 *param, char *search_str, char *in_str, char *out_str)
{
    
	if (parse(search_str, in_str, out_str))
	{
	    *param = strtoul(out_str, NULL, 10);
		return 1;
	}
   
    else
	{
	return 0;
	}
}

int floatparse(float *param, char *search_str, char *in_str, char *out_str)
{
    
	if (parse(search_str, in_str, out_str))
	{
	    *param = strtof(out_str, NULL);
		return 1;
	}
   
    else
	{
	return 0;
	}
}

/*
Searches InfluxDB json packet, "packet", for name and 
returns the associated value as a char array.
Returns 1 if successful, and 0 if there was an error.

This is ported from packet_get_value() from packet.c in kLabUM/IoT
*/
uint8 parse_influxdb(char* value, char* packet, char* name) {
    char *a, *b;
       
    // Find the first occurence of "name" in "packet"
    a = strstr(packet,name);
    if (a == NULL) {

        return 0u;
    }
    
    // Move the pointer to the timestamp after the first occurence of "name"
    // This marks where the value is
    a = strstr(a,"Z\",")+strlen("Z\",");
    if (a == NULL) {

        return 0u;
    }
    
    // Find the closing bracket that follows the timestamp, marking the end of the value
    b = strstr(a,"]");
    if (b == NULL) {

        return 0u;
    }
    
    strncpy(value,a,b-a);   
    value[b-a] = '\0';
    return 1u;
}


/*
Searches InfluxDB json-packet, "packet" for "name" and 
stores the associated value as string in "param".
Removes the quotations that wrap the strings returned in the query.
Returns 1 if successful, and 0 if there was an error.
*/
uint8 strparse_influxdb(char* param, char* packet, char* name) {
    char value_str[100] = {'\0'}; // assume a string no longer than 100 bytes is stored
    
    if( parse_influxdb(value_str, packet, name) == 1u ) {
        if ( strstr(value_str,"null") == NULL) {
            
            // Copy the result into "param"
            strncpy(param,value_str+1,strlen(value_str)-2);
            param[strlen(value_str)-2] = '\0';
            
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
uint8 intparse_influxdb(int* param, char* packet, char* name) {
    char value_str[20] = {'\0'}; // note, int is at most 5 digits long
    
    if( parse_influxdb(value_str, packet, name) ) {
        *param = (int) strtol(value_str,(char **) NULL, 10); // Base 10
        // http://www.cplusplus.com/reference/cstdlib/strtol/
        return 1u;
    }
    
    return 0u;
}

uint8 clear_str(char* str){
    memset(str,'\0',strlen(str));
    return 1u;
}

/* [] END OF FILE */
