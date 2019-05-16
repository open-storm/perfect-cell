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

#ifndef _FS_INT_H_              // Avoid multiple/recursive inclusion
#define _FS_INT_H_

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "FS_Types.h"
#include "FS.h"
#include "FS_Debug.h"
#include "FS_ConfDefaults.h"        /* FS Configuration */
#include "FS_OS.h"

#if FS_SUPPORT_FAT
  #include "FAT.h"
#endif
#if FS_SUPPORT_EFS
  #include "EFS.h"
#endif

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

#ifdef FS_CORE_C
  #define EXTERN
#else
  #define EXTERN extern
#endif

/*********************************************************************
*
*       Global defines
*
**********************************************************************
*/
#define COUNTOF(a)          (sizeof(a)/sizeof(a[0]))
#define MIN(a,b)            (((a) < (b)) ? (a) : (b))
#define MAX(a,b)            (((a) > (b)) ? (a) : (b))
#define ZEROFILL(p, Size)   (memset(p, 0, Size))


#define FS_DISKINFO_FLAG_USE_FREE_SPACE          (1 << 0)


/*********************************************************************
*
*       Data types
*
**********************************************************************
*/

typedef struct {
  int TypeMask;    /* Combination of FS_SECTOR_TYPE_DATA, FS_SECTOR_TYPE_MAN, FS_SECTOR_TYPE_DIR */
  int ModeMask;    /* Combination of FS_CACHE_MODE_R, FS_CACHE_MODE_W, FS_CACHE_MODE_D */
} CACHE_MODE;

typedef struct {
  int TypeMask;    /* Combination of FS_SECTOR_TYPE_DATA, FS_SECTOR_TYPE_MAN, FS_SECTOR_TYPE_DIR */
  int NumSectors;  /* Number of sectors to use for the type(s) specified */
} CACHE_QUOTA;

typedef struct {
  U32 FirstSector;
  U32 NumSectors;
} CACHE_FREE;


/*********************************************************************
*
*       FS_INFO
*
*  The following structures are related:
*  The first 3 entries of the structures in the union need to be identical.
*/
struct FS_FAT_INFO {
  U32          NumSectors;       // RSVD + FAT + ROOT + FATA
  U16          BytesPerSec;      // 512,1024,2048,4096
  U16          ldBytesPerSector; // 9, 10, 11, 12
  U32          FirstDataSector;
  U32          BytesPerCluster;
  U32          FATSize;          // Number of FAT sectors
  U32          RootDirPos;       /* Position of root directory. FAT32: Cluster, FAT12/16: Sector */
  U16          RootEntCnt;       /* number of root dir entries     */
  U16          RsvdSecCnt;       /* 1 for FAT12 & FAT16            */
  U8           SecPerClus;       /* sec in allocation unit         */
  U8           NumFATs;          /* Typical value is 2             */
  U8           FATType;          /* FS_FAT_TYPE_FAT12/16/32        */
  U32          NumClusters;
  U32          NumFreeClusters;  /* Once known, we keep track of the number of free clusters */
  U32          NextFreeCluster;  /* Once known, we keep track of the next free cluster */
#if FS_FAT_USE_FSINFO_SECTOR
  U16          FSInfoSector;
  U16          FSInfoSectorIsInvalidated;
#endif
  //
  //  Used with File write operation mode is set to fast
  //
  U32          FreeClusterStart;
  U32          FreeClusterCnt;
  U32          FreeClusterUse;
  FS_FILE *    pFileUsedWithFreeCluster;
};

struct FS_EFS_INFO {
  U32          NumSectors;         // RSVD + EFS alloc table + DATA
  U16          BytesPerSector;     // 512,1024,2048,4096
  U16          ldBytesPerSector;   // 9, 10, 11, 12
  U32          FirstDataSector;    // First data cluster -> First Sector after InfoSector, EFSAllocTable.
  U32          BytesPerCluster;    // Bytes for each cluster: 512 - 32768
  U32          EFSTableSize;       // EFS Alloc table size in no. of sectors
  U32          RootDirPos;         // Cluster that is used for the root directory
  U8           SectorsPerCluster;  // Number of sectors for a cluster
  U32          NumClusters;        // Number of available clusters
  U32          NumFreeClusters;    // Once known, we keep track of the number of free clusters
  U32          NextFreeCluster;    // Once known, we keep track of the next free cluster
};


typedef union {
  FS_FAT_INFO  FATInfo;
  FS_EFS_INFO  EFSInfo;
  struct {
    U32          NumSectors;         // RSVD + EFS alloc table + DATA
    U16          BytesPerSector;     // 512,1024,2048,4096
    U16          ldBytesPerSector;   // 9, 10, 11, 12
  } Info;
} FS_INFO;

typedef struct {
  U8 Unit;
  U8 IsInited;
  U8 JournalIsActive;
  U8 JournalUnit;
#if FS_SUPPORT_CACHE
  const FS_CACHE_API * pCacheAPI;
  void               * pCacheData;
#endif
#if FS_SUPPORT_BUSY_LED
  FS_BUSY_LED_CALLBACK * pfSetBusyLED;
#endif
#if FS_SUPPORT_CHECK_MEMORY
  FS_MEMORY_IS_ACCESSIBLE_CALLBACK * pfMemoryIsAccessible;
#endif
  FS_ONDEVICEACTIVITYHOOK * pfOnDeviceActivityHook;
} FS_DEVICE_DATA;

struct FS_DEVICE {
  const FS_DEVICE_TYPE * pType;
  FS_DEVICE_DATA         Data;
};

struct FS_PARTITION {
  FS_DEVICE Device;
  U32       StartSector;
  U32       NumSectors;
};

typedef struct {
  int   InUse;
  U32 * pBuffer;
} SECTOR_BUFFER;


typedef void        FS_CB_CLOSE               (FS_FILE    * pFile);
typedef int         FS_CB_CHECKFS_API         (FS_VOLUME  * pVolume);
typedef U32         FS_CB_READ                (FS_FILE    * pFile,           void  * pData, U32 NumBytes);
typedef U32         FS_CB_WRITE               (FS_FILE    * pFile,     const void  * pData, U32 NumBytes);
typedef char        FS_CB_OPEN                (const char * pFileName, FS_FILE * pFile, char DoDel, char DoOpen, char DoCreate);
typedef int         FS_CB_FORMAT              (FS_VOLUME  * pVolume,   FS_FORMAT_INFO * pFormatInfo);
typedef int         FS_CB_OPENDIR             (const char * pDirName,  FS__DIR *pDir);
typedef int         FS_CB_CLOSEDIR            (FS__DIR    * pDir);
typedef int         FS_CB_READDIR             (FS__DIR    * pDir,      FS_DIRENTRY_INFO * pDirEntryInfo);
typedef int         FS_CB_REMOVEDIR           (FS_VOLUME  * pVolume,   const char * pDirName);
typedef int         FS_CB_CREATEDIR           (FS_VOLUME  * pVolume,   const char * pDirName);
typedef int         FS_CB_RENAME              (const char * sOldName,  const char * sNewName, FS_VOLUME * pVolume);
typedef int         FS_CB_MOVE                (const char * sOldName,  const char * sNewName, FS_VOLUME * pVolume);
typedef char        FS_CB_SETDIRENTRYINFO     (FS_VOLUME  * pVolume,   const char * sName, const void * p, int Mask);
typedef char        FS_CB_GETDIRENTRYINFO     (FS_VOLUME  * pVolume,   const char * sName,       void * p, int Mask);
typedef int         FS_CB_SETENDOFFILE        (FS_FILE    * pFile);
typedef void        FS_CB_UNMOUNT             (FS_VOLUME  * pVolume);

