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
File        : FS.h
Purpose     : Define global functions and types to be used by an
              application using the file system.

              This file needs to be included by any module using the
              file system.
---------------------------END-OF-HEADER------------------------------
*/

#ifndef _FS_H_               // Avoid recursive and multiple inclusion
#define _FS_H_

/*********************************************************************
*
*             #include Section
*
**********************************************************************
*/

#include "FS_ConfDefaults.h"        /* FS Configuration */
#include "FS_Types.h"
#include "FS_Storage.h"
#include "FS_Dev.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*             #define constants
*
**********************************************************************
*/

/* File system version */
#define FS_VERSION                32203UL

#define FS_ERROR_ALLOC                  (1)
#define FS_ERROR_NOMEMORY               (2)
#define FS_ERROR_ILLEGAL_FORMAT_STRING  (3)
#define FS_ERROR_UNKNOWN_DEVICE         (4)

/* Global error codes */
#define FS_ERR_OK                 0
#define FS_ERR_EOF                  -1
#define FS_ERR_DISKFULL             -2
#define FS_ERR_INVALIDPAR           -3
#define FS_ERR_CMDNOTSUPPORTED      -4
#define FS_ERR_WRITEONLY            -5
#define FS_ERR_READONLY             -6
#define FS_ERR_READERROR            -7
#define FS_ERR_WRITEERROR           -8
#define FS_ERR_FILE_ALREADY_OPENED  -9

/* Global constants*/
/* File Positions */
#define FS_SEEK_CUR               1
#define FS_SEEK_END               2
#define FS_SEEK_SET               0

#define FS_FILE_CURRENT           FS_SEEK_CUR
#define FS_FILE_END               FS_SEEK_END
#define FS_FILE_BEGIN             FS_SEEK_SET

/*********************************************************************
*
*       I/O commands for driver
*/
#define FS_CMD_REQUIRES_FORMAT        1003
#define FS_CMD_GET_DEVINFO            1004
#define FS_CMD_FORMAT_LOW_LEVEL       1005            /* Used internally by FS_FormatLow() to command the driver to perform low-level format */
#define FS_CMD_FREE_SECTORS           1006            /* Used internally: Allows the FS layer to inform driver about free sectors */
#define FS_CMD_SET_DELAY              1007            /* Used in the simulation to simulate a slow device with RAM driver */

#define FS_CMD_UNMOUNT                1008            /* Used internally by FS_STORAGE_Unmount() to inform the driver. Driver invalidates caches and all other information about medium. */
#define FS_CMD_UNMOUNT_FORCED         1009            /* Used internally by FS_STORAGE_UnmountForced() to inform the driver about an unforced remove of the device. */
                                                      /* Driver invalidates caches and all other information about medium. */
#define FS_CMD_SYNC                   1010            /* Tells the driver to clean caches. Typically, all dirty sectors are written */
#define FS_CMD_UNMOUNT_VOLUME         FS_CMD_UNMOUNT  // Obsolete: FS_CMD_UNMOUNT shall be used instead of FS_CMD_UNMOUNT_VOLUME.

#define FS_CMD_DEINIT                 1011

/*********************************************************************
*
*       Directory entry attribute definitions
*/
#define FS_ATTR_READ_ONLY    0x01
#define FS_ATTR_HIDDEN       0x02
#define FS_ATTR_SYSTEM       0x04
#define FS_ATTR_ARCHIVE      0x20
#define FS_ATTR_DIRECTORY    0x10

/*********************************************************************
*
*       Directory entry attribute definitions
*/
#define FS_FILETIME_CREATE    0
#define FS_FILETIME_ACCESS    1
#define FS_FILETIME_MODIFY    2

/*********************************************************************
*
*       Volume mount flags
*/
#define FS_MOUNT_R            1
#define FS_MOUNT_RW           3

/*********************************************************************
*
*       File system selection
*/
#define FS_FAT                0
#define FS_EFS                1

