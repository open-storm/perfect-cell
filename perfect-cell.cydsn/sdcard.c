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
char    SD_dir[10]      = {'\0'};
char    SD_body[3000]   = {'\0'};
char    SD_filename[30] = "Filename.txt";
char    SD_filemode[5]  = "a+";

void SD_init()
{
    FS_Init();  
    emFile_Sleep();
    return;
    
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
    
    // Wakeup the SD Card component
    emFile_Wakeup();
    
    // Make a directory for the node using its node_id //
    // We are using the "No Long File Name (LFN)" library,
    //  so names are limited to 8 characters and 3 bytes for extensions
    // Turn on the SD Card
    SD_ON_Write(1u);
    
    sprintf(SD_dir,"%.8s",dir_name); 
    status = (uint8) ( 1+FS_MkDir(SD_dir) );
    // The arithmetic makes it 0 if a failure, 1 if successful
    
    // Turn off the SD Card
    SD_ON_Write(0u);   
    
    // Sleep the SD Card component
    emFile_Sleep();
    
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

    // Wakeup the SD Card component
    emFile_Wakeup();
    
    // Turn on the SD Card
    SD_ON_Write(1u);
    
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
    
    // Turn off the SD Card
    SD_ON_Write(0u);  
    
    // Sleep the SD Card component
    emFile_Sleep();
    
    return status;
}
/* [] END OF FILE */