typedef int         FS_CB_GETDISKINFO         (FS_VOLUME  * pVolume,   FS_DISK_INFO * pDiskData, int Flags);
typedef int         FS_CB_GETVOLUMELABEL      (FS_VOLUME  * pVolume,   char * pVolumeLabel, unsigned VolumeLabelSize);
typedef int         FS_CB_SETVOLUMELABEL      (FS_VOLUME  * pVolume,   const char * pVolumeLabel);
typedef int         FS_CB_CREATEJOURNALFILE   (FS_VOLUME  * pVolume,   U32 NumBytes, U32 * pFirstSector, U32 * pNumSectors);
typedef int         FS_CB_OPENJOURNALFILE     (FS_VOLUME  * pVolume);
typedef U32         FS_CB_GETINDEXOFLASTSECTOR(FS_VOLUME  * pVolume);
typedef int         FS_CB_CHECKDISK           (FS_VOLUME  * pVolume,   FS_DISK_INFO * pDiskInfo, void * pBuffer, U32 BufferSize, int MaxRecursionLevel, FS_QUERY_F_TYPE * pfOnError);
typedef void        FS_CB_UPDATE_FILE         (FS_FILE    * pFile);

typedef struct {
  FS_CB_CHECKFS_API          * pfCheckBootSector;
  FS_CB_OPEN                 * pfOpen;
  FS_CB_CLOSE                * pfClose;
  FS_CB_READ                 * pfRead;
  FS_CB_WRITE                * pfWrite;
  FS_CB_FORMAT               * pfFormat;
  FS_CB_OPENDIR              * pfOpenDir;
  FS_CB_CLOSEDIR             * pfCloseDir;
  FS_CB_READDIR              * pfReadDir;
  FS_CB_REMOVEDIR            * pfRemoveDir;
  FS_CB_CREATEDIR            * pfCreateDir;
  FS_CB_RENAME               * pfRename;
  FS_CB_MOVE                 * pfMove;
  FS_CB_SETDIRENTRYINFO      * pfSetDirEntryInfo;
  FS_CB_GETDIRENTRYINFO      * pfGetDirEntryInfo;
  FS_CB_SETENDOFFILE         * pfSetEndOfFile;
  FS_CB_UNMOUNT              * pfUnmount;
  FS_CB_GETDISKINFO          * pfGetDiskInfo;
  FS_CB_GETVOLUMELABEL       * pfGetVolumeLabel;
  FS_CB_SETVOLUMELABEL       * pfSetVolumeLabel;
  FS_CB_CREATEJOURNALFILE    * pfCreateJournalFile;
  FS_CB_OPENJOURNALFILE      * pfOpenJournalFile;
  FS_CB_GETINDEXOFLASTSECTOR * pfGetIndexOfLastSector;
  FS_CB_CHECKDISK            * pfCheckDisk;
  FS_CB_UPDATE_FILE          * pfUpdateFile;
} FS_FS_API;

struct FS_VOLUME {
  FS_PARTITION Partition;
  FS_INFO      FSInfo;
  U8           PartitionIndex;
  U8           IsMounted;
  U8           AllowAutoMount;
  U8           InUse;
#if FS_SUPPORT_MULTIPLE_FS
  const FS_FS_API * pFS_API;
#endif
  FS_VOLUME *  pNext;
};

/*********************************************************************
*
*       Directory types
*/
struct FS_DIR {
  FS__DIR     Dir;
  FS_DIRENT   DirEntry;
  I16         error;              /* error code                   */
  U8          InUse;              /* handle in use mark           */
};


/*********************************************************************
*
*       File types
*/
typedef struct {
    U32      CurClusterFile; /* Current cluster within the file. First cluster is 0, next cluster is 1 ... */
    U32      CurClusterAbs;  /* Current cluster on the medium. This needs to be computed from file cluster, consulting the FAT */
    U32      DirEntrySector; /* Sector of directory */
    U16      DirEntryIndex;  /* Index of directory entry */
    U16      NumAdjClusters; /* Number of cluster that are sequentially in chain behind current one */
} FS_INT_DATA_FAT;

typedef struct {
      U32      DirCluster;     /* Start cluster of directory file */
      U32      DirEntryPos;    /* Offset of directory entry in directory file */
      U32      CurClusterFile; /* Current cluster within the file. First cluster is 0, next cluster is 1 ... */
      U32      CurClusterAbs;  /* Current cluster on the medium. This needs to be computed from file cluster, consulting the FAT */
      U16      NumAdjClusters; /* Number of cluster that are sequentially in chain behind current one */
} FS_INT_DATA_EFS;

typedef union {
  FS_INT_DATA_FAT Fat;
  FS_INT_DATA_EFS Efs;
} FS_INT_DATA;


typedef struct FS_FILE_OBJ FS_FILE_OBJ;
typedef struct {
  U8       * pData;
  U32        FilePos;
  U32        SectorNo;
  unsigned   BufferSize;
  unsigned   NumBytesInBuffer;
  U8         IsDirty;
  U8         Flags;
} FS_FILE_BUFFER;


struct FS_FILE_OBJ {      /* Describes the file object structure below the handle */
  U32           FirstCluster;   /* First cluster used for file  */
  U32           Size;           /* size of file                 */
  FS_VOLUME   * pVolume;
  U8            UseCnt;         /* handle in use mark           */
  FS_INT_DATA   Data;
#if FS_MULTI_HANDLE_SAFE
  char          acFullFileName[FS_MAX_LEN_FULL_FILE_NAME];
#endif
#if FS_USE_FILE_BUFFER
  FS_FILE_BUFFER * pBuffer;
#endif
  FS_FILE_OBJ * pNext;
};

struct FS_FILE { /* Describes the file handle structure */
  FS_FILE_OBJ * pFileObj;
  U32           FilePos;        /* current position in file     */
  I16           Error;          /* error code                   */
  U8            InUse;          /* handle in use mark           */
  U8            AccessFlags;
  FS_FILE     * pNext;
};

