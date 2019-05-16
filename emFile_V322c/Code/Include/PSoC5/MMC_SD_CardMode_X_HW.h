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
File        : MMC_SD_CardMode_X_HW.h
Purpose     : MMC hardware layer
---------------------------END-OF-HEADER------------------------------
*/

#ifndef __MMC_SD_CARDMODE_X_HW_H__
#define __MMC_SD_CARDMODE_X_HW_H__


/*********************************************************************
*
*        Defines: Error codes
*
**********************************************************************
*/
#define FS_MMC_CARD_NO_ERROR               0
#define FS_MMC_CARD_RESPONSE_TIMEOUT       1
#define FS_MMC_CARD_RESPONSE_CRC_ERROR     2
#define FS_MMC_CARD_READ_TIMEOUT           3
#define FS_MMC_CARD_READ_CRC_ERROR         4
#define FS_MMC_CARD_WRITE_CRC_ERROR        5
#define FS_MMC_CARD_RESPONSE_GENERIC_ERROR 6
#define FS_MMC_CARD_READ_GENERIC_ERROR     7
#define FS_MMC_CARD_WRITE_GENERIC_ERROR    8


#define FS_MMC_RESPONSE_FORMAT_NONE        0
#define FS_MMC_RESPONSE_FORMAT_R1          1
#define FS_MMC_RESPONSE_FORMAT_R2          2
#define FS_MMC_RESPONSE_FORMAT_R3          3
#define FS_MMC_RESPONSE_FORMAT_R6          1  /* Response format R6 is the same as R1 */

#define FS_MMC_CMD_FLAG_DATATRANSFER    (1 <<  0)
#define FS_MMC_CMD_FLAG_WRITETRANSFER   (1 <<  1)
#define FS_MMC_CMD_FLAG_SETBUSY         (1 <<  2)
#define FS_MMC_CMD_FLAG_INITIALIZE      (1 <<  3)
#define FS_MMC_CMD_FLAG_USE_SD4MODE     (1 <<  4)
#define FS_MMC_CMD_FLAG_STOP_TRANS      (1 <<  5)
#define FS_MMC_CMD_FLAG_REPEAT_SAME_SECTOR_DATA (1 <<  6)

/*********************************************************************
*
*             Global function prototypes
*
**********************************************************************
*/

void   FS_MMC_HW_X_Delay              (int ms);
int    FS_MMC_HW_X_GetResponse        (U8 Unit, void *pBuffer, U32 Size);
int    FS_MMC_HW_X_IsPresent          (U8 Unit);
int    FS_MMC_HW_X_IsWriteProtected   (U8 Unit);
int    FS_MMC_HW_X_ReadData           (U8 Unit, void * pBuffer, unsigned NumBytes, unsigned NumBlocks);
void   FS_MMC_HW_X_SendCmd            (U8 Unit, unsigned Cmd, unsigned CmdFlags, unsigned ResponseType, U32 Arg);
void   FS_MMC_HW_X_SetResponseTimeOut (U8 Unit, U32 Value);
void   FS_MMC_HW_X_SetReadDataTimeOut (U8 Unit, U32 Value);
void   FS_MMC_HW_X_SetHWBlockLen      (U8 Unit, U16 BlockSize);
void   FS_MMC_HW_X_SetHWNumBlocks     (U8 Unit, U16 NumBlocks);
U16    FS_MMC_HW_X_SetMaxSpeed        (U8 Unit, U16 Freq);
int    FS_MMC_HW_X_WriteData          (U8 Unit, const void * pBuffer, unsigned NumBytes, unsigned NumBlocks);
void   FS_MMC_HW_X_InitHW             (U8 Unit);
U16    FS_MMC_HW_X_GetMaxReadBurst    (U8 Unit);
U16    FS_MMC_HW_X_GetMaxWriteBurst   (U8 Unit);
void   FS_MMC_HW_X_SetDataPointer     (U8 Unit, const void * p);
#endif  /* __MMC_SD_CARDMODE_X_HW_H__ */

/*************************** End of file ****************************/
