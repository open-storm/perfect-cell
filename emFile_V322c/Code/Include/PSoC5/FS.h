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
*       CACHE Commands (internal)
*/
#define FS_CMD_CACHE_SET_MODE     6000L
#define FS_CMD_CACHE_CLEAN        6001L   /* Write out all dirty sectors */
#define FS_CMD_CACHE_SET_QUOTA    6002L
#define FS_CMD_CACHE_FREE_SECTORS 6003L
#define FS_CMD_CACHE_INVALIDATE   6004L   /* Invalidate all sectors in cache */

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
#define FS_FILETIME_CREATE     0
#define FS_FILETIME_ACCESS     1
#define FS_FILETIME_MODIFY     2

/*********************************************************************
*
*       Volume mount flags
*/
#define FS_MOUNT_R                  1
#define FS_MOUNT_RW                 3

/*********************************************************************
*
*       File system selection
*/
#define FS_FAT                  0
#define FS_EFS                  1

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

typedef int  FS_QUERY_F_TYPE(int ErrCode, ...);
typedef void FS_BUSY_LED_CALLBACK(U8 OnOff);
typedef int  FS_MEMORY_IS_ACCESSIBLE_CALLBACK(void * p, U32 NumBytes);

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
  FS_DIR_POS  DirPos;             /* current position in file     */
  U16         DirEntryIndex;
  U32         FirstCluster;
  FS_VOLUME * pVolume;
  I16         error;              /* error code                   */
  U8          InUse;              /* handle in use mark           */
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
  FS_DEV_INFO * pDevInfo;
} FS_FORMAT_INFO;

