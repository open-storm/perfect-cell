/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

/* [] END OF FILE */
#include "project.h"

#define CONDUCTIVITY 101
#define TEMPERATURE 96
#define DO 97
#define ORP 98
#define PH 102

struct{
    float ec;
    float tds;
    float sal;
    float sg;
}typedef con_reading;

struct{
    float temperature;
    float dissloved_oxygen;
    float orp;
    float ph;
    float ec;
    float tds;
    float sal;
    float sg;
}typedef wq_reading;

// Sensor function to take reading
int go_to_sleep(uint8 sensor_address);
float atlas_take_single_reading(uint8 sensor_address);
con_reading atlas_con_reading();
wq_reading atlas_wq_reading();

// Additional functionality