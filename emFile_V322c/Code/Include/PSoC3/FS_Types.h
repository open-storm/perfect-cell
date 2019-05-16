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
File        : FS_Int.h
Purpose     : Internals used accross different layers of the file system
---------------------------END-OF-HEADER------------------------------
*/

#ifndef _FS_TYPES_H_
#define _FS_TYPES_H_


#include "SEGGER.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif



/*********************************************************************
*
*       Defines non configurable
*
**********************************************************************
*/
#define FS_FILE_ACCESS_FLAG_A (1 << 0)
#define FS_FILE_ACCESS_FLAG_B (1 << 1)
#define FS_FILE_ACCESS_FLAG_C (1 << 2)
#define FS_FILE_ACCESS_FLAG_R (1 << 3)
#define FS_FILE_ACCESS_FLAG_W (1 << 4)
//
// Combined access modes which are frequently used
//
#define FS_FILE_ACCESS_FLAGS_AW     (FS_FILE_ACCESS_FLAG_A | FS_FILE_ACCESS_FLAG_W)
#define FS_FILE_ACCESS_FLAGS_BR     (FS_FILE_ACCESS_FLAG_B | FS_FILE_ACCESS_FLAG_R)
#define FS_FILE_ACCESS_FLAGS_CW     (FS_FILE_ACCESS_FLAG_C | FS_FILE_ACCESS_FLAG_W)
#define FS_FILE_ACCESS_FLAGS_RW     (FS_FILE_ACCESS_FLAG_R | FS_FILE_ACCESS_FLAG_W)

#define FS_FILE_ACCESS_FLAGS_ACW    (FS_FILE_ACCESS_FLAG_C | FS_FILE_ACCESS_FLAGS_AW)
#define FS_FILE_ACCESS_FLAGS_ARW    (FS_FILE_ACCESS_FLAG_A | FS_FILE_ACCESS_FLAGS_RW)
#define FS_FILE_ACCESS_FLAGS_BCW    (FS_FILE_ACCESS_FLAG_B | FS_FILE_ACCESS_FLAGS_CW)
#define FS_FILE_ACCESS_FLAGS_BRW    (FS_FILE_ACCESS_FLAG_W | FS_FILE_ACCESS_FLAGS_BR)
#define FS_FILE_ACCESS_FLAGS_CRW    (FS_FILE_ACCESS_FLAG_C | FS_FILE_ACCESS_FLAG_R | FS_FILE_ACCESS_FLAG_W)

#define FS_FILE_ACCESS_FLAGS_ABCW   (FS_FILE_ACCESS_FLAG_B | FS_FILE_ACCESS_FLAGS_ACW)
#define FS_FILE_ACCESS_FLAGS_ACRW   (FS_FILE_ACCESS_FLAG_A | FS_FILE_ACCESS_FLAGS_CRW)
#define FS_FILE_ACCESS_FLAGS_BCRW   (FS_FILE_ACCESS_FLAG_B | FS_FILE_ACCESS_FLAGS_CRW)

#define FS_FILE_ACCESS_FLAGS_ABCRW  (FS_FILE_ACCESS_FLAGS_ACRW | FS_FILE_ACCESS_FLAG_B)

//
// Partition table definitions
//
#define FS__DEV_PART_0_TABLE       0x01BE /* start of first entry */

//
// Directory entry get/set macros 
//
#define FS_DIRENTRY_GET_ATTRIBUTES             (1 << 0)
#define FS_DIRENTRY_GET_TIMESTAMP_CREATE       (1 << 1)
#define FS_DIRENTRY_GET_TIMESTAMP_MODIFY       (1 << 2)
#define FS_DIRENTRY_GET_TIMESTAMP_ACCESS       (1 << 3)
#define FS_DIRENTRY_SET_ATTRIBUTES             (1 << 0)
#define FS_DIRENTRY_SET_TIMESTAMP_CREATE       (1 << 1)
#define FS_DIRENTRY_SET_TIMESTAMP_MODIFY       (1 << 2)
#define FS_DIRENTRY_SET_TIMESTAMP_ACCESS       (1 << 3)
//
//  Smart buffer (SB) type defines
//
#define FS_SB_TYPE_DATA               FS_SECTOR_TYPE_DATA
#define FS_SB_TYPE_MANAGEMENT         FS_SECTOR_TYPE_MAN
#define FS_SB_TYPE_DIRECTORY          FS_SECTOR_TYPE_DIR

/*********************************************************************
*
*       Sector type defines
*/
#define FS_SECTOR_TYPE_DATA    0
#define FS_SECTOR_TYPE_DIR     1
#define FS_SECTOR_TYPE_MAN     2
#define FS_SECTOR_TYPE_COUNT   3   /* Number of sector types */

/*********************************************************************
*
*       Sector type defines
*/
#define FS_SECTOR_TYPE_MASK_DATA    (1 << FS_SECTOR_TYPE_DATA)
#define FS_SECTOR_TYPE_MASK_DIR     (1 << FS_SECTOR_TYPE_DIR)
#define FS_SECTOR_TYPE_MASK_MAN     (1 << FS_SECTOR_TYPE_MAN)
#define FS_SECTOR_TYPE_MASK_ALL     (FS_SECTOR_TYPE_MASK_DATA | FS_SECTOR_TYPE_MASK_DIR | FS_SECTOR_TYPE_MASK_MAN)

/*********************************************************************
*
*       Data types, opaque
*
**********************************************************************
*/
typedef struct FS_FAT_INFO    FS_FAT_INFO;
typedef struct FS_EFS_INFO    FS_EFS_INFO;
typedef struct FS_SB          FS_SB;

/*********************************************************************
*
*       Internal data types, used as opaque types for pointers
*
**********************************************************************
*/
typedef struct FS_CACHE_API     FS_CACHE_API;
typedef struct FS_DEVICE        FS_DEVICE;
typedef struct FS_DIR           FS_DIR;
typedef struct FS_FILE          FS_FILE;
typedef struct FS_PARTITION     FS_PARTITION;
typedef struct FS_VOLUME        FS_VOLUME;
typedef struct FS_DEVICE_TYPE   FS_DEVICE_TYPE;
typedef struct FS_DIRENT        FS_DIRENT;
typedef struct FS_DEV_INFO      FS_DEV_INFO;

#if defined(__cplusplus)
}                /* Make sure we have C-declarations in C++ programs */
#endif

#endif  /* _FS_INT_H_ */

/*************************** End of file ****************************/
