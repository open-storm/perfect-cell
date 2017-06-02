/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

/* [] END OF FILE */
#include "atlas_wq_sensor.h"
#include <stdio.h>
#include <stdlib.h>

float temperature_take_reading(){
    uint8 com[] = "R";
    uint8 ADDRESS=96;
    uint8 COMMAND_BUFFER=7;
    uint8 temp_var;
    int a, a1;
    int delay=600;
    uint8 reading[8u]={0};
    float temp_read;
    char *a2;
    
    for(a1=0; a1<5;a1++){
    while(I2C_MasterWriteBuf(ADDRESS, com, COMMAND_BUFFER, I2C_MODE_COMPLETE_XFER) & I2C_MSTAT_WR_CMPLT){}
    for(a=0;a<100;a++)
    {
        if (I2C_MasterGetReadBufSize()==8){break;}
        else{while(I2C_MasterWriteBuf(ADDRESS, com, COMMAND_BUFFER, I2C_MODE_COMPLETE_XFER) & I2C_MSTAT_WR_CMPLT){};}
    }
    CyDelay(delay);
    while(I2C_MasterReadBuf(ADDRESS, reading, COMMAND_BUFFER, I2C_MODE_COMPLETE_XFER) & I2C_MSTAT_RD_CMPLT){};
    for(a=0;a<100;a++)
    {
        if (I2C_MasterGetReadBufSize()==8){break;}
        else{while(I2C_MasterReadBuf(ADDRESS, reading, COMMAND_BUFFER, I2C_MODE_COMPLETE_XFER) & I2C_MSTAT_RD_CMPLT){};}
    }}
    a2 = strchr(reading, 1);
    temp_read = strtof(a2+1, NULL);
    return temp_read;
}

float do_take_reading(){
    uint8 com[] = "R";
    uint8 ADDRESS=101;
    uint8 COMMAND_BUFFER=7;
    uint8 temp_var;
    int a, a1;
    int delay=600;
    uint8 reading[8u]={0};
    float temp_read;
    char *a2;
    
    for(a1=0; a1<5;a1++){
    while(I2C_MasterWriteBuf(ADDRESS, com, COMMAND_BUFFER, I2C_MODE_COMPLETE_XFER) & I2C_MSTAT_WR_CMPLT){}
    for(a=0;a<100;a++)
    {
        if (I2C_MasterGetReadBufSize()==8){break;}
        else{while(I2C_MasterWriteBuf(ADDRESS, com, COMMAND_BUFFER, I2C_MODE_COMPLETE_XFER) & I2C_MSTAT_WR_CMPLT){};}
    }
    CyDelay(delay);
    while(I2C_MasterReadBuf(ADDRESS, reading, COMMAND_BUFFER, I2C_MODE_COMPLETE_XFER) & I2C_MSTAT_RD_CMPLT){};
    for(a=0;a<100;a++)
    {
        if (I2C_MasterGetReadBufSize()==8){break;}
        else{while(I2C_MasterReadBuf(ADDRESS, reading, COMMAND_BUFFER, I2C_MODE_COMPLETE_XFER) & I2C_MSTAT_RD_CMPLT){};}
    }}
    a2 = strchr(reading, 1);
    temp_read = strtof(a2+1, NULL);
    return temp_read;
}