/*********************************************************************
*
*       Smart buffer (SB) type, internal
*
*/
struct FS_SB {
  U32 SectorNo;
#if FS_MAINTAIN_FAT_COPY
  U32 WriteCopyOff;  /* Duplicate on write if value is != 0 */
#endif
  FS_PARTITION * pPart;
  U8 *           pBuffer;
  char           IsDirty;
  char           HasError;
  U8             Type;
  U8             Read;          /* 1 if data is valid, usually because the sector has been read */
};

/*********************************************************************
*
*       Partition related
*/
U32           FS__GetMediaStartSecEx(FS_VOLUME * pVolume, U32 * pNumSectors, U8* pBuffer);
U32           FS__GetMediaStartSec(U8 PartIndex, U8 *pBuffer);
signed char   FS__LocatePartition(FS_VOLUME * pVolume);

/*********************************************************************
*
*       little endian translation functions, internal
*/
U16           FS_LoadU16LE (const U8 *pBuffer);
U32           FS_LoadU32LE (const U8 *pBuffer);
void          FS_StoreU16LE(      U8 *pBuffer, unsigned Data);
void          FS_StoreU24LE(      U8 *pBuffer, U32 Data);
void          FS_StoreU32LE(      U8 *pBuffer, U32 Data);

/*********************************************************************
*
*       big endian translation functions, internal
*/
U32           FS_LoadU32BE (const U8 *pBuffer);
U16           FS_LoadU16BE (const U8 *pBuffer);
void          FS_StoreU32BE(      U8 *pBuffer, U32 Data);
void          FS_StoreU16BE(      U8 *pBuffer, unsigned Data);


/*********************************************************************
*
*       CACHE API type, internal
*
*/
struct FS_CACHE_API {
  char   (*pfReadFromCache)   (FS_DEVICE * pDevice,      U32 SectorNo,       void * pData, U8  Type);
  char   (*pfUpdateCache)     (FS_DEVICE * pDevice,      U32 SectorNo, const void * pData, U8  Type);    /* Returns 0 if write cached, which means no further write is required */
  void   (*pfInvalidateCache) (void      * pCacheData);
  int    (*pfCommand)         (FS_DEVICE * pDevice   ,   int Cmd, void *p);
  char   (*pfWriteIntoCache)  (FS_DEVICE * pDevice   ,   U32 SectorNo, const void * pData, U8  Type);    /* Returns 0 if write cached, which means no further write is required */

};

/*********************************************************************
*
*       Smart buffer (SB) API-functions, internal
*
*/
void FS__SB_Clean          (FS_SB * pSB);
char FS__SB_Create         (FS_SB * pSB, FS_PARTITION *pPart);
void FS__SB_Delete         (FS_SB * pSB);
void FS__SB_Flush          (FS_SB * pSB);
void FS__SB_MarkDirty      (FS_SB * pSB);
void FS__SB_MarkValid      (FS_SB * pSB, U32 SectorNo, U8 Type);
void FS__SB_MarkNotDirty   (FS_SB * pSB);
char FS__SB_Read           (FS_SB * pSB);
void FS__SB_SetSector      (FS_SB * pSB, U32 SectorNo, U8 Type);
char FS__SB_Write          (FS_SB * pSB);

#if FS_MAINTAIN_FAT_COPY
  void FS__SB_SetWriteCopyOff(FS_SB * pSB, U32 Off);
#endif

/*********************************************************************
*
*       SB-functions, locking and unlocking SB operations
*
*/
void    FS_Lock_SB  (FS_PARTITION * pPart, U32 SectorIndex);
void    FS_Unlock_SB(FS_PARTITION * pPart, U32 SectorIndex);


/*********************************************************************
*
*       Cache related fucntions, internal
*
*/
int   FS__CACHE_CommandVolume  (FS_VOLUME * pVolume, int Cmd, void * pData);
int   FS__CACHE_CommandVolumeNL(FS_VOLUME * pVolume, int Cmd, void * pData);
int   FS__CACHE_CommandDevice  (FS_DEVICE * pDevice, int Cmd, void * pData);
void  FS__CACHE_CleanNL        (FS_DEVICE * pDevice);
void  FS__CACHE_Clean          (FS_VOLUME * pVolume);


/*********************************************************************
*
*       Sector allocation API-functions, internal
*
*/
U8 *  FS__AllocSectorBuffer (void);
void  FS__FreeSectorBuffer  (void * p);

/*********************************************************************
*
*       String operation API-functions, internal
*
*/
void           FS_memcpy(void * pDest, const void * pSrc, int NumBytes);
const char *   FS__strchr     (const char *s, int c);
void           FS__AddSpaceHex(U32 v, U8 Len, char** ps);

/*********************************************************************
*
*       Volume API-functions, internal
*
*/
FS_VOLUME * FS__FindVolume     (const char *pFullName, const char ** pFileName);
int         FS__Mount          (FS_VOLUME * pVolume, U8 MountType);
int         FS__AutoMount      (FS_VOLUME * pVolume);
void        FS__Unmount        (FS_VOLUME * pVolume);
void        FS__UnmountLL      (FS_VOLUME * pVolume);
void        FS__UnmountForcedLL(FS_VOLUME * pVolume);
void        FS__Sync           (FS_VOLUME * pVolume);
void        FS__STORAGE_Sync   (FS_VOLUME * pVolume);

/*********************************************************************
*
*       API-functions, internal (without Global locking)
*
*/
int         FS__FClose           (FS_FILE * pFile);
U32         FS__GetFileSize      (FS_FILE * pFile);
U32         FS__Read             (FS_FILE * pFile,       void * pData, U32 NumBytes);
U32         FS__Write            (FS_FILE * pFile, const void * pData, U32 NumBytes);
int         FS__Verify           (FS_FILE * pFile, const void * pData, U32 NumBytes);
int         FS__Remove           (const char * pFileName);
FS_FILE *   FS__FOpen            (const char * pFileName, const char * pMode);
FS_FILE *   FS__FOpenEx          (const char * pFileName, U8 AccessFlags, char DoCreate, char DoDel, char DoOpen);
FS_FILE *   FS__OpenEx           (FS_VOLUME  * pVolume,   const char * sFilePath, U8 AccessFlags, char DoCreate, char DoDel, char DoOpen);

U32         FS__CalcSizeInBytes  (U32 NumClusters,   U32 SectorsPerCluster, U32 BytesPerSector);
U32         FS__CalcSizeInKB     (U32 NumClusters, U32 SectorsPerCluster, U32 BytesPerSector);

FS_VOLUME * FS__AddDevice        (const FS_DEVICE_TYPE * pDevType);
int         FS__AddPhysDevice    (const FS_DEVICE_TYPE * pDevType);