/*********************************************************************
*
*       CheckDisk error codes
*/
enum {
  FS_ERRCODE_0FILE = 0x10,
  FS_ERRCODE_SHORTEN_CLUSTER,
  FS_ERRCODE_CROSSLINKED_CLUSTER,
  FS_ERRCODE_FEW_CLUSTER,
  FS_ERRCODE_CLUSTER_UNUSED,
  FS_ERRCODE_CLUSTER_NOT_EOC,
  FS_ERRCODE_INVALID_CLUSTER,
  FS_ERRCODE_INVALID_DIRECTORY_ENTRY
};

/*********************************************************************
*
*       FS_MTYPE
*
*  Ids to distinguish different message types
*/
#define FS_MTYPE_INIT       (1 << 0)
#define FS_MTYPE_API        (1 << 1)
#define FS_MTYPE_FS         (1 << 2)
#define FS_MTYPE_STORAGE    (1 << 3)
#define FS_MTYPE_JOURNAL    (1 << 4)
#define FS_MTYPE_CACHE      (1 << 5)
#define FS_MTYPE_DRIVER     (1 << 6)
#define FS_MTYPE_OS         (1 << 7)
#define FS_MTYPE_MEM        (1 << 8)

/*********************************************************************
*
*       Global data types
*
**********************************************************************
*/

typedef void FS_BUSY_LED_CALLBACK(U8 OnOff);

struct FS_DIRENT {
  char  DirName[FS_MAX_PATH];
  U8    Attributes;
  U32   Size;
  U32   TimeStamp;
};

typedef struct {
  U32 Cluster;                    /* Cluster of current sector */
  U32 FirstCluster;
  U32 DirEntryIndex;              /* Directory entry index (first directory entry has index 0 */
  U32 ClusterIndex;
} FS_DIR_POS;

typedef struct {
  FS_DIR_POS        DirPos;             /* current position in file     */
  U16               DirEntryIndex;
  U32               FirstCluster;
  FS_VOLUME xdata * pVolume;
  U8                InUse;              /* handle in use mark           */
} FS__DIR;

typedef struct {
  U8     Attributes;
  U32    CreationTime;
  U32    LastAccessTime;
  U32    LastWriteTime;
  U32    FileSize;
  char * sFileName;
  int    SizeofFileName;
} FS_DIRENTRY_INFO;

typedef struct {
  U8     Attributes;
  U32    CreationTime;
  U32    LastAccessTime;
  U32    LastWriteTime;
  U32    FileSize;
  char * sFileName;
  // Private elements. Not be used by the application
  int SizeofFileName;
  FS__DIR Dir;
} FS_FIND_DATA;

typedef struct {
  U32 NumTotalClusters;
  U32 NumFreeClusters;
  U16 SectorsPerCluster;
  U16 BytesPerSector;
} FS_DISK_INFO;

 typedef struct {
  U16        SectorsPerCluster;
  U16        NumRootDirEntries;    /* Proposed, actual value depends on FATType */
  FS_DEV_INFO xdata * pDevInfo;
} FS_FORMAT_INFO;

typedef struct {
  U16           SectorsPerCluster;
  U16           NumRootDirEntries;    /* Proposed, actual value depends on FATType */
  U16           NumReservedSectors;
  U8            UpdatePartition;
  FS_DEV_INFO xdata * pDevInfo;
} FS_FORMAT_INFO_EX;

typedef struct {
  U16 Year;
  U16 Month;
  U16 Day;
  U16 Hour;
  U16 Minute;
  U16 Second;
} FS_FILETIME;

typedef enum {
  FS_WRITEMODE_SAFE,
  FS_WRITEMODE_MEDIUM,
  FS_WRITEMODE_FAST
} FS_WRITEMODE;

/*********************************************************************
*
*       Global function prototypes
*
**********************************************************************
*/

/*********************************************************************
*
*       "Standard" file I/O functions
*/
FS_FILE xdata *  FS_FOpen (const char * pFileName, const char * pMode);
int              FS_FClose(FS_FILE xdata * pFile);
U16              FS_FRead (      void xdata * pData, U16 Size, U16 N, FS_FILE xdata * pFile);
U16              FS_FWrite(const void       * pData, U16 Size, U16 N, FS_FILE xdata * pFile);

