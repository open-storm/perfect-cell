#ifndef AUTOSAMPLER_H
#define AUTOSAMPLER_H
#endif

#include <project.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

#define AUTOSAMPLER_STATE_OFF       0
#define AUTOSAMPLER_STATE_IDLE      1
#define AUTOSAMPLER_STATE_BUSY      3
#define MAX_BOTTLE_COUNT            24
#define PULSE_COUNT                 20

extern uint8    autosampler_state;
extern uint8    SampleCount;

uint8 autosampler_start();
uint8 autosampler_stop();
uint8 autosampler_power_on();
uint8 autosampler_power_off();
uint8 autosampler_take_sample(uint8 *count);
uint8 zip_autosampler(char *labels[], float readings[], uint8 *array_ix, int *autosampler_trigger, uint8 *bottle_count, uint8 max_size);

/* [] END OF FILE */
