#include <stdlib.h>
#include <stdio.h>
#include "extern.h"
#include "device.h"


void construct_route(char* route, char* base, char* user, char* pass, char* database){
    memset(route, '\0', sizeof(*route));
    sprintf(route, "%s%s?u=%s&p=%s&db=%s", route, base, user, pass, database); 
}

void construct_influxdb_body(char *data_packet, char *labels[], float readings[],
                        int nvars){
	int i = 0; // iterator through labels and readings
	
	// Default to influxdb line protocol
    for (i = 0; i < nvars; i++){
		if (labels[i] && labels[i][0]){
            sprintf(data_packet, "%s%s,node_id=%s,%s value=%f\n",
                    data_packet, labels[i],
                    node_id, main_tags,
                    readings[i]);
		}
    }
}

uint8 append_tags(char *main_tags, char *appended_label, char *appended_value)
{
    sprintf(main_tags, "%s,%s=%s", main_tags, appended_label, appended_value);
    return 1u;
}

/* [] END OF FILE */
