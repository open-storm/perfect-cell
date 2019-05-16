/*********************************************************************
*                SEGGER MICROCONTROLLER GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 2003-2010     SEGGER Microcontroller GmbH & Co KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

**** emFile file system for embedded applications ****
emFile is protected by international copyright laws. Knowledge of the
source code may not be used to write a similar product. This file may
only be used in accordance with a license and should not be re-
distributed in any way. We appreciate your understanding and fairness.
----------------------------------------------------------------------
File    : FS_ConfDefaults.h
Purpose : File system configuration defaults
--------  END-OF-HEADER  ---------------------------------------------
*/

#ifndef _FS_CONFDEFAULT_H_
#define _FS_CONFDEFAULT_H_

#include "FS_Conf.h"
#include "SEGGER.h"


/*********************************************************************
*
*       Defaults for optimizations
*
**********************************************************************
*/

/*********************************************************************
*
*       File system defaults
*
**********************************************************************
*/
#ifndef   FS_LOG_MASK_DEFAULT     
  #define FS_LOG_MASK_DEFAULT    (1 << 0)   // FS_MTYPE_INIT
#endif

#ifndef   FS_MAX_PATH
  #define FS_MAX_PATH                 260
#endif

#ifndef   FS_SUPPORT_FAT
  #define FS_SUPPORT_FAT                1   // Support the FAT file system if enabled
#endif

#ifndef   FS_SUPPORT_EFS
  #define FS_SUPPORT_EFS                0   // Support the EFS file system if enabled
#endif

#ifndef   FS_SUPPORT_MULTIPLE_FS
  #define FS_SUPPORT_MULTIPLE_FS      ((FS_SUPPORT_EFS) && (FS_SUPPORT_FAT))
#endif

#ifndef   FS_SUPPORT_FREE_SECTOR
  #define FS_SUPPORT_FREE_SECTOR        1     // Informs lower layer of unused sectors ... Makes sense only for drivers which use it
#endif

#ifndef   FS_SUPPORT_CACHE
  #define FS_SUPPORT_CACHE              1
#endif

#ifndef   FS_MULTI_HANDLE_SAFE
  #define FS_MULTI_HANDLE_SAFE          0
#endif

#ifndef   FS_MAX_LEN_FULL_FILE_NAME
  #define FS_MAX_LEN_FULL_FILE_NAME   256
#endif

#ifndef   FS_DRIVER_ALIGNMENT
  //
  // Added for compatibility reasons.
  //
  #ifdef    FS_DRIVER_ALIGMENT  
    #define FS_DRIVER_ALIGNMENT           FS_DRIVER_ALIGMENT
  #else
    #define FS_DRIVER_ALIGNMENT           4
  #endif
#endif

#ifndef   FS_DIRECTORY_DELIMITER
  #define FS_DIRECTORY_DELIMITER     '\\'
#endif

#ifndef   FS_VERIFY_WRITE
  #define FS_VERIFY_WRITE              0    // Verify every write sector operation (tests the driver and hardware)
#endif

#ifndef   FS_SUPPORT_BUSY_LED
  #define FS_SUPPORT_BUSY_LED          1
#endif

#ifndef   FS_SUPPORT_CHECK_MEMORY
  #define FS_SUPPORT_CHECK_MEMORY      0    // Check whether the memory is accessible for device driver, otherwise burst operation are not used.
                                            // To check the memory, please make sure the the FS_SetMemAccessCallback() is called for the appropriate
                                            // volume/device.
#endif

#ifndef   FS_SUPPORT_JOURNAL
  #define FS_SUPPORT_JOURNAL           0
#endif

#ifndef   FS_OPTIMIZE
  #define FS_OPTIMIZE                       // Allow optimizations such as "__arm __ramfunc" or similar. What works best depends on target.
#endif

#ifndef   FS_SUPPORT_DEINIT
  #define FS_SUPPORT_DEINIT            0    // Allows to have a deinitialization of the file system
#endif

#ifndef   FS_SUPPORT_EXT_MEM_MANAGER
  #define FS_SUPPORT_EXT_MEM_MANAGER   0    // Set to 1 will use external alloc/free memory functions, these must be set with FS_SetMemFunc()
#endif 

#ifndef FS_JOURNAL_FILE_NAME
  #define FS_JOURNAL_FILE_NAME "Journal.dat"
#endif

#ifndef   FS_USE_FILE_BUFFER
  #define FS_USE_FILE_BUFFER           1    // Enables file buffering, which makes file access faster and code bigger & increases ram usage
#endif

#ifndef FS_HUGE
  #define FS_HUGE
#else
  #define FS_HUGE  huge
#endif

/*********************************************************************
*
*       FAT File System Layer defines
*
*  For each media in your system using FAT, the file system reserves
*  memory to keep required information of the boot sector of that media.
*  FS_MAXDEV is the number of device drivers in your system used
*  with FAT, FS_FAT_MAXUNIT is the maximum number of logical units
*  supported by one of the activated drivers.
*/

#ifndef   FS_FAT_SUPPORT_FAT32                         // 0  disables FAT32 support
  #define FS_FAT_SUPPORT_FAT32      1