/*********************************************************************
*
*       Non-standard file I/O functions
*/
U16              FS_Read (FS_FILE xdata * pFile,       void xdata * pData, U16 NumBytes);
U16              FS_Write(FS_FILE xdata * pFile, const void       * pData, U16 NumBytes);

/*********************************************************************
*
*       File pointer handling
*/
int              FS_FSeek       (FS_FILE xdata * pFile, I32 Offset, int Origin);
int              FS_SetEndOfFile(FS_FILE xdata * pFile);
I32              FS_FTell       (FS_FILE xdata * pFile);

#define FS_GetFilePos(pFile)                                FS_FTell(pFile)
#define FS_SetFilePos(pFile, DistanceToMove, MoveMethod)    FS_FSeek(pFile, DistanceToMove, MoveMethod)

/*********************************************************************
*
*       I/O error handling
*/
int              FS_FEof        (FS_FILE xdata * pFile);
I16              FS_FError      (FS_FILE xdata * pFile);
void             FS_ClearErr    (FS_FILE xdata * pFile);

const char code * FS_ErrorNo2Text(int ErrCode);

/*********************************************************************
*
*       File functions
*/
int              FS_CopyFile   (const char * sSource,       const char * sDest);
U32              FS_GetFileSize(FS_FILE xdata * pFile);
int              FS_Move       (const char * sExistingName, const char * sNewName);
int              FS_Remove     (const char * pFileName);
int              FS_Rename     (const char * sOldName,      const char * sNewName);
int              FS_Truncate   (FS_FILE xdata * pFile,      U32          NewSize);
int              FS_Verify     (FS_FILE xdata * pFile,      const void * pData, U16 NumBytes);

/*********************************************************************
*
*       IOCTL
*/
int              FS_IoCtl(const char * pDevName, I32 Cmd, I32 Aux, void * pBuffer);

/*********************************************************************
*
*       Volume related functions
*/
int               FS_GetVolumeName       (int Index, char xdata * pBuffer, int MaxSize);
U32               FS_GetVolumeSize       (const char * sVolume);
U32               FS_GetVolumeSizeKB     (const char * sVolume);
U32               FS_GetVolumeFreeSpace  (const char * sVolume);
U32               FS_GetVolumeFreeSpaceKB(const char * sVolume);
int               FS_GetNumVolumes       (void);
FS_VOLUME xdata * FS_AddDevice           (const FS_DEVICE_TYPE code * pDevType);
void              FS_Unmount             (const char * sVolume);
int               FS_Mount               (const char * sVolName);
int               FS_MountEx             (const char * sVolume, U8 MountType);
int               FS_GetVolumeInfo       (const char * sVolume, FS_DISK_INFO xdata * pInfo);
int               FS_IsVolumeMounted     (const char * sVolumeName);
int               FS_GetVolumeLabel      (const char * sVolume,       char xdata * pVolumeLabel, unsigned VolumeLabelSize);
int               FS_SetVolumeLabel      (const char * sVolume, const char       * pVolumeLabel);
void              FS_CleanVolume         (const char * sVolume);
int               FS_IsLLFormatted       (const char * sVolume);
int               FS_IsHLFormatted       (const char * sVolume);
int               FS_FormatLLIfRequired  (const char * sVolume);
void              FS_UnmountForced       (const char * sVolume);
void              FS_SetAutoMount        (const char * sVolume, U8 MountType);
void              FS_UnmountLL           (const char * sVolume);
int               FS_GetVolumeStatus     (const char * sVolume);
FS_VOLUME xdata * FS_FindVolume          (const char * sVolume);
void              FS_RemoveDevice        (const char * sVolume);
int               FS_Sync                (const char * sVolume);
void              FS_SetFileWriteMode    (FS_WRITEMODE WriteMode);

