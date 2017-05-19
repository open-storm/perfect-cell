#include "packet.h"

char* error_str;

/*
Searches Xively packet for name and returns the associated value
as a char array.
Returns 1 if successful, and 0 if there was an error.
*/
uint8 packet_get_value(char* packet, char* name, char* value) {
    char *a, *b;
    
    // Find the first occurence of "name" in "packet"
    a = strstr(packet,name);
    if (a == NULL) {
        sprintf(error_str, "%s not found in packet: \n%s", name, packet);
        /* Write to SD Card debugger
        debug_write( error_str );
        */
        return 0u;
    }
    
    // Move the pointer to the timestamp after the first occurence of "name"
    // This marks where the value is
    a = strstr(a,"Z,")+strlen("Z,");
    if (a == NULL) {
        sprintf(error_str, "'Z,' not found in packet: \n%s", packet);
        /* Write to SD Card debugger
        debug_write( error_str );
        */
        return 0u;
    }
    
    // Find the carriage return, marking the end of the value
    b = strstr(a,"\n");
    if (b == NULL) {
        sprintf(error_str, "'\r' not found in packet: \n%s", packet);
        /* Write to SD Card debugger
        debug_write( error_str );
        */
        return 0u;
    }
    
    strncpy(value,a,b-a);   
    return 1u;
}

/*
Searches Xively packet for name and returns the associated value
as uint32.
Returns 1 if successful, and 0 if there was an error.
*/
uint8 packet_get_uint8(char* packet, char* name, uint8* value) {
    char value_str[20] = {0}; // uint8 is at most 3 digits long
    
    if( packet_get_value(packet, name, value_str) ){
        *value = (uint8) strtol(value_str,(char **) NULL, 10); // Base 10
        // http://www.cplusplus.com/reference/cstdlib/strtol/
        return 1u;
    }
        
    sprintf(error_str, "%s not found in packet: \n%s", name, packet);
    /* Write to SD Card debugger
    debug_write( error_str );
    */
    return 0u;
}

/*
Searches Xively packet for name and returns the associated value
as uint32.
Returns 1 if successful, and 0 if there was an error.
*/
uint8 packet_get_uint32(char* packet, char* name, uint32* value) {
    char value_str[20] = {0}; // uint32 is at most 10 digits long
    
    if( packet_get_value(packet, name, value_str) ){
        *value = (uint32) strtol(value_str,(char **) NULL, 10); // Base 10
        // http://www.cplusplus.com/reference/cstdlib/strtol/
        return 1u;
    }
        
    sprintf(error_str, "%s not found in packet: \n%s", name, packet);
    /* Write to SD Card debugger
    debug_write( error_str );
    */
    return 0u;
}


uint8 clear_packet(char* packet){

    memset(packet,0,strlen(packet));
    return 1u;
}
/* [] END OF FILE */