typedef struct {
  U16           SectorsPerCluster;
  U16           NumRootDirEntries;    /* Proposed, actual value depends on FATType */
  U16           NumReservedSectors;
  U8            UpdatePartition;
  FS_DEV_INFO * pDevInfo;
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
*             Non blocking (backgrounded) file I/O functions
*/
/* Information for background data */
typedef struct FS_BG_DATA {
  struct FS_BG_DATA * pNext;
  U32                 NumBytes;
  U32                 NumBytesRem;         /* Remaining bytes to transfer */
  void              * pData;
  FS_FILE           * pFile;
  U8                  Operation;
  void (*pfOnCompletion) (void * p);   /* Optional completion routine */
  void              * pCompletionData;     /* Optional data for completion routine */
  char                IsCompleted;
} FS_BG_DATA;

void FS_Daemon(void);
void FS_FReadNonBlock (void       * pData,
                       U32          NumBytes,
                       FS_FILE    * pFile,
                       FS_BG_DATA * pBGData,                    /* User supplied management block */
                       void (*pfOnCompletion) (void * p),   /* Optional completion routine */
                       void * pCompletionData                   /* Optional data for completion routine */
                      );
void FS_FWriteNonBlock(const void * pData,
                       U32          NumBytes,
                       FS_FILE    * pFile,
                       FS_BG_DATA * pBGData,                    /* User supplied management block */
                       void (*pfOnCompletion) (void * p),   /* Optional completion routine */
                       void * pCompletionData                   /* Optional data for completion routine */
                       );
char FS_IsCompleted(FS_BG_DATA * pBGData);

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
FS_FILE *        FS_FOpen (const char * pFileName, const char * pMode);
int              FS_FClose(FS_FILE    * pFile);
U32              FS_FRead (      void * pData, U32 Size, U32 N, FS_FILE * pFile);
U32              FS_FWrite(const void * pData, U32 Size, U32 N, FS_FILE * pFile);

/*********************************************************************
*
*       Non-standard file I/O functions
*/
U32           FS_Read (FS_FILE * pFile,       void * pData, U32 NumBytes);
U32           FS_Write(FS_FILE * pFile, const void * pData, U32 NumBytes);

/*********************************************************************
*
*       File pointer handling
*/
int              FS_FSeek       (FS_FILE * pFile, I32 Offset,         int Origin);
int              FS_SetEndOfFile(FS_FILE * pFile);
I32              FS_FTell       (FS_FILE * pFile);

#define FS_GetFilePos(pFile)                                FS_FTell(pFile)
#define FS_SetFilePos(pFile, DistanceToMove, MoveMethod)    FS_FSeek(pFile, DistanceToMove, MoveMethod)

/*********************************************************************
*
*       I/O error handling
*/
int              FS_FEof        (FS_FILE * pFile);
I16              FS_FError      (FS_FILE * pFile);
void             FS_ClearErr    (FS_FILE * pFile);
const char *     FS_ErrorNo2Text(int       ErrCode);

/*********************************************************************
*
*       File functions
*/
int              FS_CopyFile   (const char * sSource,       const char * sDest);
U32              FS_GetFileSize(FS_FILE    * pFile);
int              FS_Move       (const char * sExistingName, const char * sNewName);
int              FS_Remove     (const char * pFileName);
int              FS_Rename     (const char * sOldName,      const char * sNewName);
int              FS_Truncate   (FS_FILE    * pFile,         U32          NewSize);
int              FS_Verify     (FS_FILE    * pFile,         const void * pData, U32 NumBytes);

/*********************************************************************
*
*       IOCTL
*/
int              FS_IoCtl(const char *pDevName, I32 Cmd, I32 Aux, void *pBuffer);

/*********************************************************************
*
*       Volume related functions
*/
int              FS_GetVolumeName       (int Index, char * pBuffer, int MaxSize);
U32              FS_GetVolumeSize       (const char * sVolume);
U32              FS_GetVolumeSizeKB     (const char * sVolume);
U32              FS_GetVolumeFreeSpace  (const char * sVolume);
U32              FS_GetVolumeFreeSpaceKB(const char * sVolume);
int              FS_GetNumVolumes       (void);
FS_VOLUME *      FS_AddDevice           (const FS_DEVICE_TYPE * pDevType);
void             FS_Unmount             (const char * sVolume);
int              FS_Mount               (const char * sVolName);
int              FS_MountEx             (const char * sVolume, U8 MountType);
int              FS_GetVolumeInfo       (const char * sVolume, FS_DISK_INFO * pInfo);
int              FS_IsVolumeMounted     (const char * sVolumeName);
int              FS_GetVolumeLabel      (const char * sVolume, char * pVolumeLabel, unsigned VolumeLabelSize);
int              FS_SetVolumeLabel      (const char * sVolume, const char * pVolumeLabel);
void             FS_CleanVolume         (const char * sVolume);
int              FS_IsLLFormatted       (const char * sVolume);
int              FS_IsHLFormatted       (const char * sVolume);
int              FS_FormatLLIfRequired  (const char * sVolume);
void             FS_UnmountForced       (const char * sVolume);
void             FS_SetAutoMount        (const char * sVolume, U8 MountType);
void             FS_UnmountLL           (const char * sVolume);
int              FS_GetVolumeStatus     (const char * sVolume);
FS_VOLUME *      FS_FindVolume          (const char * sVolume);
void             FS_RemoveDevice        (const char * sVolume);
int              FS_Sync                (const char * sVolume);
void             FS_SetFileWriteMode    (FS_WRITEMODE WriteMode);

/*********************************************************************
*
*       Journaling / transaction safety
*/
void FS_JOURNAL_Begin  (const char *sVolume);
void FS_JOURNAL_End    (const char *sVolume);
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
void             FS_TimeStampToFileTime(U32                 TimeStamp, FS_FILETIME * pFileTime);


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
char FS_FindFirstFile  (FS_FIND_DATA * pfd, const char * sPath, char * sFilename, int sizeofFilename);
char FS_FindNextFile   (FS_FIND_DATA * pfd);
void FS_FindClose      (FS_FIND_DATA * pfd);
U32  FS_FAT_GrowRootDir(const char * sVolume, U32 NumAddEntries);



/*********************************************************************
*
*       Olsolete directory functions
*/

void     FS_DirEnt2Attr   (FS_DIRENT  * pDirEnt, U8   * pAttr);
void     FS_DirEnt2Name   (FS_DIRENT  * pDirEnt, char * pBuffer);
U32      FS_DirEnt2Size   (FS_DIRENT  * pDirEnt);
U32      FS_DirEnt2Time   (FS_DIRENT  * pDirEnt);
U32      FS_GetNumFiles   (FS_DIR     * pDir);

FS_DIR    * FS_OpenDir    (const char * pDirName);
int         FS_CloseDir   (FS_DIR     * pDir);
FS_DIRENT * FS_ReadDir    (FS_DIR     * pDir);
void        FS_RewindDir  (FS_DIR     * pDir);

/*********************************************************************
*
*       File system standard application
*/
void FSTask(void);

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
int              FS_Format            (const char * sVolumeName, FS_FORMAT_INFO * pFormatInfo);
int              FS_FormatSD          (const char * sVolumeName);

/*********************************************************************
*
*       CheckDisk functionality
*/
const char * FS_CheckDisk_ErrCode2Text(int ErrCode);
int          FS_CheckDisk(const char * sVolumeName, void * pBuffer, U32 BufferSize, int MaxRecursionLevel, FS_QUERY_F_TYPE * pfOnError);
void         FS_FAT_AbortCheckDisk(void);

/*********************************************************************
*
*       CheckDisk macro replacement for
*/
#define FS_EFS_CheckDisk(sVolumeName, pBuffer, BufferSize, MaxRecursionLevel, pfOnError)  FS_CheckDisk(sVolumeName, pBuffer, BufferSize, MaxRecursionLevel, pfOnError)
#define FS_FAT_CheckDisk(sVolumeName, pBuffer, BufferSize, MaxRecursionLevel, pfOnError)  FS_CheckDisk(sVolumeName, pBuffer, BufferSize, MaxRecursionLevel, pfOnError)
#define FS_EFS_CheckDisk_ErrCode2Text(ErrCode)                                            FS_CheckDisk_ErrCode2Text(ErrCode)
#define FS_FAT_CheckDisk_ErrCode2Text(ErrCode)                                            FS_CheckDisk_ErrCode2Text(ErrCode)

/*********************************************************************
*
*       Configuration functions.
*/

// General runtime configuration functions
void FS_SetMaxSectorSize(unsigned MaxSectorSize);
void FS_ConfigUpdateDirOnWrite(char OnOff);
//  FAT runtime configuration functions
void FS_FAT_SupportLFN(void);
void FS_FAT_DisableLFN(void);

/*********************************************************************
*
*       File buffer related functions.
*/
#if FS_USE_FILE_BUFFER
  #define FS_FILE_BUFFER_WRITE   (1 << 0)
  void FS_ConfigFileBufferDefault(int BufferSize, int Flags);    // Configures default file buffer size and flags for all files which will be opened. Should only be called once, in FS_X_AddDevices()
  void FS_ConfigFileBufferFlags(FS_FILE * pFile, int Flags);     // Is only allowed to be called immediately after a FS_FOpen(), in order to change the buffer flags of a specific file
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
*       Memory accessible support
*/
void FS_SetMemAccessCallback(const char * sVolumeName, FS_MEMORY_IS_ACCESSIBLE_CALLBACK * pfIsAccessibleCallback);

/*********************************************************************
*
*       Device Drivers
*
*/
struct FS_DEVICE_TYPE {      // Note: This definition is really intern and should be in "FS_Int.h". In order to avoid problems with old compilers, keep it here.
  const char *      (*pfGetName)      (U8 Unit);
  int               (*pfAddDevice)    (void);                                                       // Called from AddDevice. Usually the first call to the driver
  int               (*pfRead)         (U8 Unit, U32 SectorNo,       void *pBuffer, U32 NumSectors);
  int               (*pfWrite)        (U8 Unit, U32 SectorNo, const void *pBuffer, U32 NumSectors, U8 RepeatSame);
  int               (*pfIoCtl)        (U8 Unit, I32 Cmd, I32 Aux, void *pBuffer);
  int               (*pfInitMedium)   (U8 Unit);                                                       // Called when formatting or mounting a device
  int               (*pfGetStatus)    (U8 Unit);
  int               (*pfGetNumUnits)  (void);
};

extern const FS_DEVICE_TYPE    FS_RAMDISK_Driver;       /* RAMDISK_DRIVER function table */
extern const FS_DEVICE_TYPE    FS_WINDRIVE_Driver;      /* WINDRIVE_DRIVER function table */
extern const FS_DEVICE_TYPE    FS_MMC_CardMode_Driver;  /* MMC_DRIVER SPI function table */
extern const FS_DEVICE_TYPE    FS_MMC_SPI_Driver;       /* MMC_DRIVER CardMode function table */
extern const FS_DEVICE_TYPE    FS_IDE_Driver;           /* IDE_CF_DRIVER function table */
extern const FS_DEVICE_TYPE    FS_NOR_Driver;           /* NOR flash driver function table */
extern const FS_DEVICE_TYPE    FS_GENERIC_Driver;       /* GENERIC_DRIVER function table */
extern const FS_DEVICE_TYPE    FS_NAND_Driver;          /* Nand driver function table */

/*********************************************************************
*
*       NOR driver
*/
typedef struct {
  U32 Off;
  U32 Size;
  U32 EraseCnt;
  U16 NumUsedSectors;
  U16 NumFreeSectors;
  U16 NumEraseableSectors;
} FS_NOR_SECTOR_INFO;

typedef struct {
  U32 NumPhysSectors;
  U32 NumLogSectors;
  U32 NumUsedSectors;   /* Number of used logical sectors */
} FS_NOR_DISK_INFO;

typedef struct FS_NOR_PHY_TYPE {
  int  (*pfWriteOff)         (U8 Unit, U32 Off, const void * pSrc, U32 Len);
  int  (*pfReadOff)          (U8 Unit, void * pDest, U32 Off, U32 Len);
  int  (*pfEraseSector)      (U8 Unit, unsigned int SectorIndex);
  void (*pfGetSectorInfo)    (U8 Unit, unsigned int SectorIndex, U32 * pOff, U32 * pLen);
  int  (*pfGetNumSectors)    (U8 Unit);
  void (*pfConfigure)        (U8 Unit, U32 BaseAddr, U32 StartAddr, U32 NumBytes);
  void (*pfOnSelectPhy)      (U8 Unit);
  void (*pfDeInit)           (U8 Unit);
} FS_NOR_PHY_TYPE;

typedef void (FS_NOR_READ_CFI_FUNC)(U8 Unit, U32 BaseAddr, U32 Off, U8 * pData, unsigned NumItems);

extern const FS_NOR_PHY_TYPE FS_NOR_PHY_CFI_1x16;            // 1 x 16-bit CFI compliant NOR flash
extern const FS_NOR_PHY_TYPE FS_NOR_PHY_CFI_2x16;            // 2 x 16-bit CFI compliant NOR flash
extern const FS_NOR_PHY_TYPE FS_NOR_PHY_ST_M25;              // ST M25P compliant Serial NOR flash

void         FS_NOR_GetDiskInfo            (U8 Unit, FS_NOR_DISK_INFO * pDiskInfo);
void         FS_NOR_GetSectorInfo          (U8 Unit, U32 PhysSectorIndex, FS_NOR_SECTOR_INFO * pSectorInfo);
void         FS_NOR_Configure              (U8 Unit, U32 BaseAddr, U32 StartAddr, U32 NumBytes);
void         FS_NOR_ConfigureReserve       (U8 Unit, U8 Percentage2Reserve);
void         FS_NOR_SetPhyType             (U8 Unit, const FS_NOR_PHY_TYPE * pPhyType);
const void * FS_NOR_LogSector2PhySectorAddr(U8 Unit, U32 LogSectorNo);
void         FS_NOR_SetSectorSize          (U8 Unit, U16 SectorSize);
int          FS_NOR_FormatLow              (U8 Unit);
int          FS_NOR_IsLLFormatted          (U8 Unit);
void         FS_NOR_SPI_Configure          (U8 Unit, U32  SectorSize, U16  NumSectors);
void         FS_NOR_CFI_SetReadCFIFunc     (U8 Unit, FS_NOR_READ_CFI_FUNC * pReadCFI);

/*********************************************************************
*
*       RAMDISK driver
*/
void FS_RAMDISK_Configure(U8 Unit, void * pData, U16 BytesPerSector, U32 NumSectors);

/*********************************************************************
*
*       MMC/SD driver
*/
typedef struct {
  U8 aData[16];
} MMC_CARD_ID;

void FS_MMC_ActivateCRC        (void);
void FS_MMC_DeactivateCRC      (void);
int  FS_MMC_GetCardId          (U8 Unit, MMC_CARD_ID * pCardId);
void FS_MMC_CM_Allow4bitMode   (U8 Unit, U8 OnOff);


/*********************************************************************
*
*       IDE/CF driver
*/
void FS_IDE_Configure(U8 Unit, U8 IsSlave);

/*********************************************************************
*
*       NAND driver
*/

typedef struct {
  U16          brsi;              // Block relative sector index.
  U32          ECC;               // Ecc stored for this sector.
  U8           IsValid;           // Sector contains valid data.
} FS_NAND_SECTOR_INFO;

typedef struct {
  U32          EraseCnt;
  U32          lbi;
  U16          NumSectorsBlank;               // Sectors are not used yet.
  U16          NumSectorsValid;               // Sectors contain correct data.
  U16          NumSectorsInvalid;             // Sectors have been invalidated.
  U16          NumSectorsECCError;            // Sectors have incorrect ECC.
  U16          NumSectorsECCCorrectable;      // Sectors have correctable ECC error.
  const char * sType;
} FS_NAND_BLOCK_INFO;

typedef struct {
  U32 NumPhyBlocks;
  U32 NumLogBlocks;
  U32 NumUsedPhyBlocks;
  U32 NumBadPhyBlocks;
  U32 NumPagesPerBlock;
  U32 NumSectorsPerBlock;
  U32 BytesPerPage;
  U32 BytesPerSector;
} FS_NAND_DISK_INFO;

typedef struct FS_NAND_DEVICE_INFO {
  U8  BPP_Shift;              // Bytes per Page shift: 9 for 512 bytes/page or 11 for 2048 bytes/page
  U8  PPB_Shift;              // Pages per block shift: 32 -> 5, 64 -> 6
  U16 NumBlocks;              // Number of Blocks in device
} FS_NAND_DEVICE_INFO;

typedef struct FS_NAND_PHY_TYPE {
  int    (*pfEraseBlock)    (U8 Unit, U32 Block);
  int    (*pfInitGetDeviceInfo) (U8 Unit, FS_NAND_DEVICE_INFO * pDevInfo);
  int    (*pfIsWP)          (U8 Unit);
  int    (*pfRead)          (U8 Unit, U32 PageNo,       void * pData, unsigned Off, unsigned NumBytes);
  int    (*pfReadEx)        (U8 Unit, U32 PageNo,       void * pData, unsigned Off, unsigned NumBytes, void * pSpare, unsigned OffSpare, unsigned NumBytesSpare);
  int    (*pfWrite)         (U8 Unit, U32 PageNo, const void * pData, unsigned Off, unsigned NumBytes);
  int    (*pfWriteEx)       (U8 Unit, U32 PageNo, const void * pData, unsigned Off, unsigned NumBytes, const void* pSpare, unsigned OffSpare, unsigned NumBytesSpare);
} FS_NAND_PHY_TYPE;


extern const FS_NAND_PHY_TYPE FS_NAND_PHY_x;                        // 512 or 2048 byte pages,  8-bit or 16-bit interface
extern const FS_NAND_PHY_TYPE FS_NAND_PHY_x8;                       // 512 or 2048 byte pages,  8-bit interface
extern const FS_NAND_PHY_TYPE FS_NAND_PHY_x16;                      // 512 or 2048 byte pages, 16-bit interface
extern const FS_NAND_PHY_TYPE FS_NAND_PHY_512x8;                    // 512 byte pages,  8-bit interface
extern const FS_NAND_PHY_TYPE FS_NAND_PHY_512x16;                   // 512 byte pages, 16-bit interface
extern const FS_NAND_PHY_TYPE FS_NAND_PHY_2048x8;                   // 2048 byte pages,  8-bit interface
extern const FS_NAND_PHY_TYPE FS_NAND_PHY_2048x16;                  // 2048 byte pages,  16-bit interface
extern const FS_NAND_PHY_TYPE FS_NAND_PHY_4096x8;                   // 4096 byte pages,  8-bit interface
extern const FS_NAND_PHY_TYPE FS_NAND_PHY_DataFlash;                // Physical layer for Atmel serial DATAFLASH

void FS_NAND_ResetStatCounters(U8 Unit);
void FS_NAND_SetPhyType       (U8 Unit, const FS_NAND_PHY_TYPE * pPhyType);
void FS_NAND_SetBlockRange    (U8 Unit, U16 FirstBlock, U16 MaxNumBlocks);
void FS_NAND_GetDiskInfo      (U8 Unit, FS_NAND_DISK_INFO * pDiskInfo);
void FS_NAND_GetBlockInfo     (U8 Unit, U32 PhysBlockIndex, FS_NAND_BLOCK_INFO * pBlockInfo);
void FS_NAND_GetSectorInfo    (U8 Unit, U32 PhysSectorIndex, FS_NAND_SECTOR_INFO * pBlockInfo);
int  FS_NAND_FormatLow        (U8 Unit);
int  FS_NAND_IsLLFormatted    (U8 Unit);
void FS_NAND_EraseFlash       (U8 Unit);
int  FS_NAND_ReadPhySector    (U8 Unit, U32 PhySectorIndex, void * pData, unsigned * pNumBytesData, void * pSpare, unsigned * pNumBytesSpare);
/*********************************************************************
*
*       WinDrive driver
*/
void WINDRIVE_Configure(U8 Unit, const char * sWindowsDriveName);


/*********************************************************************
*
*       Data Flash PHY layer
*/
void FS_DF_ChipErase(U8 Unit);

/*********************************************************************
*
*       Cache handling
*/
#if FS_SUPPORT_CACHE

/*********************************************************************
*
*       General Cache mode defines, do not use in application
*/
#define FS_CACHE_MODE_R         0x01
#define FS_CACHE_MODE_W         0x02
#define FS_CACHE_MODE_D         0x04
// Obsolete Cache mode define, do not use
#define FS_CACHE_MODE_FULL      (FS_CACHE_MODE_R | FS_CACHE_MODE_W | FS_CACHE_MODE_D)

/*********************************************************************
*
*       Cache mode defines that can be used in application
*/
#define FS_CACHE_MODE_WT        (FS_CACHE_MODE_R | FS_CACHE_MODE_W)
#define FS_CACHE_MODE_WB        (FS_CACHE_MODE_R | FS_CACHE_MODE_W | FS_CACHE_MODE_D)

/*********************************************************************
*
*       Cache specific defines
*/
#define FS_CACHE_NONE      NULL
#define FS_CACHE_MAN       FS_CacheMan_Init
#define FS_CACHE_RW        FS_CacheRW_Init
#define FS_CACHE_RW_QUOTA  FS_CacheRWQuota_Init
#define FS_CACHE_ALL       FS_CacheAll_Init
#define FS_CACHEALL_API    FS_CacheAll_Init    /* For compatibility with older version */
#define FS_CACHEMAN_API    FS_CacheMan_Init    /* For compatibility with older version */
/*********************************************************************
*
*       Cache specific types
*/
typedef U32 FS_INIT_CACHE (FS_DEVICE * pDevice, void * pData,   I32 NumBytes);

/*********************************************************************
*
*       Cache specific prototypes
*/
U32    FS_AssignCache     (const char * pName, void * pData, I32 NumBytes, FS_INIT_CACHE * pfInit);
void   FS_CACHE_Clean     (const char * pName);
int    FS_CACHE_Command   (const char * pName, int Cmd,      void * pData);
int    FS_CACHE_SetMode   (const char * pName, int TypeMask, int    ModeMask);
int    FS_CACHE_SetQuota  (const char * pName, int TypeMask, U32    NumSectors);
int    FS_CACHE_Invalidate(const char * sVolumeName);

/*********************************************************************
*
*       Cache initialization prototypes
*/
U32 FS_CacheAll_Init    (FS_DEVICE * pDev, void * pData, I32 NumBytes);
U32 FS_CacheMan_Init    (FS_DEVICE * pDev, void * pData, I32 NumBytes);
U32 FS_CacheRW_Init     (FS_DEVICE * pDev, void * pData, I32 NumBytes);
U32 FS_CacheRWQuota_Init(FS_DEVICE * pDev, void * pData, I32 NumBytes);

#endif

/*********************************************************************
*
*       File system selection prototype
*/
#if FS_SUPPORT_MULTIPLE_FS
int FS_SetFSType(const char * sVolume, int FSType);
#endif

typedef void   (FS_PF_FREE) (void * p);
typedef void * (FS_PF_ALLOC)(U32 NumBytes);
/*********************************************************************
*
*       Memory allocation functions
*/
void * FS_Alloc         (I32     NumBytes);
void * FS_AllocZeroed   (I32     NumBytes);
void   FS_Free          (void *  p);
void   FS_AllocZeroedPtr(void ** pp, I32 NumBytes);
void * FS_TryAlloc      (I32     NumBytesReq);

/*********************************************************************
*
*       Logical driver config
*/
int FS_AddPhysDevice   (const FS_DEVICE_TYPE * pDevType);
int FS_LOGVOL_Create   (const char * sVolName);
int FS_LOGVOL_AddDevice(const char * sLogVolName, const FS_DEVICE_TYPE * pDevice, U8 Unit, U32 StartOff, U32 NumSectors);


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

U32  FS_GetMaxSectorSize(void);
void FS_AssignMemory    (U32 *pMem, U32 NumBytes);
void FS_SetMemHandler   (FS_PF_ALLOC * pfAlloc, FS_PF_FREE * pfFree);

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

#endif                        // Avoid recursive and multiple inclusion

/*************************** End of file ****************************/