int         FS__IoCtl            (FS_VOLUME * pVolume, I32 Cmd, I32 Aux, void *pBuffer);
int         FS__Format           (FS_VOLUME * pVolume, FS_FORMAT_INFO * pFormatInfo);
int         FS__SD_Format        (FS_VOLUME  * pVolume);
int         FS__GetNumVolumes    (void);
int         FS__CopyFile         (const char * sSource, const char   * sDest);
int         FS__GetVolumeInfo    (const char * sVolume, FS_DISK_INFO * pInfo);
int         FS__GetVolumeInfoEx  (const char * sVolume, FS_DISK_INFO * pInfo, int Flags);
int         FS__CreateDir        (const char * sDir);
int         FS__MkDir            (const char * pDirName);
int         FS__RmDir            (const char * pDirName);
int         FS__FSeek            (FS_FILE *pFile, I32 Offset, int Origin);
I32         FS__FTell            (FS_FILE *pFile);
int         FS__GetFileTimeEx    (const char * pName, U32 * pTimeStamp, int Index);
int         FS__SetFileTimeEx    (const char * pName, U32   TimeStamp,  int Index);
int         FS__SetFileAttributes(const char * pName, U8    Attributes);
U8          FS__GetFileAttributes(const char * pName);
int         FS__SetEndOfFile     (FS_FILE    * pFile);
void        FS__RemoveDevices    (void);
void        FS__RemoveDevice     (FS_VOLUME * pVolume);

FS_DIR    * FS__OpenDir      (const char *pDirName);
FS_DIRENT * FS__ReadDir      (FS_DIR *pDir);
int         FS__CloseDir     (FS_DIR *pDir);
void        FS__RewindDir    (FS_DIR *pDir);
void        FS__DirEnt2Attr  (FS_DIRENT *pDirEnt, U8* pAttr);
U32         FS__GetNumFiles  (FS_DIR *pDir);
int         FS__FindFirstFile(FS_FIND_DATA * pfd, const char * sPath, char * sFilename, int sizeofFilename);
int         FS__FindFirstFileEx(FS_FIND_DATA * pfd, FS_VOLUME * pVolume, const char * sPath, char * sFilename, int sizeofFilename);
int         FS__FindNextFile (FS_FIND_DATA * pfd);
void        FS__FindClose    (FS_FIND_DATA * pfd);

int         FS__FormatLow      (FS_VOLUME * pVolume);
void        FS__UnmountForced  (FS_VOLUME * pVolume);
int         FS__GetVolumeStatus(FS_VOLUME * pVolume);

int         FS__ReadSector   (FS_VOLUME * pVolume,       void * pData, U32 SectorIndex);
int         FS__ReadSectors  (FS_VOLUME * pVolume,       void * pData, U32 SectorIndex, U32 NumSectors);
int         FS__WriteSector  (FS_VOLUME * pVolume, const void * pData, U32 SectorIndex);
int         FS__WriteSectors (FS_VOLUME * pVolume, const void * pData, U32 SectorIndex, U32 NumSectors);
int         FS__GetDeviceInfo(FS_VOLUME * pVolume, FS_DEV_INFO * pDevInfo);
int         FS__IsLLFormatted(FS_VOLUME * pVolume);

/*********************************************************************
*
*       API-functions, internal (without driver locking)
*
*/
int         FS__FCloseNL             (FS_FILE   * pFile);
int         FS__CACHE_CommandDeviceNL(FS_DEVICE * pDevice, int Cmd, void * pData);
int         FS__IoCtlNL              (FS_VOLUME * pVolume, I32 Cmd, I32 Aux, void * Buffer);
void        FS__UnmountForcedNL      (FS_VOLUME * pVolume);
void        FS__UnmountNL            (FS_VOLUME * pVolume);
int         FS__MountNL              (FS_VOLUME * pVolume, U8 MountType);
int         FS__AutoMountNL          (FS_VOLUME * pVolume);

/*********************************************************************
*
*       API-functions, file handle operations
*
*/
FS_FILE * FS__AllocFileHandle(void);
void      FS__FreeFileHandle(FS_FILE * pFile);

/*********************************************************************
*
*       API-functions, file object operations
*
*/
FS_FILE_OBJ * FS__AllocFileObj(const char  * sFullFileName);
FS_FILE_OBJ * FS__GetFileObj  (const char  * sFullFileName);
void          FS__FreeFileObj (FS_FILE_OBJ * pFileObj);

/*********************************************************************
*
*       ECC256
*
*/
int  FS__ECC256_Apply(U32 * pData, U32 eccRead);
U32  FS__ECC256_Calc (const U32 * pData);
int  FS__ECC256_IsValid(U32 ecc);
U32  FS__ECC256_Load (const U8 * p);
void FS__ECC256_Store(U8 * p, U32 ecc);

/*********************************************************************
*
*       Helper functions
*
*/
U32      FS__DivideU32Up            (      U32  Nom,      U32      Div);
U32      FS_CRC32_Calc              (const U8 * pData,    unsigned NumBytes, U32 crc);
U16      FS_CRC16_Calc              (const U8 * pData,    unsigned NumBytes, U16 crc);
unsigned FS_BITFIELD_CalcSize       (      U32  NumItems, unsigned BitsPerItem);
unsigned FS_BITFIELD_CalcNumBitsUsed(      U32  NumItems);
U32      FS_BITFIELD_ReadEntry      (const U8 * pBase,    U32      Index,       unsigned NumBits);
void     FS_BITFIELD_WriteEntry     (      U8 * pBase,    U32      Index,       unsigned NumBits, U32 v);


/*********************************************************************
*
*       Public data
*
*/

/*********************************************************************
*
*       FS_GLOBAL
*/
typedef struct {
  unsigned        FileBufferSize;
  char            FileBufferFlags;
  SECTOR_BUFFER * paSectorBuffer;
  unsigned        NumSectorBuffers;
  U16             MaxSectorSize;
  U8              IsInited;
  FS_FILE       * pFirstFilehandle;
  FS_FILE_OBJ   * pFirstFileObj;
  FS_VOLUME       FirstVolume;
  int             NumVolumes;
  FS_WRITEMODE    WriteMode;
} FS_GLOBAL;

EXTERN FS_GLOBAL FS_Global;

/*********************************************************************
*
*       OS mapping macros (multi tasking locks)
*
*
* Notes
*   These macros map to locking routines or are empty,
*   depending on the configuration
*   There are 3 different lock-levels:
*   FS_OS == 0                     -> No locking
*   FS_OS == 1
*     FS_OS_LOCK_PER_DRIVER == 0     -> Single, global lock in every API function
*     FS_OS_LOCK_PER_DRIVER == 1     -> Multiple locks
*
**********************************************************************
*/

#if (FS_OS == 0)                               /* No locking */
  #define FS_LOCK()
  #define FS_UNLOCK()

  #define FS_LOCK_SYS()
  #define FS_UNLOCK_SYS()

  #define FS_OS_INIT(MaxNumLocks)
  #define FS_OS_DEINIT()


  #define FS_LOCK_DRIVER(pDriver)
  #define FS_UNLOCK_DRIVER(pDriver)

  #define FS_OS_ADD_DRIVER(pDevice)
  #define FS_OS_REMOVE_DRIVER(pDevice)

  #define  FS_OS_GETNUM_DRIVERLOCKS()               0
  #define  FS_OS_GETNUM_SYSLOCKS()                  0

  #define FS_OS_WAIT(Timeout)                       
  #define FS_OS_SIGNAL()                            


