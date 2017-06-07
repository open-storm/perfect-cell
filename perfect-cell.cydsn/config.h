// Default node info
#define DEFAULT_NODE_ID "example_node"

// Default user info
#define DEFAULT_HOME_USER   "home_user"
#define DEFAULT_HOME_PASS   "home_pass"
#define DEFAULT_HOME_DB     "HOME_DB"
#define DEFAULT_HOME_PORT   8086
#define DEFAULT_HOME_HOST   "ec2-52-87-156-130.compute-1.amazonaws.com"

// Meta user info
#define DEFAULT_META_USER   "meta_user"
#define DEFAULT_META_PASS   "meta_pass"
#define DEFAULT_META_DB     "META_DB"
#define DEFAULT_META_PORT   8086
#define DEFAULT_META_HOST   "ec2-52-87-156-130.compute-1.amazonaws.com"

// Other InfluxDB parameters
#define DEFAULT_GLOBAL_TAGS "source=node"

// Modem parameters
#define MAX_CONNECTION_ATTEMPTS 5

// SSL parameters
#define SSL_ENABLED            1u
#define ENABLE_SSL_CONFIG      1u
#define ENABLE_SSL_SEC_CONFIG  1u

// GPS
#define MIN_SATELLITES    5
#define MAX_GPS_TRIES     6

// Global parameters
#define SLEEPTIMER        460u

// Flags to activate devices
#define MODEM_FLAG        1u
#define META_FLAG         1u
#define VBAT_FLAG         1u
#define GPS_FLAG          1u
#define ULTRASONIC_FLAG   0u
#define ULTRASONIC_2_FLAG 0u
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

/* [] END OF FILE */
