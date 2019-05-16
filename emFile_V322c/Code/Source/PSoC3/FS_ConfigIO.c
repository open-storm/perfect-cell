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
File        : FS_ConfigIO.c
Purpose     : I/O Configuration routines for Filesystem
---------------------------END-OF-HEADER------------------------------
*/

#include <stdio.h>
#include <device.h>
#include "FS.h"

/*********************************************************************
*
*       FS_X_Panic
*
*  Function description
*    Referred in debug builds of the file system only and
*    called only in case of fatal, unrecoverable errors.
*/
void FS_X_Panic(int ErrorCode) {
  FS_USE_PARA(ErrorCode);
  while (1);
}

/*********************************************************************
*
*      Logging: OS dependent

Note:
  Logging is used in higher debug levels only. The typical target
  build does not use logging and does therefore not require any of
  the logging routines below. For a release build without logging
  the routines below may be eliminated to save some space.
  (If the linker is not function aware and eliminates unreferenced
  functions automatically)

*/
void FS_X_Log(const char *s) {
  FS_USE_PARA(s);
}

void FS_X_Warn(const char *s) {
  FS_USE_PARA(s);
}

void FS_X_ErrorOut(const char *s) {
  FS_USE_PARA(s);
}

/*************************** End of file ****************************/
