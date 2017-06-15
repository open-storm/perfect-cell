/**
 * @file data.h
 * @brief Declares the sensor subroutine, metadata updater, trigger updater, and parameter updater.
 * @author TODO
 * @version TODO
 * @date 2017-06-01
 */
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

// Function prototypes
int take_readings(char *labels[], float readings[], uint8 *array_ix, uint8 take_average, uint8 max_size);
uint8 execute_triggers(char *labels[], float readings[], uint8 *array_ix, uint8 max_size);
uint8 zip_meta(char *labels[], float readings[], uint8 *array_ix, uint8 max_size);
uint8 zip_modem(char *labels[], float readings[], uint8 *array_ix, uint8 max_size);
uint8 run_meta_subroutine(char* meid, char* send_str, char* response_str, uint8 get_device_meid);
int update_meta(char* meid, char* send_str, char* response_str);
int update_triggers(char* body, char* send_str, char* response_str);
void update_params(char* body, char* send_str, char* response_str);
void construct_default_body(char *data_packet, char *labels[], float readings[],
                        int nvars);
void construct_route(char* route, char* base, char* user, char* pass, char* database);
uint8 send_readings(char* body, char* send_str, char* response_str, char* socket_dial_str,
                    char *labels[], float readings[], uint8 nvars);
char *url_encode(char *str);

/* [] END OF FILE */
