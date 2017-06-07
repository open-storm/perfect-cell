#include <device.h>

float32 read_vbat();
uint8 zip_vbat(char *labels[], float readings[], uint8 *array_ix, uint8 max_size);
uint8   blink_LED(uint8 n_blinks);
void    init_pins();

/* [] END OF FILE */
