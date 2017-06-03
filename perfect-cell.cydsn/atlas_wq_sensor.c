#include "atlas_wq_sensor.h"
#include <stdio.h>
#include <stdlib.h>

int atlas_sensor_sleep(uint8 sensor_address){
    uint8 com[] = "Sleep";
    uint8 COMMAND_BUFFER=7;
    int iter;
    
    for(iter=0; iter<5;iter++){
    while(I2C_MasterWriteBuf(sensor_address, com, COMMAND_BUFFER, I2C_MODE_COMPLETE_XFER) & I2C_MSTAT_WR_CMPLT){}}
    return 1;
}

int atlas_sensor_wake(uint8 sensor_address){
    uint8 com[] = "W";
    uint8 COMMAND_BUFFER=7;
    int iter;
    
    for(iter=0; iter<5;iter++){
    while(I2C_MasterWriteBuf(sensor_address, com, COMMAND_BUFFER, I2C_MODE_COMPLETE_XFER) & I2C_MSTAT_WR_CMPLT){}}
    return 1;
}

uint8 atlas_take_single_reading(uint8 sensor_address, float *reading){
    uint8 com[] = "R";
    uint8 COMMAND_BUFFER=7;
    uint8 temp_var;
    int write_iter, read_iter;
    int delay=1000;
    uint8 raw_reading[8u]={0};
//    float reading = -9999.0;
    char *reading_start;
    
    for(write_iter=0; write_iter<5;write_iter++){
    while(I2C_MasterWriteBuf(sensor_address, com, COMMAND_BUFFER, I2C_MODE_COMPLETE_XFER) & I2C_MSTAT_WR_CMPLT){}
    CyDelay(delay);
    while(I2C_MasterReadBuf(sensor_address, raw_reading, COMMAND_BUFFER, I2C_MODE_COMPLETE_XFER) & I2C_MSTAT_RD_CMPLT){};
    for(read_iter=0;read_iter<100;read_iter++)
    {
        if (I2C_MasterGetReadBufSize()==7){break;}
        else{while(I2C_MasterReadBuf(sensor_address, raw_reading, COMMAND_BUFFER, I2C_MODE_COMPLETE_XFER) & I2C_MSTAT_RD_CMPLT){};}
    }}
    reading_start = strchr(raw_reading, 1);
    if (reading_start == NULL){
        return 0u;
    }
    
    *reading = strtof(reading_start+1, NULL);
    
    memset(raw_reading, 0u, sizeof(raw_reading));
    return 1u;
}

uint8 atlas_take_con_reading(con_reading *reading){
    uint8 com[] = "R";
    uint8 sensor_address=CONDUCTIVITY;
    uint8 COMMAND_BUFFER=19;
    int read_iter, write_iter;
    int delay=1000;
    uint8 raw_reading[19u]={0};
//    con_reading reading;
    char *reading_start;
    char *ec;
    char *tds;
    char *sal;
    char *sg;
    
    for(write_iter=0; write_iter<5;write_iter++){
    while(I2C_MasterWriteBuf(sensor_address, com, COMMAND_BUFFER, I2C_MODE_COMPLETE_XFER) & I2C_MSTAT_WR_CMPLT){}
    CyDelay(delay);
    while(I2C_MasterReadBuf(sensor_address, raw_reading, COMMAND_BUFFER, I2C_MODE_COMPLETE_XFER) & I2C_MSTAT_RD_CMPLT){};
    for(read_iter=0;read_iter<100;read_iter++)
    {
        if (I2C_MasterGetReadBufSize()==19){break;}
        else{while(I2C_MasterReadBuf(sensor_address, raw_reading, COMMAND_BUFFER, I2C_MODE_COMPLETE_XFER) & I2C_MSTAT_RD_CMPLT){};}
    }}
    reading_start = strchr(raw_reading, 1);
    // This should probably be redone to write to an input rather than return struct
    if (reading_start == NULL){
        return 0u;
    }
    else {
        ec = strtok(reading_start+1, ",");
        tds = strtok(NULL, ",");
        sal = strtok(NULL, ",");
        sg = strtok(NULL, ",");
    }
    
    (*reading).ec = atof(ec);
    (*reading).sal = atof(sal);
    (*reading).tds = atof(tds);
    (*reading).sg = atof(sg);
    return 1u;
}

/*
wq_reading atlas_wq_reading(){
    wq_reading temp;
    temp.dissloved_oxygen = atlas_take_single_reading(DO);
    temp.temperature = atlas_take_single_reading(TEMPERATURE);
    temp.orp = atlas_take_single_reading(ORP);
    temp.ph = atlas_take_single_reading(PH);
    con_reading temp_con;
    temp_con = atlas_take_con_reading(CONDUCTIVITY);
    temp.ec = temp_con.ec;
    temp.sal = temp_con.sal;
    temp.tds = temp_con.tds;
    temp.sg = temp_con.sg;
    return temp;
};
*/

/* [] END OF FILE */