#elif (FS_OS) && (FS_OS_LOCK_PER_DRIVER == 0)    
  //
  // Coarse lock granularity: 
  //   One global lock for all FS API functions
  //

  #define FS_LOCK_ID_SYSTEM                  0

  #define FS_LOCK()           FS_OS_Lock(FS_LOCK_ID_SYSTEM)
  #define FS_UNLOCK()         FS_OS_Unlock(FS_LOCK_ID_SYSTEM)

  #define FS_LOCK_SYS()
  #define FS_UNLOCK_SYS()

  #define FS_LOCK_DRIVER(pDriver)
  #define FS_UNLOCK_DRIVER(pDriver)

  #define FS_OS_ADD_DRIVER(pDevice)
  #define FS_OS_REMOVE_DRIVER(pDevice)

  #define  FS_OS_GETNUM_DRIVERLOCKS()               0
  #define  FS_OS_GETNUM_SYSLOCKS()                  1

  #define FS_OS_INIT(MaxNumLocks)                       FS_OS_Init(MaxNumLocks)
  #define FS_OS_DEINIT()                                FS_OS_DeInit()

  #define FS_OS_WAIT(Timeout)                           FS_X_OS_Wait(Timeout)
  #define FS_OS_SIGNAL()                                FS_X_OS_Signal()
#else
  //
  // Fine lock granularity: 
  //   Lock for different FS functions
  //
  #define FS_LOCK_ID_SYSTEM                  0
  #define FS_LOCK_ID_DEVICE                  1

  #define FS_LOCK()
  #define FS_UNLOCK()


  #define FS_LOCK_SYS()                      FS_OS_Lock  (FS_LOCK_ID_SYSTEM)
  #define FS_UNLOCK_SYS()                    FS_OS_Unlock(FS_LOCK_ID_SYSTEM)

  #define FS_LOCK_DRIVER(pDevice)            FS_OS_LockDriver(pDevice)
  #define FS_UNLOCK_DRIVER(pDevice)          FS_OS_UnlockDriver(pDevice)
  void FS_OS_LockDriver  (const FS_DEVICE * pDevice);
  void FS_OS_UnlockDriver(const FS_DEVICE * pDevice);


  #define  FS_OS_GETNUM_DRIVERLOCKS()               FS_OS_GetNumDriverLocks()
  #define  FS_OS_GETNUM_SYSLOCKS()                  1
  unsigned FS_OS_GetNumDriverLocks(void);

  #define FS_OS_ADD_DRIVER(pDevice)         FS_OS_AddDriver(pDevice)
  void FS_OS_AddDriver(const FS_DEVICE_TYPE * pDriver);

  #define FS_OS_REMOVE_DRIVER(pDevice)              FS_OS_RemoveDriver(pDevice)
  void FS_OS_RemoveDriver(const FS_DEVICE_TYPE * pDriver);

  #define FS_OS_INIT(MaxNumLocks)                       FS_OS_Init(MaxNumLocks)
  #define FS_OS_DEINIT()                                FS_OS_DeInit()

  #define FS_OS_WAIT(Timeout)                           FS_X_OS_Wait(Timeout)
  #define FS_OS_SIGNAL()                                FS_X_OS_Signal()
#endif

void FS_OS_Lock  (unsigned LockIndex);
void FS_OS_Unlock(unsigned LockIndex);
void FS_OS_Init  (unsigned NumLocks);
void FS_OS_DeInit(void);

/*********************************************************************
*
*       FS_JOURNAL
*
*/
int  FS__JOURNAL_Begin     (FS_VOLUME * pVolume);
int  FS__JOURNAL_End       (FS_VOLUME * pVolume);
void FS__JOURNAL_Delete    (FS_VOLUME * pVolume, U32 LastSectorInFS);
void FS__JOURNAL_Invalidate(FS_VOLUME * pVolume);
int  FS__JOURNAL_Clean     (FS_VOLUME * pVolume);

#if FS_SUPPORT_JOURNAL
  #define  FS_JOURNAL_BEGIN(pVolume)                  FS__JOURNAL_Begin(pVolume)
  #define  FS_JOURNAL_END(pVolume)                    FS__JOURNAL_End  (pVolume)
  #define  FS_JOURNAL_MOUNT(pVolume)                  FS__JOURNAL_Mount(pVolume)
  #define  FS_JOURNAL_DELETE(pVolume, LastSector)     FS__JOURNAL_Delete(pVolume, LastSector)
  #define  FS_JOURNAL_INVALIDATE(pVolume)             FS__JOURNAL_Invalidate(pVolume)
  #define  FS_JOURNAL_CLEAN(pVolume)                  FS__JOURNAL_Clean(pVolume)
#else
  #define  FS_JOURNAL_BEGIN(pVolume)                  FS_USE_PARA(pVolume)
  #define  FS_JOURNAL_END(pVolume)                    FS_USE_PARA(pVolume)
  #define  FS_JOURNAL_MOUNT(pVolume)
  #define  FS_JOURNAL_DELETE(pVolume, LastSector)
  #define  FS_JOURNAL_INVALIDATE(pVolume)
  #define  FS_JOURNAL_CLEAN(pVolume)
#endif

#if FS_SUPPORT_DEINIT && FS_SUPPORT_JOURNAL
  #define  FS_JOURNAL_DEINIT(pVolume)                 FS__JOURNAL_DeInit(pVolume)
  void FS_JOURNAL_DeInit(FS_VOLUME * pVolume);
#else
  #define  FS_JOURNAL_DEINIT(pVolume)
#endif


/*********************************************************************
*
*       MEMORY API
*
*       This macro map to the memory free functions of the file system
*       when deinitialization is enabled.
*
**********************************************************************
*/
#if FS_SUPPORT_DEINIT
  #define  FS_FREE(pMem)                 FS_Free(pMem)
#else
  #define  FS_FREE(pMem)
#endif

/*********************************************************************
*
*       API mapping macros
*
*       These macros map to the functions of the file system (Currently FAT or EFS)
*       or - in case of multiple file systems - to a mapping layer, which calls the
*       appropriate function depending on the filesystem of the volume
*
**********************************************************************
*/

#define FS_IOCTL(    pDevice, Cmd, Aux, pBuffer)                FS_LB_Ioctl(pDevice, Cmd, Aux, pBuffer)