/*********************************************************************
*
*       Journaling / transaction safety
*/
void FS_JOURNAL_Begin  (const char * sVolume);
void FS_JOURNAL_End    (const char * sVolume);
int  FS_JOURNAL_Create (const char * sVolume, U32 NumBytes);


#define FS_CreateJournal(sVolume, NumBytes)    FS_JOURNAL_Create(sVolume, NumBytes)
#define FS_BeginTransaction(sVolume)           FS_JOURNAL_Begin(sVolume)
#define FS_EndTransaction(sVolume)             FS_JOURNAL_End(sVolume)

/*********************************************************************
*
*       FS_Attrib
*/
int              FS_SetFileAttributes(const char * pName, U8 Attributes);
U8               FS_GetFileAttributes(const char * pName);

/*********************************************************************
*
*       FS_Time
*/
void             FS_FileTimeToTimeStamp(const FS_FILETIME * pFileTime, U32 *         pTimeStamp);
int              FS_GetFileTime        (const char *        pName,     U32 *         pTimeStamp);
int              FS_GetFileTimeEx      (const char *        pName,     U32 *         pTimeStamp, int Index);
int              FS_SetFileTime        (const char *        pName,     U32            TimeStamp);
int              FS_SetFileTimeEx      (const char *        pName,     U32            TimeStamp, int Index);
void             FS_TimeStampToFileTime(U32                 TimeStamp, FS_FILETIME xdata * pFileTime);

/*********************************************************************
*
*       FS_GetNumFilesOpen
*/
int      FS_GetNumFilesOpen(void);

/*********************************************************************
*
*       File system directory functions
*/
int  FS_CreateDir      (const char * sDirPath);
int  FS_MkDir          (const char * pDirName);
int  FS_RmDir          (const char * pDirName);
char FS_FindFirstFile  (FS_FIND_DATA xdata * pfd, const char * sPath, char * sFilename, int sizeofFilename);
char FS_FindNextFile   (FS_FIND_DATA xdata * pfd);
void FS_FindClose      (FS_FIND_DATA xdata * pfd);
U32  FS_FAT_GrowRootDir(const char * sVolume, U32 NumAddEntries);

/*********************************************************************
*
*       Olsolete directory functions
*/

void     FS_DirEnt2Attr   (FS_DIRENT xdata * pDirEnt, U8   xdata * pAttr);
void     FS_DirEnt2Name   (FS_DIRENT xdata * pDirEnt, char xdata * pBuffer);
U32      FS_DirEnt2Size   (FS_DIRENT xdata * pDirEnt);
U32      FS_DirEnt2Time   (FS_DIRENT xdata * pDirEnt);
U32      FS_GetNumFiles   (FS_DIR    xdata * pDir);

FS_DIR xdata *    FS_OpenDir    (const char   * pDirName);
int               FS_CloseDir   (FS_DIR xdata * pDir);
FS_DIRENT xdata * FS_ReadDir    (FS_DIR xdata * pDir);
void              FS_RewindDir  (FS_DIR xdata * pDir);

/*********************************************************************
*
*       File system control functions
*/
void     FS_Init          (void);
#if FS_SUPPORT_DEINIT
void     FS_DeInit        (void);
#endif

/*********************************************************************
*
*       Formatting
*/
int              FS_FormatLLIfRequired(const char * sVolumeName);
int              FS_FormatLow         (const char * sVolumeName);
int              FS_Format            (const char * sVolumeName, FS_FORMAT_INFO xdata * pFormatInfo);
int              FS_FormatSD          (const char * sVolumeName);

/*********************************************************************
*
*       Configuration functions.
*/

// General runtime configuration functions
void             FS_SetMaxSectorSize      (unsigned MaxSectorSize);
void             FS_ConfigUpdateDirOnWrite(char OnOff);

