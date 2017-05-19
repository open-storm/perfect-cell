#include <device.h>
#include <stdlib.h>
#include <string.h>

typedef struct{
    float temp;
    float depth;
    uint8 valid;
} UltrasonicReading;

void                ultrasonic_start();
void                ultrasonic_stop();
uint8               ultrasonic_power_on(uint8 which_ultrasonic);
uint8               ultrasonic_power_off(uint8 which_ultrasonic);
uint8               ultrasonic_get_reading();
void                uart_ultrasonic_string_reset();


//[] END OF FILE