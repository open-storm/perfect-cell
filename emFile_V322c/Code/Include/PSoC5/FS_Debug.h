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
File        : FS_Debug.h
Purpose     : Debug macros
----------------------------------------------------------------------
   Debug macros for logging

 In the FS Simulation, all output is transferred into the log window.
*/

#ifndef FS_DEBUG_H
#define FS_DEBUG_H

#include <stdarg.h>
#include "SEGGER.h"
#include "FS_ConfDefaults.h"


#define FS_DEBUG_LEVEL_NOCHECK       0  /* No run time checks are performed */
#define FS_DEBUG_LEVEL_CHECK_PARA    1  /* Parameter checks are performed to avoid crashes */
#define FS_DEBUG_LEVEL_CHECK_ALL     2  /* Parameter checks and consistency checks are performed */
#define FS_DEBUG_LEVEL_LOG_ERRORS    3  /* Errors are recorded */
#define FS_DEBUG_LEVEL_LOG_WARNINGS  4  /* Errors & Warnings are recorded */
#define FS_DEBUG_LEVEL_LOG_ALL       5  /* Errors, Warnings and Messages are recorded. */

#ifndef FS_DEBUG_LEVEL
  #ifdef WIN32
    #define FS_DEBUG_LEVEL FS_DEBUG_LEVEL_LOG_WARNINGS  /* Simulation should log all warnings */
  #else
    #define FS_DEBUG_LEVEL FS_DEBUG_LEVEL_CHECK_PARA  /* For most targets, min. size is important */
  #endif
#endif

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Logging (for debugging primarily)
*
**********************************************************************
*/

void FS_AddWarnFilter (U32 FilterMask);
void FS_SetWarnFilter (U32 FilterMask);
void FS_AddLogFilter  (U32 FilterMask);
void FS_SetLogFilter  (U32 FilterMask);
void FS_AddErrorFilter(U32 FilterMask);
void FS_SetErrorFilter(U32 FilterMask);

void FS_PrintfSafe(char * pBuffer, const char * sFormat, int BufferSize, va_list * pParamList);
void FS_ErrorOutf(U32 Type, const char * sFormat, ...);
void FS_Logf     (U32 Type, const char * sFormat, ...);
void FS_Warnf    (U32 Type, const char * sFormat, ...);

/*******************************************************************
*
*       Commandline
*
********************************************************************
*/

/*******************************************************************
*
*       Error macros
*
********************************************************************
*/

/* Make sure the macros are actually defined */

#if FS_DEBUG_LEVEL >= FS_DEBUG_LEVEL_LOG_ERRORS
  #define FS_DEBUG_ERROROUT(s)              FS_ErrorOutf s
  #define FS_DEBUG_ERROROUT_IF(exp,s)              { if (exp) FS_DEBUG_ERROROUT (s); }
#else
  #define FS_DEBUG_ERROROUT(s)
#endif

/*******************************************************************
*
*       Warning macros
*
********************************************************************
*/

/* Make sure the macros are actually defined */

#if FS_DEBUG_LEVEL >= FS_DEBUG_LEVEL_LOG_WARNINGS
  #define FS_DEBUG_WARN(s)              FS_Warnf s
  #define FS_DEBUG_WARN_IF(MType,exp,s)                  { if (exp) FS_DEBUG_WARN((MType, s)); }
#else
  #define FS_DEBUG_WARN(s)
  #define FS_DEBUG_WARN_IF(MType,exp,s)
#endif

/*******************************************************************
*
*       Logging macros
*
********************************************************************
*/
/* Make sure the macros are actually defined */

#if FS_DEBUG_LEVEL >= FS_DEBUG_LEVEL_LOG_ALL
  #define FS_DEBUG_LOG(s)              FS_Logf s
  #define FS_DEBUG_LOG_IF(MType,exp,s)                   { if (exp) FS_DEBUG_LOG((MType, s)); }
#else
  #define FS_DEBUG_LOG(s)
  #define FS_DEBUG_LOG_IF(MType,exp,s)
#endif



/*******************************************************************
*
*       Asserts
*
********************************************************************
*/
#if FS_DEBUG_LEVEL >= FS_DEBUG_LEVEL_LOG_ERRORS
  #define FS_DEBUG_ASSERT(MType,exp)                     { if (!(exp)) FS_DEBUG_ERROROUT ((MType, (#exp)));}
#else
  #define FS_DEBUG_ASSERT(MType,exp)
#endif

#if defined(__cplusplus)
  }              /* Make sure we have C-declarations in C++ programs */
#endif

#endif /* FS_DEBUG_H */




/*************************** End of file ****************************/
