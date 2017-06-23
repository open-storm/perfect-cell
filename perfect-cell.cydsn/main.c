/**
 * @file main.c
 *
 * @brief Contains the main "operating system" for the sensor node.
 * At a recurring interval, this program:
 * - "wakes" the board from sleep mode,
 * - takes measurements using the attached sensors,
 * - connects to the cellular network,
 * - sends sensor data to a web server,
 * - updates device metadata and onboard parameters,
 * - triggers attached actuators, and
 * - puts the board back into sleep mode.
 *
 * @author Brandon Wong, Matt Bartos, Abhiramm Mullapudi, Branko Kerkez, Ivan Mondragon
 * @version TODO
 * @date 2017-05-31
 */
#include <device.h>
#include <project.h>
#include "modem.h"
#include "autosampler.h"
#include "data.h"
#include "extern.h"
#include "startup.h"
// Uncomment to use the SERVICES script to create requests
// #include "services.h"

// define global variables
#define NVARS 25

// Arrays for request strings
char body[MAX_PACKET_LENGTH] = {'\0'};
char socket_dial_str[100] = {'\0'};
char send_str[MAX_PACKET_LENGTH] = {'\0'};
char response_str[MAX_PACKET_LENGTH] = {'\0'};

// Arrays for holding sensor data and labels
char *labels[NVARS];
float readings[NVARS];

// Sleeptimer variables
uint8 awake = 1u;
uint32 wakeup_interval_counter = 0u;
uint8 connection_counter = 0u;

// Misc variables
int status;
uint8 data_sent;
uint8 ssl_initialized = 0u;
char meid[20] = {'\0'};
int numFilled = 0;

// Misc function declarations
void clear_all_arrays(uint8 clear_readings_and_labels);

CY_ISR_PROTO(Wakeup_ISR);
CY_ISR(Wakeup_ISR)
{
    /***************************************************************************
    * This function must always be called (when the Sleep Timer's interrupt
    * is disabled or enabled) after wake up to clear the ctw_int status bit.
    * It is required to call SleepTimer_GetStatus() within 1 ms (1 clock cycle
    * of the ILO) after CTW event occurred.
    ***************************************************************************/
    SleepTimer_GetStatus();
}

void main()
{

    CyGlobalIntEnable; /* Uncomment this line to enable global interrupts. */
	CyDelay(5u);  // Short delay to make sure device is ready for low power entry

	sleep_isr_StartEx(Wakeup_ISR); // Start Sleep ISR
	SleepTimer_Start();			   // Start SleepTimer Compnent	

	// Initialize Pins
    init_pins();

    // TODO: Implement generic sensor unit tests here?
    //// Test the valve
    test_valve();
    blink_LED(4u);
    
    // Update influxdb tags
    status = append_tags(main_tags, "commit_hash", CURRENT_COMMIT);

    // Update metadata (node_id, user, pass, database
    if (modem_startup(&connection_attempt_counter)){
        // Initialize SSL if enabled
        initialize_ssl(&ssl_enabled, &ssl_initialized);
        // Update metadata if enabled
        status = run_meta_subroutine(meid, send_str, response_str, 1u);
        modem_shutdown();
    }

    // Blink the LED to indicate the board is awake and about to
    // initialize loop
    blink_LED(4u);

    // Initialize loop
    for(;;)
	{
		if ( awake ){
			
			// Reset arrays
			clear_all_arrays(1u);

            // Start up sensors that need to remain on continuously
            counter_sensor_initialize();

            // Take readings and fill output arrays with labels and values

            numFilled = take_readings(labels, readings, &array_ix, 0u, NVARS);

            // Connect to network
			if (modem_startup(&connection_attempt_counter)) {
                
                // Get modem connection attempts
                if ( modem_flag == 1u ){
                    zip_modem(labels, readings, &array_ix, NVARS);
                }
                
                // Update triggers for autosampler and valve
                status = update_triggers(body, send_str, response_str);
                                    
                // Update device metadata
                status = run_meta_subroutine(meid, send_str, response_str, 1u);

			    // Reset arrays
                clear_all_arrays(0u);


                // Execute triggers and fill output arrays with labels and values
                numFilled += execute_triggers(labels, readings, &array_ix, NVARS);
                // Reset array index for next pass through
                array_ix = 0u;

                // Send readings to remote endpoint
                data_sent = send_readings(body, send_str, response_str, socket_dial_str,
                                          labels, readings, NVARS);

                if (!data_sent){
                	// Check & update the database the node should be writing to
                    status = run_meta_subroutine(meid, send_str, response_str, 1u);
                }
                else {								
					// Reset the connection attempt counter because data was successfully sent
					connection_attempt_counter = 0;

                    // Reset the counter for the optical rain sensor because data was successfully sent
                    optical_rain_reset_count();
                }
				
                // Check the signal quality before closing the connection
				modem_check_signal_quality(&rssi, &fer);  // This should only be checked while the modem is on/while not idle				

				// Reset arrays
			    clear_all_arrays(1u);
				
				// Update parameters
				update_params(body, send_str, response_str);

                // Every 100 connects, check meta database for updates
                connection_counter++;
                if (connection_counter % 100 == 0){
                    status = run_meta_subroutine(meid, send_str, response_str, 1u);
                    connection_counter = 0;
                }
				
			}
			
			modem_shutdown();
			awake = 0u; // COMMENT TO SKIP GOING TO SLEEP
		}

		/* If not ready, update the counter for the sleep timer */
		else {
			
			/* Prepares system clocks for the Sleep mode */
	        CyPmSaveClocks();

	        do
	        {
	            /*******************************************************************
	            * Switch to the Sleep Mode for the other devices:
	            *  - PM_SLEEP_TIME_NONE: wakeup time is defined by Sleep Timer
	            *  - PM_SLEEP_SRC_CTW :  wakeup on CTW sources is allowed
	            *******************************************************************/
	            CyPmSleep(PM_SLEEP_TIME_NONE, PM_SLEEP_SRC_CTW);


	            /***********************************************************************
	            * After the device is woken up the Sleep Timer's ISR is executed.
	            * Afterwards the CyPmSleep() execution is finished the clock
	            * configuration is restored.
	            ***********************************************************************/
	            if(sleeptimer == wakeup_interval_counter)
	            {

	                wakeup_interval_counter = 0u;
					awake = 1u;
	            }
	            else
	            {
	                wakeup_interval_counter++;
	            }
	
	        } while (wakeup_interval_counter != 0u);
	

	        /* Restore clock configuration */
	        CyPmRestoreClocks();
		}
		
    }
	
}

void clear_all_arrays(uint8 clear_readings_and_labels){
    if (clear_readings_and_labels){
        memset(labels, '\0', sizeof(labels));
        memset(readings, 0, sizeof(readings));
    }
	memset(socket_dial_str, '\0', sizeof(socket_dial_str));
	memset(body, '\0', sizeof(body));
	memset(send_str, '\0', sizeof(send_str));
	memset(response_str, '\0', sizeof(response_str));
}

/* [] END OF FILE */
