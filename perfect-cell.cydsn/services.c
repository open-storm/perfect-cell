#include <project.h>
#include <stdio.h>
#include "services.h"
#include "extern.h"

// Flags to specify service

#define XIVELY      1u
#define INFLUXDB    2u
#define THINGSPEAK  3u
#define CHORDS      4u

// Xively parameters
int XIVELY_FEED_ID = 726713202;
const char *xively_api_key = "examplekey";
const char *xively_endpoint = "api.xively.com";
const char *xively_port = "80";

// Influxdb parameters
int INFLUXDB_AUTHENTICATION = 0u;
const char *influxdb_endpoint = "example.compute.amazonaws.com";
const char *influxdb_port = "8086";
const char *influxdb_db = "TEST_DATABASE";
const char *influxdb_user = "root";
const char *influxdb_pw = "root";

// Thingspeak parameters
const char *thingspeak_endpoint = "api.thingspeak.com";
const char *thingspeak_port = "80";
const char *thingspeak_api_key = "examplekey";

// Chords parameters
const char *chords_endpoint = "storm.chordsrt.com";
const char *chords_port = "80";
const char *chords_write_key = "examplekey";

int construct_data_body(char *data_packet, char *labels[], float readings[],
                        int nvars, int service_flag){
    int i = 0; // iterator through labels and readings

    // XIVELY
    if (service_flag == 1u){
        for (i = 0; i < nvars; i++){
			if (labels[i] && labels[i][0]){
            sprintf(data_packet, "%s%s, %f\r\n",
                    data_packet, labels[i], readings[i]);
			}
        }
    }
    // INFLUXDB
    if (service_flag == 2u){
        for (i = 0; i < nvars; i++){
			if (labels[i] && labels[i][0]){
            sprintf(data_packet, "%s%s,node_id=%s,site=%s value=%f\n",
                    data_packet, labels[i],
                    node_id, site_name,
                    readings[i]);
			}
        }
    }
	
    // THINGSPEAK
    if (service_flag == 3u){
        for (i = 0; i < nvars; i++){
			if (labels[i] && labels[i][0]){
            // This could potentially be buggy because
            // we can't tie field number to the measurement label
            sprintf(data_packet, "%s%s%d%s%f", data_packet,
                    "field", i, "=", readings[i]);
			}
        }
    }
    
	// CHORDS
    if (service_flag == 4u){
        sprintf(data_packet, "%sinstrument_id=%s", data_packet, node_id);
        for (i = 0; i < nvars; i++){
			if (labels[i] && labels[i][0]){
            sprintf(data_packet, "%s&%s=%f", data_packet,
                    labels[i], readings[i]);
			}
        }
        sprintf(data_packet, "%skey=%s", data_packet, chords_write_key);
    }
    return i;
}

void construct_data_post_request(char* body, char* socket_dial_str,
	                             char* send_str, int service_flag){
    
    if (service_flag == 1u){
        // Fill socket dial string
        sprintf(socket_dial_str, "AT#SD=1,0,%s,\"%s\",0,0,1\r",
                xively_port, xively_endpoint);
        // Fill send string
        sprintf(send_str, "%s%1u%s\r\n%s%s\r\n%s%s\r\n%s%d\r\n\r\n%s\r\n\032",
                "PUT /v2/feeds/", XIVELY_FEED_ID, ".csv HTTP/1.0",
                "X-ApiKey: ", xively_api_key,
                "Host: ", xively_endpoint,
	            "Content-Length: ", strlen(body),
                body);
    }
    else if (service_flag == 2u){
        // Fill socket dial string
        sprintf(socket_dial_str, "AT#SD=1,0,%s,\"%s\",0,0,1\r",
                influxdb_port, influxdb_endpoint);
        // Fill send string
        sprintf(send_str, "%s%s%s\r\n%s%s%s%s\r\n%s%d\r\n\r\n%s\r\n\032",
                    "POST /write?db=", influxdb_db, " HTTP/1.0",
	                "Host: ", influxdb_endpoint, ":", influxdb_port,
	                "Content-Length: ", strlen(body),
	                body);
    }
    else if (service_flag == 3u){
        sprintf(socket_dial_str, "AT#SD=1,0,%s,\"%s\",0,0,1\r",
                thingspeak_port, thingspeak_endpoint);
        
	    sprintf(send_str,"%s\r\n%s%s\r\n%s\r\n%s%s\r\n%s\r\n%s%d\r\n\r\n%s\r\n\r\n\032",
	                "POST /update HTTP/1.0",
                    "Host: ", thingspeak_endpoint,
                    "Connection: close",
	                "X-THINGSPEAKAPIKEY: ", thingspeak_api_key,
                    "Content-Type: application/x-www-form-urlencoded",
                    "Content-Length: ", strlen(body),
	                body);
    }
}

/* [] END OF FILE */
