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

/**
 * @file sdcard.h
 * @brief Write to an SD card.
 * @author Brandon Wong
 * @version 0.0.01
 * @date 2019-05-15
 */
#ifndef SDCARD_H
#define SDCARD_H
#include <project.h>
#include <FS.h>

extern char	    dir[10];
extern char     filename[30];
char            filemode[5];

/**
 * @brief Initialize the file system for the SD card
 *
 * @param Null
 *
 * @return void
 */
void SD_init();

/**
 * @brief Make a directory on the SD card
 *
 * @param dir_name is the name of the directory to create
 *
 * @return 1u on success, 0u otherwise
 */
uint8 SD_mkdir(char* dir_name);

/**
 * @brief Write data to the SD card
 *
 * @param fileName is the name of the file to write to
 * @param pMode is the write mode
 * @param pData is the data to be written
 *
 * @return 1u on success, 0u otherwise
 */
uint8 SD_write(const char * fileName, const char * pMode, const void * pData);

#endif
/* [] END OF FILE */
