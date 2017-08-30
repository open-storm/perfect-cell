#include <device.h>

CY_ISR_PROTO(Wakeup_ISR);

void initialize_sleeptimer();
void go_to_sleep(int sleeptimer, uint8 *awake);

/* [] END OF FILE */