#if   (FS_SUPPORT_FAT) && (! FS_SUPPORT_EFS)
  #define FS_CHECK_INFOSECTOR(pVolume)                                FS_FAT_CheckBPB(pVolume)
  #define FS_CLOSEDIR(pDir)                                           FS_FAT_CloseDir(pDir);
  #define FS_CREATEDIR(pVolume, s)                                    FS_FAT_CreateDir(pVolume, s)
  #define FS_CLOSE_FILE(   hFile)                                     FS_FAT_Close(hFile)
  #define FS_OPEN_FILE(    s,       pFile, DoDel, DoOpen, DoCreate)   FS_FAT_Open(s, pFile, DoDel, DoOpen, DoCreate)
  #define FS_FORMAT( pVolume,  pFormatInfo)                           FS_FAT_Format(pVolume, pFormatInfo)
  #define FS_FREAD(    pFile,   pData, NumBytes)                      FS_FAT_Read(pFile, pData, NumBytes)
  #define FS_FWRITE(   pFile,   pData, NumBytes)                      FS_FAT_Write(pFile, pData, NumBytes)
  #define FS_GETDIRENTRYINFO(pVolume, sName, p, Mask)                 FS_FAT_GetDirEntryInfo(pVolume, sName, p, Mask)
  #define FS_GET_DISKINFO(  pVolume, pInfo, Flags)                    FS_FAT_GetDiskInfo(pVolume, pInfo, Flags)
  #define FS_MOVE(     sSrc,    sDest, pVolume)                       FS_FAT_Move(sSrc, sDest, pVolume)
  #define FS_OPENDIR(  s,       pDirHandle)                           FS_FAT_OpenDir(s, pDirHandle)
  #define FS_READDIR(  pDir, pDirEntryInfo)                           FS_FAT_ReadDir(pDir, pDirEntryInfo)
  #define FS_REMOVEDIR(pVolume, s)                                    FS_FAT_RemoveDir(pVolume, s)
  #define FS_RENAME(   s,       sNewName, pVolume)                    FS_FAT_Rename(s, sNewName, pVolume)
  #define FS_SETDIRENTRYINFO(pVolume, sName, p, Mask)                 FS_FAT_SetDirEntryInfo(pVolume, sName, p, Mask)
  #define FS_SET_END_OF_FILE(pFile)                                   FS_FAT_SetEndOfFile(pFile)
  #define FS_CLEAN_FS(         pVolume)                               FS_FAT_Clean(pVolume)
  #define FS_GET_VOLUME_LABEL(pVolume, pVolLabel, VolLabelSize)       FS_FAT_GetVolumeLabel(pVolume, pVolLabel, VolLabelSize)
  #define FS_SET_VOLUME_LABEL(pVolume, pVolLabel)                     FS_FAT_SetVolumeLabel(pVolume, pVolLabel)
  #define FS_CREATE_JOURNAL_FILE(pVolume, NumBytes, pFirstS, pNumS)   FS_FAT_CreateJournalFile(pVolume, NumBytes, pFirstS, pNumS)
  #define FS_OPEN_JOURNAL_FILE(pVolume)                               FS_FAT_OpenJournalFile(pVolume)
  #define FS_GET_INDEX_OF_LAST_SECTOR(pVolume)                        FS_FAT_GetIndexOfLastSector(pVolume)
  #define FS_CHECKDISK(pVolume, pDiskInfo, pBuffer, BufferSize, MaxRecursionLevel, pfOnError) FS_FAT__CheckDisk(pVolume, pDiskInfo, pBuffer, BufferSize, MaxRecursionLevel, pfOnError)
  #define FS_UPDATE_FILE(   hFile)                                    FS_FAT_Close(hFile)
#elif (! FS_SUPPORT_FAT) && (FS_SUPPORT_EFS)
  #define FS_CHECK_INFOSECTOR(pVolume)                                FS_EFS_CheckInfoSector(pVolume)
  #define FS_CLOSEDIR( pDir)                                          FS_EFS_CloseDir(pDir)
  #define FS_CREATEDIR(pVolume, s)                                    FS_EFS_CreateDir(pVolume, s)
  #define FS_CLOSE_FILE(   hFile)                                     FS_EFS_Close(hFile)
  #define FS_OPEN_FILE(    s,       pFile, DoDel, DoOpen, DoCreate)   FS_EFS_Open(s, pFile, DoDel, DoOpen, DoCreate)
  #define FS_FORMAT( pVolume,  pFormatInfo)                           FS_EFS_Format(pVolume, pFormatInfo)
  #define FS_FREAD(    pFile,   pData, NumBytes)                      FS_EFS_Read(pFile, pData, NumBytes)
  #define FS_FWRITE(   pFile,   pData, NumBytes)                      FS_EFS_Write(pFile, pData, NumBytes)
  #define FS_GETDIRENTRYINFO(pVolume, sName, p, Mask)                 FS_EFS_GetDirEntryInfo(pVolume, sName, p, Mask)
  #define FS_GET_DISKINFO(  pVolume, pInfo, Flags)                    FS_EFS_GetDiskInfo(pVolume, pInfo, Flags)
  #define FS_MOVE(     sSrc,    sDest, pVolume)                       FS_EFS_Move(sSrc, sDest, pVolume)
  #define FS_OPENDIR(  s,       pDirHandle)                           FS_EFS_OpenDir(s, pDirHandle)
  #define FS_READDIR(  pDir, pDirEntryInfo)                           FS_EFS_ReadDir(pDir, pDirEntryInfo)
  #define FS_REMOVEDIR(pVolume, s)                                    FS_EFS_RemoveDir(pVolume, s)
  #define FS_RENAME(   s,       sNewName, pVolume)                    FS_EFS_Rename(s, sNewName, pVolume)
  #define FS_SETDIRENTRYINFO(pVolume, sName, p, Mask)                 FS_EFS_SetDirEntryInfo(pVolume, sName, p, Mask)
  #define FS_SET_END_OF_FILE(pFile)                                   FS_EFS_SetEndOfFile(pFile)
  #define FS_CLEAN_FS(pVolume)                                        FS_EFS_Clean(pVolume)
  #define FS_GET_VOLUME_LABEL(pVolume, pVolLabel, VolLabelSize)       FS_EFS_GetVolumeLabel(pVolume, pVolLabel, VolLabelSize)
  #define FS_SET_VOLUME_LABEL(pVolume, pVolLabel)                     FS_EFS_SetVolumeLabel(pVolume, pVolLabel)
  #define FS_CREATE_JOURNAL_FILE(pVolume, NumBytes, pFirstS, pNumS)   FS_EFS_CreateJournalFile(pVolume, NumBytes, pFirstS, pNumS)
  #define FS_OPEN_JOURNAL_FILE(pVolume)                               FS_EFS_OpenJournalFile(pVolume)
  #define FS_GET_INDEX_OF_LAST_SECTOR(pVolume)                        FS_EFS_GetIndexOfLastSector(pVolume)
  #define FS_CHECKDISK(pVolume, pDiskInfo, pBuffer, BufferSize, MaxRecursionLevel, pfOnError) FS_EFS__CheckDisk(pVolume, pDiskInfo, pBuffer, BufferSize, MaxRecursionLevel, pfOnError)
  #define FS_UPDATE_FILE(   hFile)                                    FS_EFS_Close(hFile)
