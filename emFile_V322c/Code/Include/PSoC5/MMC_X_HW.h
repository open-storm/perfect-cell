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
File        : MMC_X_HW.h
Purpose     : MMC hardware layer
---------------------------END-OF-HEADER------------------------------
*/

#ifndef __MMC_X_HW_H__
#define __MMC_X_HW_H__

#include "Global.h"

/*********************************************************************
*
*             Global function prototypes
*
**********************************************************************
*/

/* Control line functions */
void  FS_MMC_HW_X_EnableCS   (U8 Unit);
void  FS_MMC_HW_X_DisableCS  (U8 Unit);

/* Medium status functions */
int   FS_MMC_HW_X_IsPresent        (U8 Unit);
int   FS_MMC_HW_X_IsWriteProtected (U8 Unit);

/* Operation condition detection & adjusting */
U16   FS_MMC_HW_X_SetMaxSpeed(U8 Unit, U16 MaxFreq);
int   FS_MMC_HW_X_SetVoltage (U8 Unit, U16 Vmin,   U16 Vmax);

/* Data transfer functions */
void  FS_MMC_HW_X_Read (U8 Unit,       U8 * pData, int NumBytes);
void  FS_MMC_HW_X_Write(U8 Unit, const U8 * pData, int NumBytes);

#endif  /* __MMC_X_HW_H__ */

/*************************** End of file ****************************/
