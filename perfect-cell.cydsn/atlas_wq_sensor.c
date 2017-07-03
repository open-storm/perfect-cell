/**
 * @file atlas_wq_sensor.c
 * @brief Implements functions for Atlas water quality sensors
 * @author Abhiramm Mullapudi and Matt Bartos
 * @version TODO
 * @date 2017-06-19
 */
#include "atlas_wq_sensor.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h> // Sleep function header file

int atlas_sensor_sleep(uint8 sensor_address){
    uint8 com[] = "Sleep";
    uint8 COMMAND_BUFFER=7;
    int iter;
    for(iter=0; iter<5;iter++){
    I2C_MasterWriteBuf(sensor_address, com, COMMAND_BUFFER, I2C_MODE_COMPLETE_XFER);
    }
    return 1;
}

int atlas_sensor_wake(uint8 sensor_address){
    uint8 com[] = "W";
    uint8 COMMAND_BUFFER=7;
    int iter;
    // Variables to prevent soft-brick of wq sensors
    time_t end_wait;
    time_t start_wait = time(NULL);
    time_t seconds = 120; // 2 Min of trial time(chosen arbitarly)
    end_wait = start_wait + seconds;

    for(iter=0; iter<5;iter++){
    while(I2C_MasterWriteBuf(sensor_address, com, COMMAND_BUFFER, I2C_MODE_COMPLETE_XFER) & I2C_MSTAT_WR_CMPLT){
    sleep(1);
    start_wait=time(NULL);
    if(start_wait > end_wait){break;}
    }}
    return 1;
}

uint8 atlas_take_single_reading(uint8 sensor_address, float *reading){
    uint8 com[] = "R";
    uint8 COMMAND_BUFFER=7;
    uint8 temp_var;
    int write_iter, read_iter;
    int delay=1000;
    uint8 raw_reading[8u]={0};
    //  float reading = -9999.0;
    char *reading_start;

    // Variables to prevent soft-brick of wq sensors
    time_t end_wait;
    time_t start_wait = time(NULL);
    time_t seconds = 120; // 2 Min of trial time(chosen arbitarly)
    end_wait = start_wait + seconds;

    for(write_iter=0; write_iter<5;write_iter++){
    while(I2C_MasterWriteBuf(sensor_address, com, COMMAND_BUFFER, I2C_MODE_COMPLETE_XFER) & I2C_MSTAT_WR_CMPLT){
    sleep(1);
    start_wait=time(NULL);
    if(start_wait > end_wait){break;}
    }

    CyDelay(delay);
    while(I2C_MasterReadBuf(sensor_address, raw_reading, COMMAND_BUFFER, I2C_MODE_COMPLETE_XFER) & I2C_MSTAT_RD_CMPLT){
    sleep(1);
    start_wait=time(NULL);
    if(start_wait > end_wait){break;}
    };

    for(read_iter=0;read_iter<100;read_iter++)
    {
        if (I2C_MasterGetReadBufSize()==7){break;}
        else{while(I2C_MasterReadBuf(sensor_address, raw_reading, COMMAND_BUFFER, I2C_MODE_COMPLETE_XFER) & I2C_MSTAT_RD_CMPLT){

        sleep(1);
        start_wait=time(NULL);
        if(start_wait > end_wait){break;}
            };}
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
    // Variables to prevent soft-brick of wq sensors
    time_t end_wait;
    time_t start_wait = time(NULL);
    time_t seconds = 120; // 2 Min of trial time(chosen arbitarly)

    end_wait = start_wait + seconds;
    char *reading_start;
    char *ec;
    char *tds;
    char *sal;
    char *sg;

    for(write_iter=0; write_iter<5;write_iter++){
    while(I2C_MasterWriteBuf(sensor_address, com, COMMAND_BUFFER, I2C_MODE_COMPLETE_XFER) & I2C_MSTAT_WR_CMPLT){
    sleep(1);
    start_wait=time(NULL);
    if(start_wait > end_wait){break;}
    }
    CyDelay(delay);
    while(I2C_MasterReadBuf(sensor_address, raw_reading, COMMAND_BUFFER, I2C_MODE_COMPLETE_XFER) & I2C_MSTAT_RD_CMPLT){
    sleep(1);
    start_wait=time(NULL);
    if(start_wait > end_wait){break;}
    };
    for(read_iter=0;read_iter<100;read_iter++)
    {
        if (I2C_MasterGetReadBufSize()==19){break;}
        else{while(I2C_MasterReadBuf(sensor_address, raw_reading, COMMAND_BUFFER, I2C_MODE_COMPLETE_XFER) & I2C_MSTAT_RD_CMPLT){
        sleep(1);
        start_wait=time(NULL);
        if(start_wait > end_wait){break;}
        };}
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

uint8 zip_atlas_wq(char *labels[], float readings[], uint8 *array_ix, uint8 max_size){
    // Ensure we don't access nonexistent array index
    uint8 nvars = 8;
    if(*array_ix + nvars >= max_size){
        return *array_ix;
    }
        uint8 valid;
        con_reading atlas_conductivity = {-9999, -9999, -9999, -9999};
        float atlas_water_temp = -9999;
        float atlas_do = -9999;
        float atlas_orp = -9999;
        float atlas_ph = -9999;

        WQ_Power_Write(1u);
        WQ_Power_1_Write(1u);
        WQ_Power_2_Write(1u);
        CyDelay(1000);
        I2C_Wakeup();
        I2C_Start();
        CyDelay(500);
        // Execute readings
        atlas_take_single_reading(TEMPERATURE, &atlas_water_temp);
        CyDelay(100);
        atlas_take_single_reading(DO, &atlas_do);
        CyDelay(100);
        atlas_take_single_reading(ORP, &atlas_orp);
        CyDelay(100);
        atlas_take_single_reading(PH, &atlas_ph);
        CyDelay(100);
        atlas_take_con_reading(&atlas_conductivity);
        CyDelay(100);

        // Fill labels
        labels[*array_ix] = "atlas_water_temp";
		labels[*array_ix + 1] = "atlas_dissolved_oxygen";
		labels[*array_ix + 2] = "atlas_orp";
        labels[*array_ix + 3] = "atlas_ph";
        labels[*array_ix + 4] = "atlas_ec";
        labels[*array_ix + 5] = "atlas_tds";
        labels[*array_ix + 6] = "atlas_sal";
        labels[*array_ix + 7] = "atlas_sg";

        // Fill reading array
        readings[*array_ix] = atlas_water_temp;
		readings[*array_ix + 1] = atlas_do;
		readings[*array_ix + 2] = atlas_orp;
        readings[*array_ix + 3] = atlas_ph;
        readings[*array_ix + 4] = atlas_conductivity.ec;
        readings[*array_ix + 5] = atlas_conductivity.tds;
        readings[*array_ix + 6] = atlas_conductivity.sal;
        readings[*array_ix + 7] = atlas_conductivity.sg;
        I2C_Sleep();
        CyDelay(100);
        WQ_Power_Write(0u);
        WQ_Power_1_Write(0u);
        WQ_Power_2_Write(0u);
        (*array_ix) += 8;
        return *array_ix;
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
