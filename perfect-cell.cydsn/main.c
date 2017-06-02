#include <device.h>
#include <project.h>
#include "modem.h"
#include "autosampler.h"
#include "data.h"
#include "extern.h"
#include "atlas_wq_sensor.h"
//#include "ssl.h"
// Uncomment to use the SERVICES script to create requests
// #include "services.h"

// define global variables
#define NVARS 14

// Arrays for request strings
char body[MAX_PACKET_LENGTH] = {'\0'};
char socket_dial_str[100] = {'\0'};
char send_str[MAX_PACKET_LENGTH] = {'\0'};
char response_str[MAX_PACKET_LENGTH] = {'\0'};

// Arrays for holding sensor data and labels
char *labels[NVARS];
float readings[NVARS];

// Sleeptimer variables
uint8 awake;
uint32 wakeup_interval_counter;
uint8 connection_counter;

// Misc variables
int status, ss;
uint8 data_sent;
uint8 ssl_initialized = 0u;
uint8 enable_ssl_config = 1u;
uint8 enable_ssl_sec_config = 1u;
char meid[20] = {'\0'};

// Misc function declarations
void clear_all_arrays();

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
    
    // Initialize sleeptimer variables
    awake			= 1u;
	wakeup_interval_counter = 0u;

	// Initialize Pins
    init_pins();
    	
	// Initialize other variables
    int numFilled = 0;
    
    //// Test the valve
    test_valve();
    blink_LED(4u);
    
    // Power WQ
    WQ_Power_Write(1u);
    
    int testvar = 1;
    // Update metadata (node_id, user, pass, database
    if (modem_startup(&connection_attempt_counter)){
        
        blink_LED(2u);
        // Initialize SSL if enabled
        if (!ssl_enabled){
            modem_ssl_toggle(ssl_enabled);
        }
        else { //TODO change back to ssl_enabled
            ssl_initialized = ssl_init(enable_ssl_sec_config, 
                                       enable_ssl_config);
            // If SSL initialization fails, fall back to unsecured connection
            if (!ssl_initialized){
                testvar = 0;
                ssl_enabled = 0;
            }
        }
        
        blink_LED(2u);        
        modem_get_meid(meid);
        status = update_meta(meid, send_str, response_str);
        ss = modem_get_socket_status();
        modem_shutdown();
    }
    
    // Construct new write route based on user, pass and database
    construct_route(write_route, "write", user, pass, database);

    // Blink the LED to indicate the board is awake and about to 
    // initialize loop
    blink_LED(4u);
    
    // Initialize loop
    for(;;)
	{
		if ( awake ){
			
			// Reset arrays
			clear_all_arrays();
            
            // Enable I2C
            I2C_Start();
            int test;
            wq_reading temp;
            con_reading conair;
            float orp_reading;
            float temperature_reading;
            for(test=0;test<10;test++)
            {
            orp_reading = atlas_take_single_reading(ORP);
            CyDelay(100);
            temperature_reading = atlas_take_single_reading(TEMPERATURE);
            CyDelay(100);
            conair = atlas_con_reading();
            CyDelay(100);
//            conair = atlas_con_reading();
            }
            
            // Turn on optical rain sensor if needed
            if ( (!optical_rain_pwr_Read()) && (optical_rain_flag) ) {
	            optical_rain_start();
            } else if ( (optical_rain_pwr_Read()) && (!optical_rain_flag) ) {
	            optical_rain_stop();
            }
						
			if (modem_startup(&connection_attempt_counter)) {
                                
                // Update triggers for autosampler and valve
                status = update_triggers(body, send_str, response_str);
                
                if (meta_trigger){
                    modem_get_meid(meid);
                    status = update_meta(meid, send_str, response_str);
                    construct_route(write_route, "write", user, pass, database);
                }
                            
			    // Reset arrays
                clear_all_arrays();
                
                // Take readings and fill output arrays with labels and values
                numFilled = take_readings(labels, readings, 0u);  
                
                // If not using SERVICES use the following code
			    // Construct the data body
                construct_default_body(body, labels, readings, NVARS);
			    // Construct POST request
			    construct_generic_request(send_str, body, main_host, write_route,
				                          main_port, "POST", "Close",
				                          "", 0, "1.1");
				
                // This sends the data
                modem_socket_dial(socket_dial_str, main_host, main_port, 1, ssl_enabled);
				data_sent = modem_send_recv(send_str, response_str, 0, ssl_enabled);
                modem_socket_close(ssl_enabled);
                
                if (!data_sent){
                	// Check & update the database the node should be writing to
                    modem_get_meid(meid);
                    update_meta(meid, send_str, response_str);
                    construct_route(write_route, "write", user, pass, database);
                } else {								    
					// Reset the connection attempt counter because data was successfully sent
					connection_attempt_counter = 0;
                    
                    // Reset the counter for the optical rain sensor because data was successfully sent
                    optical_rain_reset_count();
                }
				
                // Check the signal quality before closing the connection
				modem_check_signal_quality(&rssi, &fer);  // This should only be checked while the modem is on/while not idle				

				// Reset arrays
			    clear_all_arrays();
				
				// Update parameters
				update_params(body, send_str, response_str);
                
                // Every 100 connects, check meta database for updates
                connection_counter++;
                if (connection_counter % 100 == 0){
                    modem_get_meid(meid);
                    update_meta(meid, send_str, response_str);
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

void clear_all_arrays(){
    memset(labels, '\0', sizeof(labels));
    memset(readings, 0, sizeof(readings));
	memset(socket_dial_str, '\0', sizeof(socket_dial_str));
	memset(body, '\0', sizeof(body));
	memset(send_str, '\0', sizeof(send_str));
	memset(response_str, '\0', sizeof(response_str));
}

/* [] END OF FILE */
