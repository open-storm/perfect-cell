#include "sleep.h"
#include "device.h"

CY_ISR(Wakeup_ISR) {
    // This function must always be called (when the Sleep Timer's interrupt
    // is disabled or enabled) after wake up to clear the ctw_int status bit.
    // It is required to call SleepTimer_GetStatus() within 1 ms (1 clock cycle
    // of the ILO) after CTW event occurred.
    SleepTimer_GetStatus();
}

void initialize_sleeptimer(){
    sleep_isr_StartEx(Wakeup_ISR);  // Start Sleep ISR
    SleepTimer_Start();             // Start SleepTimer Component
}

void go_to_sleep(int sleeptimer, uint8 *awake){
    int wakeup_interval_counter = 0u;
    
    // Prepares system clocks for the Sleep mode
    CyPmSaveClocks();
    
    do {
        // Switch to the Sleep Mode for the other devices:
        //  - PM_SLEEP_TIME_NONE: wakeup time is defined by Sleep Timer
        //  - PM_SLEEP_SRC_CTW :  wakeup on CTW sources is allowed
        CyPmSleep(PM_SLEEP_TIME_NONE, PM_SLEEP_SRC_CTW);

        // After the device is woken up the Sleep Timer's ISR is
        // executed.
        // Afterwards the CyPmSleep() execution is finished the clock
        // configuration is restored.
        if (sleeptimer == wakeup_interval_counter) {
            wakeup_interval_counter = 0u;
                *awake = 1u;
        } else {
        wakeup_interval_counter++;
        }

    } while (wakeup_interval_counter != 0u);

    // Restore clock configuration
    CyPmRestoreClocks();
}

/* [] END OF FILE */
