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
File        : MMC_MCI_HW.h
Purpose     : Generic MMC/SD card driver for Atmel generic MCI controller
---------------------------END-OF-HEADER------------------------------
*/

#ifndef MMC_MCI_HW_H
#define MMC_MCI_HW_H

typedef struct {
  U32 BaseAddr;
  U32 Mode;
} MCI_INFO;

typedef void(ISR_FUNC)(void);

void FS_MCI_HW_EnableClock         (U8 Unit, unsigned OnOff);
void FS_MCI_HW_EnableISR           (U8 Unit, ISR_FUNC * pISRHandler);
U32  FS_MCI_HW_GetMClk             (U8 Unit);
void FS_MCI_HW_GetMCIInfo          (U8 Unit, MCI_INFO * pInfo);
void FS_MCI_HW_Init                (U8 Unit);
int  FS_MCI_HW_IsCardPresent       (U8 Unit);
U8   FS_MCI_HW_IsCardWriteProtected(U8 Unit);

void FS_MCI_HW_CleanDCacheRange(void * p, unsigned NumBytes);
void FS_MCI_HW_InvalidateDCache(void * p, unsigned NumBytes);

#endif //#ifndef MMC_MCI_HW_H