#endif

#ifndef   FS_FAT_SUPPORT_UTF8
  #define FS_FAT_SUPPORT_UTF8       0                  // Use UTF-8 encoding to support Unicode characters
#endif

#ifndef   FS_UNICODE_UPPERCASE_EXT
  #define FS_UNICODE_UPPERCASE_EXT  {0x0000, 0x0000}   // allow to extend the static Unicode lower to upper case table
#endif

#ifndef   FS_MAINTAIN_FAT_COPY
  #define FS_MAINTAIN_FAT_COPY      0                  // Shall the 2nd FAT (copy) be maintained
#endif

#ifndef   FS_FAT_USE_FSINFO_SECTOR                     // Use and update FSInfo sector on FAT32 media. For FAT12/FAT116 there is no FSInfo sector
  #define FS_FAT_USE_FSINFO_SECTOR  1
#endif

#ifndef   FS_FAT_OPTIMIZE_DELETE                       // Accelerate delete of large files
  #define FS_FAT_OPTIMIZE_DELETE    1
#endif

#ifndef   FS_FAT_FWRITE_UPDATE_DIR                     // On size change, shall the directory entry of the file be updated
  #define FS_FAT_FWRITE_UPDATE_DIR  1
#endif


/*********************************************************************
*
*       EFS File System Layer defines
*
*/
#ifndef   FS_EFS_OPTIMIZE_DELETE                       // Accelerate delete of large files
  #define FS_EFS_OPTIMIZE_DELETE    1
#endif

#ifndef   FS_EFS_CASE_SENSITIVE
  #define FS_EFS_CASE_SENSITIVE     0                  // Should the file/directory names be case sensitively compared
#endif

#ifndef   FS_EFS_FWRITE_UPDATE_DIR
  #define FS_EFS_FWRITE_UPDATE_DIR  1                  // On size change, shall the directory entry of the file be updated
#endif

/*********************************************************************
*
*       CLib
*/
#ifndef   FS_NO_CLIB
  #define FS_NO_CLIB  0
#endif

/*********************************************************************
*
*       OS Layer
*
**********************************************************************
*/
#ifdef FS_OS_LOCKING
  #if   FS_OS_LOCKING == 0
    #define FS_OS               0
    #define FS_OS_LOCK_PER_DRIVER 0
  #elif FS_OS_LOCKING == 1
    #define FS_OS               1
    #define FS_OS_LOCK_PER_DRIVER 0
  #elif FS_OS_LOCKING == 2
    #define FS_OS               1
    #define FS_OS_LOCK_PER_DRIVER 1
  #else
  #error FS_OS_LOCKING has illegal value
  #endif
#endif

#ifndef   FS_OS
  #define FS_OS                 0
#endif

#ifndef   FS_OS_LOCK_PER_DRIVER
  #define FS_OS_LOCK_PER_DRIVER 0        // 0 = means a single lock for all files, 1 means one lock per file
#endif

/*********************************************************************
*
*       Device driver defaults
*
**********************************************************************
*/
/*********************************************************************
*
*       RAMDISK defines
*
*/
#ifndef   FS_RAMDISK_SECTOR_SIZE
  #define FS_RAMDISK_SECTOR_SIZE   512
#endif

/*********************************************************************
*
*       NOR flash driver defines
*/
#ifndef FS_NOR_WEARLEVELING
  #define FS_NOR_WEARLEVELING         1
#endif

#ifndef   FS_NOR_SECTOR_SIZE
  #define FS_NOR_SECTOR_SIZE         512
#endif

/* In order to avoid warnings for undefined parameters */
#ifndef FS_USE_PARA
  #if defined(NC30) || defined(NC308)
    #define FS_USE_PARA(para)
  #else
    #define FS_USE_PARA(para) para=para;
  #endif
#endif

/*********************************************************************
*
*       Number of
*/
#ifndef   FS_NUM_VOLUMES
  #define FS_NUM_VOLUMES 4
#endif

#ifndef   FS_NUM_DIR_HANDLES
  #define FS_NUM_DIR_HANDLES     1
#endif

#ifndef   FS_NUM_MEMBLOCKS_PER_OPERATION
  //
  // FAT file system needs at least 2 sector buffers for a FS operation
  //
  #if (FS_SUPPORT_EFS == 0)
    #define FS_NUM_FS_MEMBLOCKS_PER_OPERATION  2
  //
  // EFS needs at least 2 sector buffers for a FS operation
  //
  #elif (FS_SUPPORT_EFS == 1)
    #define FS_NUM_FS_MEMBLOCKS_PER_OPERATION  3
  #endif

  //
  // Check for journal support
  //
  #if FS_SUPPORT_JOURNAL 
    #define FS_NUM_MEMBLOCKS_PER_OPERATION      FS_NUM_FS_MEMBLOCKS_PER_OPERATION + 1
  #else
    #define FS_NUM_MEMBLOCKS_PER_OPERATION      FS_NUM_FS_MEMBLOCKS_PER_OPERATION
  #endif

#endif

#endif  /* Avoid multiple inclusion */

/****** End of file *************************************************/
