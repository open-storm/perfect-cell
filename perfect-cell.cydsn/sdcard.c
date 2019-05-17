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
 * @file sdcard.c
 * @brief  Write to an SD card.
 * @author Brandon Wong
 * @version 0.0.01
 * @date 2019-05-15
 */
#include <stdio.h>
#include "sdcard.h"

// Preallocate card
char    dir[10]      = {'\0'};
char    filename[30] = "Filename.txt";
char    filemode[5]  = "a+";

void SD_init()
{
    return FS_Init();  
    
    /* Uncomment to enable support for long file names
     * Note: To use long file names (LFN) support on PSoC 5LP devices, you must call
     * FS_FAT_SupportLFN(). For PSoC 3 devices, this feature is enabled by default.
     */
    //FS_FAT_SupportLFN();    
}

/**
 * @brief Make a directory on the SD card
 *
 * @param dir_name is the name of the directory to create
 *
 * @return 1u on success, 0u otherwise
 */
uint8 SD_mkdir(char* dir_name)
{
    uint8 status = 9; // initialize to a value not 0 or -1
    
    // Make a directory for the node using its node_id //
    // We are using the "No Long File Name (LFN)" library,
    //  so names are limited to 8 characters and 3 bytes for extensions
    // Turn on the SD Card
    SD_ON_Write(1u);
    
    sprintf(dir,"%.8s",dir_name); 
    status = (uint8) ( 1+FS_MkDir(dir) );
    // The arithmetic makes it 0 if a failure, 1 if successful
    
    // Turn off the SD Card
    SD_ON_Write(0u);   
    
    return status;
}

/**
 * @brief Write data to the SD card
 *
 * @param fileName is the name of the file to write to
 * @param pMode is the write mode
 * @param pData is the data to be written
 *
 * @return 1u on success, 0u otherwise
 */
uint8 SD_write(const char * fileName, const char * pMode, const void * pData)
{
    uint8 status = 9u; // initialize to a value not 0 or 1
    FS_FILE *pFile;

    pFile = FS_FOpen(fileName, pMode);
    
    // If the SD card doesn't open, try reinitializing the filesystem
    if (pFile == 0){
        FS_DeInit();
        FS_Init();
        pFile = FS_FOpen(fileName, pMode);
    }
    
    // Note: We are using the "No Long File Name (LFN)" library,
    //  so names are limited to 8 characters and 3 bytes for extensions
    if (pFile != 0) {
        FS_Write(pFile, pData, strlen(pData));
        status = (uint8) ( -1*FS_FClose(pFile) )+1;
        // FS_FClose returns 1 if there's a failure, 0 if successful
        // The arithmetic makes it 0 if a failure, 1 if successful
    }
    
    return status;
}
/* [] END OF FILE */
