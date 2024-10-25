/****************************************************************************/
/*                                                                          */
/*   Filename   : OsaErrorWin.c                                             */
/*                                                                          */
/*   Description: OS abstraction layer                                      */
/*                OS abstraction error implementation for Windows           */
/*                                                                          */
/*   Copyright  :                                                           */
/*                                                                          */
/****************************************************************************/

#define OSA_INTERNAL

#include <stdio.h>

#include "OsaGenTypes.h"
#include "OsaInit.h"
#include "OsaFile.h"
#include "OsaString.h"
#include "OsaError.h"

#define BUFFER_SIZE 1024

/* Formats the error string using ErrNo. */
VOID
OsaHandleErrno(CHAR* fileName, 
               INT32 lineNr, 
               INT32 errNo)
{
    CHAR  errBuf[BUFFER_SIZE];
    CHAR  msgBuf[BUFFER_SIZE];
    OsaErrorCallbackFunc cb = NULL;

    
    OSA_ASSERT(OsaInitIsInitialized());

    if (errNo == OSA_GET_ERR)                                                 
    {    
        (VOID)_get_errno(&errNo);
    }

    switch (errNo)
    {
        case 0:             /* Everything is ok */
            break;

        default:
            strerror_s(errBuf, sizeof(errBuf), errNo);
            cb = OsaInitGetOsaErrorCallbackFunc();
            if (cb != NULL)
            {
                (void) OsaFileSPrintF(msgBuf, sizeof(msgBuf), "Error(%d): %s.\n"
                                                              "File: %s Line: %d",
                                                               errNo, errBuf,
                                                               fileName, lineNr);
                cb(msgBuf);
            }

            SetLastError(errNo);                                              
            break;
    }
}

/* Formats the error string using ErrNo. */
VOID
OsaFormatErr(CHAR* fileName, 
             INT32 lineNr, 
             INT32 errNo, 
             BOOL  wsa)
{
    LPVOID errBuf = NULL;
    CHAR   msgBuf[BUFFER_SIZE];

    OSA_ASSERT(OsaInitIsInitialized());

    if (errNo == OSA_GET_ERR)                                                 
    {    
        if (wsa)
        {
            errNo = WSAGetLastError();
        }
        else
        {
            errNo = GetLastError();
        }
    }

    switch (errNo)
    {
    case 0:             /* Everything is ok */
        break;

    default:

        FormatMessage(  FORMAT_MESSAGE_FROM_SYSTEM     |    /* use system message tables to retrieve error text */                                       
                        FORMAT_MESSAGE_ALLOCATE_BUFFER |    /* allocate buffer on local heap for error text */                                               
                        FORMAT_MESSAGE_IGNORE_INSERTS,      /* Important! will fail otherwise, since we're not (and CANNOT) pass insertion parameters  */
                        NULL,                               /* unused with FORMAT_MESSAGE_FROM_SYSTEM  */              
                        errNo,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        (LPTSTR)&errBuf,                    /* output */                                                        
                        0,                                  /* minimum size for output buffer */
                        NULL);

        if (errBuf != NULL)
        {
            OsaErrorCallbackFunc cb = OsaInitGetOsaErrorCallbackFunc();

            if (cb != NULL)
            {
                (void) OsaFileSPrintF(msgBuf, sizeof(msgBuf), "Error(%d): %s"
                                                              "File: %s Line: %d",
                                                               errNo, errBuf, 
                                                               fileName, lineNr);                                                      
                cb(msgBuf);
            }

            /* release memory allocated by FormatMessage() */
            
            LocalFree(errBuf);
            errBuf = NULL;
        }
        
        SetLastError(errNo);                                                  
        break;
    }
}

#ifdef NDEBUG

/* Local invalid parameter handler */
void
OsaInvalidParameterHandler(const wchar_t*   expression,
                           const wchar_t*   function, 
                           const wchar_t*   file, 
                           unsigned int     line, 
                           uintptr_t        reserved)
{
    OsaInvalidParCallbackFunc cb = NULL;

    UNREFERENCED_PARAMETER(expression);
    UNREFERENCED_PARAMETER(function);
    UNREFERENCED_PARAMETER(file);
    UNREFERENCED_PARAMETER(line);
    UNREFERENCED_PARAMETER(reserved);

    OSA_ASSERT(OsaInitIsInitialized());

    cb = OsaInitGetInvalidParCallback();

    if (cb != NULL)
    {
        cb("Invalid parameter detected!");
    }

    if (OsaInitExitOnErr())
    {
        exit(2);                                                              
    }
}

#else   /* ! NDEBUG */

/* Buffer size definitions. */

#define BUF_SIZE        256

/* Converts a sequence of wide characters to a */
/* corresponding sequence of characters */
static BOOL
ConvertToTypeChar(CHAR*          out, 
                  size_t         outSz, 
                  const wchar_t* in)
{
    size_t  count = 0;

    int err = wcstombs_s(&count, out, outSz, in, _TRUNCATE);

    return (err != 0);
}

/* Local invalid parameter handler */
void
OsaInvalidParameterHandler(const wchar_t*   expression,
                           const wchar_t*   function, 
                           const wchar_t*   file, 
                           unsigned int     line, 
                           uintptr_t        reserved)
{
    OsaInvalidParCallbackFunc cb = NULL;
    CHAR msgBuf[BUFFER_SIZE];

    UNREFERENCED_PARAMETER(reserved);
    UNREFERENCED_PARAMETER(file);
    UNREFERENCED_PARAMETER(line);

    OSA_ASSERT(OsaInitIsInitialized());

    cb = OsaInitGetInvalidParCallback();

    if (cb != NULL)
    {
        CHAR sExpression[BUF_SIZE];
        CHAR   sFunction[BUF_SIZE];

        ConvertToTypeChar(sExpression, BUF_SIZE, expression);
        ConvertToTypeChar(sFunction,   BUF_SIZE, function);

        (void)OsaFileSPrintF(msgBuf, sizeof(msgBuf), "Invalid parameter detected in function %s\n"
                                                     "Expression: %s",
                                                     sFunction, sExpression);
        cb(msgBuf);
    }

    if (OsaInitExitOnErr())
    {
        exit(2);                                                            
    }
}

#endif  /* NDEBUG */
