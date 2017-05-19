#include <project.h>

int parse(char *search_str, char *in_str, char *out_str);                     // parse in_str for search_str and write to out_str
int intparse(int *param, char *search_str, char *in_str, char *out_str);      // parse in_str for search_str and write int to param
int uintparse(uint8 *param, char *search_str, char *in_str, char *out_str);    // parse in_str for search_str and write uint to param
int floatparse(float *param, char *search_str, char *in_str, char *out_str);  // parse in_str for search_str and write float to param
uint8 parse_influxdb(char* value, char* packet, char* name);
uint8 strparse_influxdb(char* param, char* packet, char* name);
uint8 intparse_influxdb(int* param, char* packet, char* name);
uint8 clear_str(char* str);

/* [] END OF FILE */
