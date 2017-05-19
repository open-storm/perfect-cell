#include <project.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "cytypes.h"


struct {
    float dielectric;
    float temp;
    int   conductivity;
    uint8 valid;
	uint8 err;
}typedef DecagonGS3;


DecagonGS3 Decagon_Take_Reading();
DecagonGS3 Decagon_Convert_Raw_Reading(char* raw_D);


/* [] END OF FILE */
