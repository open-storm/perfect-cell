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
File        : FS_OS.h
Purpose     : File system's OS Layer header file
----------------------------------------------------------------------
Known problems or limitations with current version
----------------------------------------------------------------------
None.
---------------------------END-OF-HEADER------------------------------
*/

#ifndef _FS_OS_H_

#define _FS_OS_H_

#include "FS_ConfDefaults.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

void FS_X_OS_Lock   (unsigned LockIndex);
void FS_X_OS_Unlock (unsigned LockIndex);

void FS_X_OS_Init   (unsigned MaxNumLocks);
void FS_X_OS_DeInit (void);

U32  FS_X_OS_GetTime(void);

int  FS_X_OS_Wait   (int Timeout);
void FS_X_OS_Signal (void);

#if defined(__cplusplus)
}                /* Make sure we have C-declarations in C++ programs */
#endif

#endif    /*  _FS_OS_H_  */

/*************************** End of file ****************************/
