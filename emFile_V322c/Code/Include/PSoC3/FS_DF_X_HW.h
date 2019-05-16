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
File        : DataFlash_X_HW.h
Purpose     : Data Flash hardware layer
---------------------------END-OF-HEADER------------------------------
*/

#ifndef __DATAFLASH_X_HW_H__
#define __DATAFLASH_X_HW_H__

/*********************************************************************
*
*       Global function prototypes
*
**********************************************************************
*/

/* Control line functions */
void          FS_DF_HW_X_EnableCS   (U8 Unit);
void          FS_DF_HW_X_DisableCS  (U8 Unit);
int           FS_DF_HW_X_Init       (U8 Unit);

/* Data transfer functions */
void          FS_DF_HW_X_Read (U8 Unit,       U8 * pData, int NumBytes);
void          FS_DF_HW_X_Write(U8 Unit, const U8 * pData, int NumBytes);

#endif  /* __DATAFLASH_X_HW_H__ */

/*************************** End of file ****************************/
