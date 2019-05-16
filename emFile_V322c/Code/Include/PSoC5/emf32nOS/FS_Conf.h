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
File        : FS_Conf.h 
Purpose     : Simple File system configuration
---------------------------END-OF-HEADER------------------------------
*/

#ifndef _FS_CONF_H_
#define _FS_CONF_H_

#define FS_DEBUG_LEVEL              0        /* 0: Smallest code, 5: Full debug. See chapter 9 "Debugging". */
#define FS_SUPPORT_CACHE            0
#define FS_USE_FILE_BUFFER          0        // Enables file buffering, which makes file access faster and code bigger & increases ram usage
#define FS_OS_LOCKING				0
#define FS_NUM_VOLUMES 				4
#define FS_MMC_MAXUNIT				4
#define FS_SUPPORT_BUSY_LED			0
#define FS_SUPPORT_FREE_SECTOR      0
#define FS_FAT_SUPPORT_FAT32		1
#define FS_FAT_USE_FSINFO_SECTOR    1
#define FS_FAT_OPTIMIZE_DELETE      0
#define FS_FAT_FWRITE_UPDATE_DIR    0
#define FS_DRIVER_ALIGNMENT         1
#define FS_SUPPORT_JOURNAL          0
#define FS_NO_CLIB                  1
#define FS_SUPPORT_DEINIT			1

#endif  /* Avoid multiple inclusion */


