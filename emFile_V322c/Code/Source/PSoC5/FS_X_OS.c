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
----------------------------------------------------------------------
File        : FS_X_OS.c
Purpose     : General template OS Layer for the file system
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*             #include Section
*
**********************************************************************
*/

#include "FS_Int.h"
#include "FS_OS.h"
#include "FS_Conf.h"


/*********************************************************************
*
*       Static data
*
**********************************************************************
*/


/*********************************************************************
*
*       Public code
*
**********************************************************************
*/


/*********************************************************************
*
*         FS_X_OS_Lock
*
*  Description:
*    Locks FS_Sema[LockIndex] semaphore.
*
*/
void FS_OS_Lock(unsigned LockIndex) {
	FS_USE_PARA(LockIndex);
}


/*********************************************************************
*
*       FS_X_OS_Unlock
*
*  Description:
*    Unlocks FS_Sema[LockIndex] semaphore.
*
*/
void FS_OS_Unlock(unsigned LockIndex) {
	FS_USE_PARA(LockIndex);
}

/*********************************************************************
*
*             FS_X_OS_init
*
*  Description:
*    Initializes the OS resources.
*
*  Parameters:
*    None.
*
*  Return value:
*    0    - on success
*    -1   - on failure.
*/
void FS_OS_Init(unsigned MaxNumLocks) {
  unsigned i;

  for (i = 0; i < MaxNumLocks; i++) {
  }
}

/*********************************************************************
*
*       FS_X_OS_DeInit
*
*  Description:
*    Delete all locks that have been created by FS_X_OS_Init().
*    This makes sure that a
*
*  Parameters:
*    None.
*
*/
void FS_OS_DeInit(void) {
  unsigned i;
  unsigned    NumLocks = 0;

  for (i = 0; i < NumLocks; i++) {
  }
}