#else
//#error FS_SUPPORT_MULTIPLE_FS: Multiple simultaneous file systems not yet supported
  #define FS_CHECK_INFOSECTOR(pVolume)                                FS_MAP_CheckFS_API(pVolume)
  #define FS_CLOSEDIR( pDir)                                          FS_MAP_CloseDir(pDir)
  #define FS_CREATEDIR(pVolume, s)                                    FS_MAP_CreateDir(pVolume, s)
  #define FS_CLOSE_FILE(   hFile)                                     FS_MAP_Close(hFile)
  #define FS_OPEN_FILE(    s,       pFile, DoDel, DoOpen, DoCreate)   FS_MAP_Open(s, pFile, DoDel, DoOpen, DoCreate)
  #define FS_FORMAT( pVolume,  pFormatInfo)                           FS_MAP_Format(pVolume, pFormatInfo)
  #define FS_FREAD(    pFile,   pData, NumBytes)                      FS_MAP_Read(pFile, pData, NumBytes)
  #define FS_FWRITE(   pFile,   pData, NumBytes)                      FS_MAP_Write(pFile, pData, NumBytes)
  #define FS_GETDIRENTRYINFO(pVolume, sName, p, Mask)                 FS_MAP_GetDirEntryInfo(pVolume, sName, p, Mask)
  #define FS_GET_DISKINFO(  pVolume, pInfo, Flags)                    FS_MAP_GetDiskInfo(pVolume, pInfo, Flags)
  #define FS_MOVE(     sSrc,    sDest, pVolume)                       FS_MAP_Move(sSrc, sDest, pVolume)
  #define FS_OPENDIR(  s,       pDirHandle)                           FS_MAP_OpenDir(s, pDirHandle)
  #define FS_READDIR(  pDir, pDirEntryInfo)                           FS_MAP_ReadDir(pDir, pDirEntryInfo)
  #define FS_REMOVEDIR(pVolume, s)                                    FS_MAP_RemoveDir(pVolume, s)
  #define FS_RENAME(   s,       sNewName, pVolume)                    FS_MAP_Rename(s, sNewName, pVolume)
  #define FS_SETDIRENTRYINFO(pVolume, sName, p, Mask)                 FS_MAP_SetDirEntryInfo(pVolume, sName, p, Mask)
  #define FS_SET_END_OF_FILE(pFile)                                   FS_MAP_SetEndOfFile(pFile)
  #define FS_CLEAN_FS(pVolume)                                        FS_MAP_Clean(pVolume)
  #define FS_GET_VOLUME_LABEL(pVolume, pVolLabel, VolLabelSize)       FS_MAP_GetVolumeLabel(pVolume, pVolLabel, VolLabelSize)
  #define FS_SET_VOLUME_LABEL(pVolume, pVolLabel)                     FS_MAP_SetVolumeLabel(pVolume, pVolLabel)
  #define FS_CREATE_JOURNAL_FILE(pVolume, NumBytes, pFirstS, pNumS)   FS_MAP_CreateJournalFile(pVolume, NumBytes, pFirstS, pNumS)
  #define FS_OPEN_JOURNAL_FILE(pVolume)                               FS_MAP_OpenJournalFile(pVolume)
  #define FS_GET_INDEX_OF_LAST_SECTOR(pVolume)                        FS_MAP_GetIndexOfLastSector(pVolume)
  #define FS_CHECKDISK(pVolume, pDiskInfo, pBuffer, BufferSize, MaxRecursionLevel, pfOnError) FS_MAP_CheckDisk(pVolume, pDiskInfo, pBuffer, BufferSize, MaxRecursionLevel, pfOnError)
  #define FS_UPDATE_FILE(   hFile)                                    FS_MAP_UpdateFile(hFile)
#endif

void        FS_MAP_Close               (FS_FILE    * pFile);
int         FS_MAP_CheckFS_API         (FS_VOLUME  * pVolume);
U32         FS_MAP_Read                (FS_FILE    * pFile,           void  * pData, U32 NumBytes);
U32         FS_MAP_Write               (FS_FILE    * pFile,     const void  * pData, U32 NumBytes);
char        FS_MAP_Open                (const char * pFileName, FS_FILE * pFile, char DoDel, char DoOpen, char DoCreate);
int         FS_MAP_Format              (FS_VOLUME  * pVolume,   FS_FORMAT_INFO * pFormatInfo);
int         FS_MAP_OpenDir             (const char * pDirName,  FS__DIR *pDir);
int         FS_MAP_CloseDir            (FS__DIR    * pDir);
int         FS_MAP_ReadDir             (FS__DIR    * pDir,      FS_DIRENTRY_INFO * pDirEntryInfo);
int         FS_MAP_RemoveDir           (FS_VOLUME  * pVolume,   const char * pDirName);
int         FS_MAP_CreateDir           (FS_VOLUME  * pVolume,   const char * pDirName);
int         FS_MAP_Rename              (const char * sOldName,  const char * sNewName, FS_VOLUME * pVolume);
int         FS_MAP_Move                (const char * sOldName,  const char * sNewName, FS_VOLUME * pVolume);
char        FS_MAP_SetDirEntryInfo     (FS_VOLUME  * pVolume,   const char * sName, const void * p, int Mask);
char        FS_MAP_GetDirEntryInfo     (FS_VOLUME  * pVolume,   const char * sName,       void * p, int Mask);
int         FS_MAP_SetEndOfFile        (FS_FILE    * pFile);
void        FS_MAP_Clean               (FS_VOLUME  * pVolume);
int         FS_MAP_GetDiskInfo         (FS_VOLUME  * pVolume,   FS_DISK_INFO * pDiskData, int Flags);
int         FS_MAP_GetVolumeLabel      (FS_VOLUME  * pVolume,   char * pVolumeLabel, unsigned VolumeLabelSize);
int         FS_MAP_SetVolumeLabel      (FS_VOLUME  * pVolume,   const char * pVolumeLabel);
int         FS_MAP_CreateJournalFile   (FS_VOLUME  * pVolume,   U32 NumBytes, U32 * pFirstSector, U32 * pNumSectors);
int         FS_MAP_OpenJournalFile     (FS_VOLUME  * pVolume);
U32         FS_MAP_GetIndexOfLastSector(FS_VOLUME  * pVolume);
int         FS_MAP_CheckDisk           (FS_VOLUME  * pVolume,   FS_DISK_INFO * pDiskInfo, void * pBuffer, U32 BufferSize, int MaxRecursionLevel, FS_QUERY_F_TYPE * pfOnError);
void        FS_MAP_UpdateFile          (FS_FILE    * pFile);



/*********************************************************************
*
*       CLIB
*
*   Optional replacements for standard "C" library routines.
*
**********************************************************************
*/

