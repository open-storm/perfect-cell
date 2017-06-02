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

int go_to_sleep(uint8 sensor_address){
    uint8 com[] = "Sleep";
    uint8 COMMAND_BUFFER=7;
    uint8 temp_var;
    int a, a1;
    int delay=1000;
    
    for(a1=0; a1<5;a1++){
    while(I2C_MasterWriteBuf(sensor_address, com, COMMAND_BUFFER, I2C_MODE_COMPLETE_XFER) & I2C_MSTAT_WR_CMPLT){}}
    return 1;
}

float atlas_take_single_reading(uint8 sensor_address){
    uint8 com[] = "R";
//    uint8 ADDRESS=96;
    uint8 COMMAND_BUFFER=7;
    uint8 temp_var;
    int a, a1;
    int delay=1000;
    uint8 reading[8u]={0};
    float temp_read = -9999.0;
    char *a2;
    char *reading_start;
    
    for(a1=0; a1<5;a1++){
    while(I2C_MasterWriteBuf(sensor_address, com, COMMAND_BUFFER, I2C_MODE_COMPLETE_XFER) & I2C_MSTAT_WR_CMPLT){}
    CyDelay(delay);
    while(I2C_MasterReadBuf(sensor_address, reading, COMMAND_BUFFER, I2C_MODE_COMPLETE_XFER) & I2C_MSTAT_RD_CMPLT){};
    for(a=0;a<100;a++)
    {
        if (I2C_MasterGetReadBufSize()==7){break;}
        else{while(I2C_MasterReadBuf(sensor_address, reading, COMMAND_BUFFER, I2C_MODE_COMPLETE_XFER) & I2C_MSTAT_RD_CMPLT){};}
    }}
    reading_start = strchr(reading, 1);
    if (reading_start != NULL){
        temp_read = strtof(reading_start+1, NULL);
    }
    memset(reading, 0u, sizeof(reading));
    return temp_read;
}

con_reading atlas_con_reading(){
    uint8 com[] = "R";
    uint8 sensor_address=CONDUCTIVITY;
    uint8 COMMAND_BUFFER=19;
    uint8 temp_var;
    int a, a1;
    int delay=1000;
    uint8 reading[19u]={0};
    char *a2;
    char *reading_start;
    char *ec;
    char *tds;
    char *sal;
    char *sg;
    
    for(a1=0; a1<5;a1++){
    while(I2C_MasterWriteBuf(sensor_address, com, COMMAND_BUFFER, I2C_MODE_COMPLETE_XFER) & I2C_MSTAT_WR_CMPLT){}
    CyDelay(delay);
    while(I2C_MasterReadBuf(sensor_address, reading, COMMAND_BUFFER, I2C_MODE_COMPLETE_XFER) & I2C_MSTAT_RD_CMPLT){};
    for(a=0;a<100;a++)
    {
        if (I2C_MasterGetReadBufSize()==19){break;}
        else{while(I2C_MasterReadBuf(sensor_address, reading, COMMAND_BUFFER, I2C_MODE_COMPLETE_XFER) & I2C_MSTAT_RD_CMPLT){};}
    }}
    reading_start = strchr(reading, 1);
    if (reading_start != NULL){
        ec = strtok(reading_start+1, ",");
        tds = strtok(NULL, ",");
        sal = strtok(NULL, ",");
        sg = strtok(NULL, ",");
    }
    con_reading temp;
    temp.ec = atof(ec);
    temp.sal = atof(sal);
    temp.tds = atof(tds);
    temp.sg = atof(sg);
    return temp;
}


wq_reading atlas_wq_reading(){
    wq_reading temp;
    temp.dissloved_oxygen = atlas_take_single_reading(DO);
    temp.temperature = atlas_take_single_reading(TEMPERATURE);
    temp.orp = atlas_take_single_reading(ORP);
    temp.ph = atlas_take_single_reading(PH);
    con_reading temp_con;
    temp_con = atlas_con_reading(CONDUCTIVITY);
    temp.ec = temp_con.ec;
    temp.sal = temp_con.sal;
    temp.tds = temp_con.tds;
    temp.sg = temp_con.sg;
    return temp;
};