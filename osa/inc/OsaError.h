/****************************************************************************/
/*                                                                          */
/*   Filename   : OsaError.h                                                */
/*                                                                          */
/*   Description: OS abstraction layer interface                            */
/*                The library error handling                                */
/*                                                                          */
/*   Copyright  :                                                           */
/*                                                                          */
/****************************************************************************/

#ifndef OSAERROR_H
#define OSAERROR_H    

#include "OsaGenTypes.h"

/* Get the error text for an OSA error value */
OSADLL const CHAR*
OsaErrStr(OsaErr error);

#ifdef OSA_INTERNAL

/* For internal use only; excluded from Doxygen */

#define OSA_HANDLE_ERRNO(errNo)             OsaHandleErrno(__FILE__, __LINE__, (errNo))                 
#define OSA_HANDLE_GETLASTERR(errNo)        OsaFormatErr  (__FILE__, __LINE__, (errNo), FALSE)          
#define OSA_HANDLE_WSAGETLASTERR(errNo)     OsaFormatErr  (__FILE__, __LINE__, (errNo), TRUE)           

#define OSA_GET_ERR     INT32_MIN

/* <b>OSA_INTERNAL!</b><br>Formats the error string using ErrNo (strerror_s variant) */
void
OsaHandleErrno(CHAR* fileName, 
               INT32 lineNr, 
               INT32 errNo);

#ifdef WIN32
/* <b>OSA_INTERNAL!</b><br>Formats the error string using ErrNo (FormatMessage variant) */
void
OsaFormatErr(CHAR* fileName, 
             INT32 lineNr, 
             INT32 errNo, 
             BOOL  wsa);
#endif  /* WIN32 */

#ifdef WIN32
/* <b>OSA_INTERNAL!</b><br>Local invalid paramater handler */
/* NOTE : parameters only have value int DEBUG mode. */
void
OsaInvalidParameterHandler(const wchar_t*   expression,
                           const wchar_t*   function,
                           const wchar_t*   file,
                           unsigned int     line,
                           uintptr_t        reserved);

#endif  /* WIN32 */

/*! \endcond
 *  Include for doxygen from here
 */

#endif  /* OSA_INTERNAL */

#endif  /* OSAERROR_H */