int            FS__CLIB_atoi    (const char *s);
int            FS__CLIB_memcmp  (const void *s1, const void *s2, unsigned n);
void *         FS__CLIB_memset  (void *s, int c, U32 n);
int            FS__CLIB_strcmp  (const char *s1, const char *s2);
char *         FS__CLIB_strcpy  (char *s1, const char *s2);
unsigned       FS__CLIB_strlen  (const char *s);
int            FS__CLIB_strncmp (const char *s1, const char *s2, int n);
char *         FS__CLIB_strncpy (char *s1, const char *s2, U32 n);
int            FS__CLIB_toupper (int c);
char *         FS__CLIB_strcat  (char *s1, const char *s2);
char *         FS__CLIB_strncat (char *s1, const char *s2, U32 n);


#if FS_NO_CLIB
  #define FS_ATOI(s)             FS__CLIB_atoi(s)
  #define FS_MEMCMP(s1,s2,n)     FS__CLIB_memcmp(s1,s2,n)
#ifndef FS_MEMCPY
  #define FS_MEMCPY(s1,s2,n)     FS_memcpy(s1,s2,n)
#endif
  #define FS_MEMSET(s,c,n)       FS__CLIB_memset(s,c,n)
  #define FS_STRCAT(s1,s2)       FS__CLIB_strcat(s1, s2)
  #define FS_STRCMP(s1,s2)       FS__CLIB_strcmp(s1,s2)
  #define FS_STRCPY(s1,s2)       FS__CLIB_strcpy(s1,s2)
  #define FS_STRLEN(s)           FS__CLIB_strlen(s)
  #define FS_STRNCAT(s1,s2,n)    FS__CLIB_strncat(s1, s2, n)
  #define FS_STRNCMP(s1,s2,n)    FS__CLIB_strncmp(s1,s2,n)
  #define FS_STRNCPY(s1,s2,n)    FS__CLIB_strncpy(s1,s2,n)
  #define FS_TOUPPER(c)          FS__CLIB_toupper(c)
#else
  #define FS_ATOI(s)             atoi(s)
  #define FS_MEMCMP(s1,s2,n)     memcmp(s1,s2,n)
#ifndef FS_MEMCPY
  #ifdef __ICCARM__
    #define FS_MEMCPY(s1,s2,n)     FS_memcpy(s1,s2,n)
  #else
    #define FS_MEMCPY(s1,s2,n)     memcpy(s1,s2,n)
  #endif
#endif
  #define FS_MEMSET(s,c,n)       memset(s,c,n)
  #define FS_STRCAT(s1,s2)       strcat(s1,s2)
  #define FS_STRCMP(s1,s2)       strcmp(s1,s2)
  #define FS_STRCPY(s1,s2)       strcpy(s1,s2)
  #define FS_STRLEN(s)           strlen(s)
  #define FS_STRNCAT(s1,s2,n)    strncat(s1, s2, n)
  #define FS_STRNCMP(s1,s2,n)    strncmp(s1,s2,n)
  #define FS_STRNCPY(s1,s2,n)    strncpy(s1,s2,n)
  #define FS_TOUPPER(c)          toupper(c)
#endif



/*********************************************************************
*
*       FS_LB
*
*       Logical block layer
*
**********************************************************************
*/

int    FS_LB_GetStatus           (FS_DEVICE    * pDevice);
U16    FS_GetSectorSize          (FS_DEVICE    * pDevice);
int    FS_LB_GetDeviceInfo       (FS_DEVICE    * pDevice, FS_DEV_INFO * pDevInfo);
int    FS_LB_InitMedium          (FS_DEVICE    * pDevice);
int    FS_LB_InitMediumIfRequired(FS_DEVICE    * pDevice);
int    FS_LB_Ioctl               (FS_DEVICE    * pDevice, I32 Cmd,         I32 Aux,              void * pBuffer);
void   FS_LB_FreePartSectors     (FS_PARTITION * pPart,   U32 SectorIndex, U32 NumSectors);
int    FS_LB_ReadBurst           (FS_DEVICE    * pDevice, U32 SectorNo,    U32 NumSectors,       void * pBuffer, U8 Type);
int    FS_LB_ReadBurstPart       (FS_PARTITION * pPart,   U32 SectorNo,    U32 NumSectors,       void * pBuffer, U8 Type);
int    FS_LB_ReadDevice          (FS_DEVICE    * pDevice, U32 Sector,                            void * pBuffer, U8 Type);
int    FS_LB_ReadPart            (FS_PARTITION * pPart,   U32 Sector,                            void * pBuffer, U8 Type);
int    FS_LB_WriteBurst          (FS_DEVICE    * pDevice, U32 SectorNo,    U32 NumSectors, const void * pBuffer, U8 Type);
int    FS_LB_WriteBurstPart      (FS_PARTITION * pPart,   U32 SectorNo,    U32 NumSectors, const void * pBuffer, U8 Type);
int    FS_LB_WriteDevice         (FS_DEVICE    * pDevice, U32 Sector,                      const void * pBuffer, U8 Type);
int    FS_LB_WritePart           (FS_PARTITION * pPart,   U32 Sector,                      const void * pBuffer, U8 Type);
int    FS_LB_WriteMultiple       (FS_DEVICE    * pDevice, U32 Sector,      U32 NumSectors, const void * pBuffer, U8 Type);
int    FS_LB_WriteMultiplePart   (FS_PARTITION * pPart,   U32 Sector,      U32 NumSectors, const void * pBuffer, U8 Type);


/*********************************************************************
*
*       FS_JOURNAL
*
*       Journal to make file system layer transaction and fail safe
*
**********************************************************************
*/

int FS__JOURNAL_Create           (FS_VOLUME * pVolume, U32 FirstSector, U32 NumSectors);
int FS__JOURNAL_GetNumFreeSectors(FS_VOLUME * pVolume);
int FS__JOURNAL_IsPresent        (FS_VOLUME * pVolume);
int FS__JOURNAL_Mount            (FS_VOLUME * pVolume);
int FS__JOURNAL_Read             (const FS_DEVICE * pDevice, U32 SectorNo, void * pBuffer, U32 NumSectors);
int FS__JOURNAL_Write            (const FS_DEVICE * pDevice, U32 SectorNo, const void * pBuffer, U32 NumSectors, U8 RepeatSame);
void FS__JOURNAL_DeInit          (FS_VOLUME * pVolume);

/*********************************************************************
*
*       Public const
*
**********************************************************************
*/
#if FS_SUPPORT_MULTIPLE_FS
  extern const FS_FS_API FS_FAT_API;
  extern const FS_FS_API FS_EFS_API;
#endif

#if defined(__cplusplus)
}                /* Make sure we have C-declarations in C++ programs */
#endif

#undef EXTERN

#endif                // Avoid multiple/recursive inclusion

/*************************** End of file ****************************/
