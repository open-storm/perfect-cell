#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chords.h"

int construct_chords_route(char *route, char *labels[], float readings[],
                        int nvars, int instrument_id, int write_key_enabled, 
                        char *chords_write_key, int is_test){
    sprintf(route, "%s%s", route, "measurements/url_create?");
    sprintf(route, "%sinstrument_id=%d", route, instrument_id);
    int i = 0u;
    for (i = 0; i < nvars; i++){
	    if (labels[i] && labels[i][0]){
            sprintf(route, "%s&%s=%f", route,
                    labels[i], readings[i]);
			}
        }
    if (write_key_enabled){
        sprintf(route, "%s&key=%s", route, chords_write_key);
    }
    // If this is a test measurement, append "&test"
    if (is_test){
        sprintf(route, "%s&test", route);
    }
    return 1u;
}
                        
/* [] END OF FILE */
