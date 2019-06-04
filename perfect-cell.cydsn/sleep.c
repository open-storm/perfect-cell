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
    
    /*
    USBUART_Suspend();
    I2C_Sleep();
    
    emFile_Sleep();
    
    Telit_UART_Sleep();
    Telit_ControlReg_Sleep();
    
    VBAT_ADC_Sleep();
    
    Senix_Comp_Sleep();
    Sensors_UART_Sleep();
    mux_controller_Sleep();
    
    SDI12_UART_Sleep();
    SDI12_control_reg_Sleep();
    */
    
    // Set Drive mode to Strong Drive
    VBAT_SetDriveMode(CY_PINS_DM_STRONG);
    Valve_POS_SetDriveMode(CY_PINS_DM_STRONG);
    Valve_2_POS_SetDriveMode(CY_PINS_DM_STRONG);
    SDI12_Data_SetDriveMode(CY_PINS_DM_STRONG);
    CyPins_SetPinDriveMode(CYREG_PRT3_PC6, CY_PINS_DM_STRONG); // Telit_rx              (Hi-Z Digital)
    CyPins_SetPinDriveMode(CYREG_PRT0_PC5, CY_PINS_DM_STRONG); // toughsonic_rs485_neg  (Hi-Z Analog)
    CyPins_SetPinDriveMode(CYREG_PRT0_PC6, CY_PINS_DM_STRONG); // toughsonic_rs485_pos  (Hi-Z Analog)
    CyPins_SetPinDriveMode(CYREG_PRT12_PC3,CY_PINS_DM_STRONG); // ultrasonic_uart_rx    (Hi-Z Digital)
    CyPins_SetPinDriveMode(CYREG_PRT12_PC7,CY_PINS_DM_STRONG); // ultrasonic_uart_tx    (Hi-Z Digital)
    
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
    
    // Reset Drive mode
    VBAT_SetDriveMode(CY_PINS_DM_ALG_HIZ);
    Valve_POS_SetDriveMode(CY_PINS_DM_ALG_HIZ);
    Valve_2_POS_SetDriveMode(CY_PINS_DM_ALG_HIZ);
    SDI12_Data_SetDriveMode(CY_PINS_DM_RES_DWN);
    CyPins_SetPinDriveMode(CYREG_PRT3_PC6, CY_PINS_DM_DIG_HIZ); // Telit_rx              (Hi-Z Digital)
    CyPins_SetPinDriveMode(CYREG_PRT0_PC5, CY_PINS_DM_ALG_HIZ); // toughsonic_rs485_neg  (Hi-Z Analog)
    CyPins_SetPinDriveMode(CYREG_PRT0_PC6, CY_PINS_DM_ALG_HIZ); // toughsonic_rs485_pos  (Hi-Z Analog)
    CyPins_SetPinDriveMode(CYREG_PRT12_PC3,CY_PINS_DM_DIG_HIZ); // ultrasonic_uart_rx    (Hi-Z Digital)
    CyPins_SetPinDriveMode(CYREG_PRT12_PC7,CY_PINS_DM_DIG_HIZ); // ultrasonic_uart_tx    (Hi-Z Digital)
   
}

/* [] END OF FILE */