/*********************************************************************
*
*       File buffer related functions.
*/
#if FS_USE_FILE_BUFFER
  #define FS_FILE_BUFFER_WRITE   (1 << 0)
  void FS_ConfigFileBufferDefault(int BufferSize, int Flags);           // Configures default file buffer size and flags for all files which will be opened. Should only be called once, in FS_X_AddDevices()
  void FS_ConfigFileBufferFlags  (FS_FILE xdata * pFile, int Flags);    // Is only allowed to be called immediately after a FS_FOpen(), in order to change the buffer flags of a specific file
#endif

/*********************************************************************
*
*       Space information functions
*/
#define FS_GetFreeSpace(sVolume)  FS_GetVolumeFreeSpace(sVolume)
#define FS_GetTotalSpace(sVolume) FS_GetVolumeSize(sVolume)

/*********************************************************************
*
*       BusyLED support
*/
void FS_SetBusyLEDCallback(const char * sVolumeName, FS_BUSY_LED_CALLBACK * pfBusyLEDCallback);

/*********************************************************************
*
*       Device Drivers
*
*/
struct FS_DEVICE_TYPE {      // Note: This definition is really intern and should be in "FS_Int.h". In order to avoid problems with old compilers, keep it here.
  const char code * (*pfGetName)      (U8 Unit);
  int               (*pfAddDevice)    (void);     // Called from AddDevice. Usually the first call to the driver
  int               (*pfRead)         (U8 Unit, U32 SectorNo,       void xdata * pBuffer, U16 NumSectors);
  int               (*pfWrite)        (U8 Unit, U32 SectorNo, const void xdata * pBuffer, U16 NumSectors, U8 RepeatSame);
  int               (*pfIoCtl)        (U8 Unit, I32 Cmd, I32 Aux, void * pBuffer);
  int               (*pfInitMedium)   (U8 Unit);                                                       // Called when formatting or mounting a device
  int               (*pfGetStatus)    (U8 Unit);
  int               (*pfGetNumUnits)  (void);
};

extern const FS_DEVICE_TYPE code   FS_MMC_SPI_Driver;       /* MMC_DRIVER CardMode function table */

/*********************************************************************
*
*       MMC/SD driver
*/
typedef struct {
  U8 aData[16];
} MMC_CARD_ID;

void FS_MMC_ActivateCRC        (void);
void FS_MMC_DeactivateCRC      (void);
int  FS_MMC_GetCardId          (U8 Unit, MMC_CARD_ID xdata * pCardId);
void FS_MMC_CM_Allow4bitMode   (U8 Unit, U8 OnOff);

/*********************************************************************
*
*       Memory allocation functions
*/
void xdata * FS_Alloc         (I32     NumBytes);
void xdata * FS_AllocZeroed   (I32     NumBytes);
void         FS_Free          (void xdata *  p);
void         FS_AllocZeroedPtr(void * xdata* pp, I32 NumBytes);
void xdata * FS_TryAlloc      (I32     NumBytesReq);

/*********************************************************************
*
*       Debug support
*/
void   FS_X_Log     (const char *s);
void   FS_X_Warn    (const char *s);
void   FS_X_ErrorOut(const char *s);

/*********************************************************************
*
*       Application supplied functions FS_X_...
*/
void   FS_X_Free(void * p);
void   FS_X_Panic(int ErrorCode);
void   FS_X_AddDevices(void);
U32    FS_X_GetTimeDate(void);

U32    FS_GetMaxSectorSize(void);
void   FS_AssignMemory    (U32 xdata * pMem, U32 NumBytes);

void     FS_SetDefFileBufferSize(U32 BufferSize);
unsigned FS_GetDefFileBufferSize(void);
void     FS_SetFileBufferSize(FS_FILE * pFile, U32 BufferSize);

/*********************************************************************
*
*       Compatibility defines
*/
#define FS_WriteUpdateDir(OnOff) FS_ConfigUpdateDirOnWrite(OnOff)

#if defined(__cplusplus)
  }              /* Make sure we have C-declarations in C++ programs */
#endif

#include "FS_Int.h"
#endif                        // Avoid recursive and multiple inclusion

/*************************** End of file ****************************/
