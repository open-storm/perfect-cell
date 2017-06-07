#include <project.h>
#include "misc.h"

int move_valve(int valve);
int test_valve();
float32 read_Valve_POS();
uint8 zip_valve(char *labels[], float readings[], uint8 *array_ix, int *valve_trigger, uint8 max_size);
uint8 zip_valve_2(char *labels[], float readings[], uint8 *array_ix, int *valve_2_trigger, uint8 max_size);

/* [] END OF FILE */
