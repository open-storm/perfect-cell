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
File        : FS_Storage.h
Purpose     : Define global functions and types to be used by an
              application using the storage API.

              This file needs to be included by any module using the
              storage API.
---------------------------END-OF-HEADER------------------------------
*/

#ifndef _FS_STORAGE_H_               // Avoid recursive and multiple inclusion
#define _FS_STORAGE_H_

/*********************************************************************
*
*             #include Section
*
**********************************************************************
*/

#include "FS_ConfDefaults.h"        /* FS Configuration */
#include "FS_Types.h"
#include "FS_Dev.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif


#ifdef FS_STORAGE_C
  #define EXTERN
#else
  #define EXTERN extern
#endif


/*********************************************************************
*
*             #define constants
*
**********************************************************************
*/

/*********************************************************************
*
*       Media states
*/
#define FS_MEDIA_NOT_PRESENT        0
#define FS_MEDIA_IS_PRESENT         1
#define FS_MEDIA_STATE_UNKNOWN      2


#define FS_OPERATION_READ           0
#define FS_OPERATION_WRITE          1

/*********************************************************************
*
*       Data structures
*/
struct FS_DEV_INFO {
  U16 NumHeads;          /* Relevant only for mechanical drives   */
  U16 SectorsPerTrack;   /* Relevant only for mechanical drives   */
  U32 NumSectors;        /* Total number of sectors on the medium */
  U16 BytesPerSector;    /* Number of bytes per sector            */
};


typedef void (FS_ONDEVICEACTIVITYHOOK)(FS_DEVICE * pDevice, unsigned Operation, U32 StartSector, U32 NumSectors, int Sectortype);




/*********************************************************************
*
*       Global function prototypes
*
**********************************************************************
*/

/*********************************************************************
*
*       Volume related functions
*/
int              FS_IsLLFormatted     (const char * sVolume);
int              FS_FormatLLIfRequired(const char * sVolume);
void             FS_UnmountForced     (const char * sVolume);
int              FS_GetVolumeStatus   (const char * sVolume);
FS_VOLUME *      FS_FindVolume        (const char * sVolume);

/*********************************************************************
*
*       File system control functions
*/
#define FS_InitStorage()                                    FS_STORAGE_Init()
#define FS_ReadSector(sVolume, pData, SectorIndex)          FS_STORAGE_ReadSector(sVolume,  pData, SectorIndex)
#define FS_WriteSector(sVolume, pData, SectorIndex)         FS_STORAGE_WriteSector(sVolume, pData, SectorIndex)
#define FS_UnmountLL(sVolume)                               FS_STORAGE_Unmount(sVolume)
#define FS_CleanVolume(sVolume)                             FS_STORAGE_Sync(sVolume)
#define FS_GetDeviceInfo(sVolume, pDevInfo)                 FS_STORAGE_GetDeviceInfo(sVolume, pDevInfo)

int      FS_STORAGE_GetDeviceInfo(const char * sVolume, FS_DEV_INFO * pDevInfo);
unsigned FS_STORAGE_Init         (void);
int      FS_STORAGE_ReadSector   (const char * sVolume,       void * pData, U32 SectorIndex);
int      FS_STORAGE_WriteSector  (const char * sVolume, const void * pData, U32 SectorIndex);
int      FS_STORAGE_ReadSectors  (const char * sVolume,       void * pData, U32 FirstSector, U32 NumSectors);
void     FS_STORAGE_Sync         (const char * sVolume);
void     FS_STORAGE_Unmount      (const char * sVolume);
void     FS_STORAGE_UnmountForced(const char * sVolume);
int      FS_STORAGE_WriteSectors (const char * sVolume, const void * pData, U32 FirstSector, U32 NumSectors);
void     FS_SetOnDeviceActivityHook(const char * sVolume, FS_ONDEVICEACTIVITYHOOK * pfOnDeviceActivityHook);


/*********************************************************************
*
*       Storage statistics
*/
typedef struct {
  U32 ReadOperationCnt;       // Number of "Read sector operation" calls
  U32 ReadSectorCnt;          // Number of sectors read (before cache). Note that this can be (and typically is) higher than the number of "Read sector operation" counts, since one operation can request multiple sectors (in a burst)
  U32 ReadSectorCachedCnt;    // Number of sectors read from cache.
  U32 WriteOperationCnt;      // Number of "Write sector operation" calls
  U32 WriteSectorCnt;         // Number of sectors written (before cache). Note that this can be (and typically is) higher than the number of "Write sector operation" counts, since one operation can write multiple sectors (in a burst)
} FS_STORAGE_COUNTERS;

void FS_STORAGE_GetCounters  (FS_STORAGE_COUNTERS * pStat);
void FS_STORAGE_ResetCounters(void);

#if FS_SUPPORT_DEINIT
void     FS_STORAGE_DeInit(void);
#endif

EXTERN FS_STORAGE_COUNTERS FS_STORAGE_Counters;

#undef EXTERN

#if defined(__cplusplus)
  }              /* Make sure we have C-declarations in C++ programs */
#endif

#endif                        // Avoid recursive and multiple inclusion

/*************************** End of file ****************************/
