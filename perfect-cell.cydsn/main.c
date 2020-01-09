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
 * @author Brandon Wong, Matt Bartos, Abhiramm Mullapudi, Branko Kerkez,
 *         Ivan Mondragon
 * @version TODO
 * @date 2017-05-31
 */
#include <device.h>
#include <project.h>
#include "USBUART.h"
//#include "autosampler.h"
#include "data.h"
#include "extern.h"
#include "modem.h"
#include "sleep.h"
#include "startup.h"
#include "config.h"
#include "sdcard.h"

#if USE_INFLUXDB
    #include "influxdb.h"
#endif

#if USE_CHORDS
    #include "chords.h"
#endif

//#ifdef DEBUG
//#include "tests.h"
//#endif

// define global variables
#define NVARS 25

// Arrays for request strings
char body[MAX_SEND_LENGTH] = {'\0'};
char socket_dial_str[100] = {'\0'};
char send_str[MAX_SEND_LENGTH] = {'\0'};
char response_str[MAX_RECV_LENGTH] = {'\0'};
char meid[20] = {'\0'};

// Arrays for holding sensor data and labels
char *labels[NVARS];
float readings[NVARS];

// Sleeptimer variables
uint8 awake = 1u;
uint32 wakeup_interval_counter = 0u;
uint8 connection_counter = 0u;

// Misc variables
int status = 0u;
uint8 data_sent = 0u;
int numFilled = 0;
uint8 ssl_initialized = 0u;

// Misc function declarations
void clear_all_arrays(uint8 clear_readings_and_labels);

int main(void) {
    CyGlobalIntEnable; /* Uncomment this line to enable global interrupts. */
    // Short delay to make sure device is ready for low power entry
    CyDelay(5u);

    // Enable sleep mode
    initialize_sleeptimer();

    // Initialize Pins
    init_pins();
    
    // Initialize SD Card
    SD_init();
    
    // Initialize USB UART  
    /* // Doesn't seem to work over 5-pin connector
    USBUART_Start(0,USBUART_DWR_POWER_OPERATION);
    for(int timeout = 0; timeout < 200; timeout++) {
        if (USBUART_bGetConfiguration()) {
            USBUART_CDC_Init();
            USBUART_PutString("Sup \n\r");
            break;
        }
        LED_Write(!LED_Read());
        CyDelay(50); // Delay 5 milliseconds
    }
    */
    
    // Test valve pins
    test_valve();

    // Update influxdb tags
    #if USE_INFLUXDB
    status = append_tags(main_tags, "commit_hash", CURRENT_COMMIT);
    #endif
    
    // Testing the sdcard library
    //int status;
    //status = SD_mkdir(node_id);
    
    // Update metadata (node_id, user, pass, database
    if (connection_flag){
        initialize_modem_params(send_str, response_str, ssl_enabled, ssl_initialized);
    }

#ifdef DEBUG

    // TODO: Implement generic sensor unit tests here?
    // result_t results;

    // test_run_all(&results);
    // test_post_results(&results);
    // test_free_results(&results);
    // test_configuration(&results);

#endif

    // Blink the LED to indicate the board is awake and about to
    // initialize loop
    blink_LED(4u);

    // Initialize loop
    for (;;) {
        if (awake) {
            //LED_Write(!LED_Read());
            
            // Reset arrays
            clear_all_arrays(1u);

            // Start up sensors that need to remain on continuously
            counter_sensor_initialize();

            // Take readings and fill output arrays with labels and values

            numFilled = take_readings(labels, readings, &array_ix, 0u, NVARS);
            
            /*/ SDI12 testing only
            // Remember to reset SLEEPTIMER to 460u and CONNECTION_FLAG to 1u            
            clear_all_arrays(0u);
            array_ix = 0u;
            //*/
            
            // Connect to network
            if (connection_flag){
                
                if (modem_startup(&connection_attempt_counter)) {
                    // Get modem connection attempts
                    if (modem_flag == 1u) {
                        zip_modem(labels, readings, &array_ix, NVARS);
                    }

                    // Update triggers for autosampler and valve
                    status = update_triggers(body, send_str, response_str);

                    // Update device metadata
                    status = run_meta_subroutine(meid, send_str, response_str, 1u);

                    // Reset arrays
                    clear_all_arrays(0u);
    
                    // Execute triggers and fill output arrays with labels and
                    // values
                    numFilled +=
                        execute_triggers(labels, readings, &array_ix, NVARS);
                    // Reset array index for next pass through
                    array_ix = 0u;

                    // Send readings to remote endpoint
                    data_sent =
                        send_readings(body, send_str, response_str, socket_dial_str,
                                      labels, readings, NVARS);

                    if (!data_sent) {
                        // Check & update the database the node should be writing to
                        status =
                            run_meta_subroutine(meid, send_str, response_str, 1u);
                    }
                    else {
                        // Reset the connection attempt counter because data was
                        // successfully sent
                        connection_attempt_counter = 0;

                        // Reset the counter for the optical rain sensor because
                        // data was successfully sent
                        optical_rain_reset_count();
                    }

                    // Check the signal quality before closing the connection
                    modem_check_signal_quality(&rssi, &fer);  // This should only be
                                                              // checked while the
                                                              // modem is on/while
                                                              // not idle
                    // Reset arrays
                    clear_all_arrays(1u);

                    // Update parameters
                    update_params(body, send_str, response_str);

                    // Every 100 connects, check meta database for updates
                    connection_counter++;
                    if (connection_counter % 100 == 0) {
                        status =
                            run_meta_subroutine(meid, send_str, response_str, 1u);
                        connection_counter = 0;
                    }
                }
            }
            awake = 0u;  // COMMENT TO SKIP GOING TO SLEEP
        }

        /* If not ready, update the counter for the sleep timer */
        else {
            modem_shutdown();
            go_to_sleep(sleeptimer, &awake);
        }
    }
}

void clear_all_arrays(uint8 clear_readings_and_labels) {
    if (clear_readings_and_labels) {
        memset(labels, '\0', sizeof(labels));
        memset(readings, 0, sizeof(readings));
    }
    memset(socket_dial_str, '\0', sizeof(socket_dial_str));
    memset(body, '\0', sizeof(body));
    memset(send_str, '\0', sizeof(send_str));
    memset(response_str, '\0', sizeof(response_str));
}

/* [] END OF FILE */
