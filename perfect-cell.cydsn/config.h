#ifndef CONFIG_H
#define CONFIG_H

// Service to use
#define USE_INFLUXDB        1
#define USE_CHORDS          0
    
// Default node info
#define DEFAULT_NODE_ID     "example_node"

#if USE_INFLUXDB
// Default user info
#define DEFAULT_HOME_USER   "home_user"
#define DEFAULT_HOME_PASS   "home_pass"
#define DEFAULT_HOME_DB     "HOME_DB"
#define DEFAULT_HOME_PORT   8086
#define DEFAULT_HOME_HOST   "ec2-13-58-145-29.us-east-2.compute.amazonaws.com"

// Meta user info
#define DEFAULT_META_USER   "meta_user"
#define DEFAULT_META_PASS   "meta_pass"
#define DEFAULT_META_DB     "META_DB"
#define DEFAULT_META_PORT   8086
#define DEFAULT_META_HOST   "ec2-13-58-145-29.us-east-2.compute.amazonaws.com"
    
// Other InfluxDB parameters
#define DEFAULT_GLOBAL_TAGS "source=node"
#endif


// CHORDS info
#if USE_CHORDS
#define CHORDS_HOST                "workshop.chordsrt.com"
#define CHORDS_PORT                80
#define CHORDS_INSTRUMENT_ID       1
#define CHORDS_WRITE_KEY_ENABLED   1
#define CHORDS_WRITE_KEY           "key"
#define CHORDS_IS_TEST             0
#endif

// Network parameters
#define MAX_ROUTE_SIZE          60
#define MAX_QUERY_SIZE          300

// Modem parameters
#define MAX_CONNECTION_ATTEMPTS 5
#define ALLOW_DM_UPDATES        0

// SSL parameters
#define SSL_ENABLED            1u
#define ENABLE_SSL_CONFIG      1u
#define ENABLE_SSL_SEC_CONFIG  1u

// GPS
#define MIN_SATELLITES    3
#define MAX_GPS_TRIES     6

// Global parameters
#define SLEEPTIMER        460u

// Flags to activate devices
#define CONNECTION_FLAG   1u
#define TRIGGER_FLAG      1u
#define PARAM_FLAG        1u
#define META_FLAG         1u
#define MODEM_FLAG        1u
#define VBAT_FLAG         1u
#define GPS_FLAG          1u
#define ULTRASONIC_FLAG   0u
#define ULTRASONIC_2_FLAG 0u
#define SENIX_FLAG        0u
#define OPTICAL_RAIN_FLAG 0u
#define DECAGON_FLAG      0u
#define AUTOSAMPLER_FLAG  0u
#define VALVE_FLAG        0u
#define VALVE_2_FLAG      0u
#define ATLAS_WQ_FLAG     0u

// Flags to trigger devices
#define AUTOSAMPLER_TRIGGER 0u
#define VALVE_TRIGGER       -1
#define VALVE_2_TRIGGER     -1
#define META_TRIGGER        1u
#define GPS_TRIGGER         1u

// Number of loops for each device
#define VBAT_LOOPS           1
#define ULTRASONIC_LOOPS     5
#define OPTICAL_RAIN_LOOPS   1
#define DECAGON_LOOPS        1

#endif
/* [] END OF FILE */
