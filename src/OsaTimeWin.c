/****************************************************************************/
/*                                                                          */
/*   Filename   : OsaTimeWin.c                                              */
/*                                                                          */
/*   Description: OS abstraction layer                                      */
/*                OS abstraction for time handling for Windows              */
/*                                                                          */
/*   Copyright  :                                                           */
/*                                                                          */
/****************************************************************************/
#define OSA_INTERNAL

#include <time.h>
#include <errno.h>
#include <Windows.h>
#include "OsaError.h"
#include "OsaTime.h"

/* The following is copied from ctime.h */

#define MAXTIME64T     0x793406fffi64       /* number of seconds from
                                                   00:00:00, 01/01/1970 UTC to
                                                   23:59:59. 12/31/3000 UTC */

/* OsaTimeConvertToLocalTime converts the unstructured time to */
/* a structured local time. */

OsaErr 
OsaTimeConvertToLocalTime(struct tm* timeBuffer,
                          time_t     timeUnstructured)
{
    OsaErr result = OSA_EOK;
    int parametersCorrect = TRUE;

    OSA_ASSERT(NULL != timeBuffer);

    parametersCorrect = parametersCorrect && (0          <= timeUnstructured);
    parametersCorrect = parametersCorrect && (MAXTIME64T >= timeUnstructured);

    if(! parametersCorrect)
    {
        result = OSA_EPARAM;
    }
    else 
    {
        /* Note that the parameters may still be incorrect (e.g. 31-feb-XXXX) */
        
        errno_t err =  localtime_s(timeBuffer, &timeUnstructured);

        if (0 != err)
        {
            result = OSA_ENOK;

            if(EINVAL == err) 
            {
                result = OSA_EPARAM;
            }

            OSA_HANDLE_ERRNO(err);
        }
    } 
    return result;
}

/* OsaTimeConvertToLocalTime converts the unstructured 32 bits time to a */
/* structured local time. */
 
OsaErr 
OsaTimeConvertToLocalTime32(struct tm* timeBuffer,
                            __time32_t timeUnstructured)
{
    OsaErr result = OSA_EOK;

    int parametersCorrect = TRUE;

    OSA_ASSERT(NULL != timeBuffer);

    parametersCorrect = parametersCorrect && (0 <= timeUnstructured);

    if(! parametersCorrect)
    {
        result = OSA_EPARAM;
    }
    else 
    {
        errno_t err = _localtime32_s(timeBuffer, &timeUnstructured);

        if(0 != err)
        {
            result = OSA_ENOK;

            if(EINVAL == err) 
            {
                result = OSA_EPARAM;
            }          

            OSA_HANDLE_ERRNO(err);
        }
    }                     
    return result;
}

/* retrieves the local system date and time and places it in a */
/* structured time buffer. */

OsaErr 
OsaTimeGetLocalTime(OsaTime* timeBuffer)
{
    OsaErr result    = OSA_EOK;
    INT32  lastError = 0;
    SYSTEMTIME SystemTime;

    OSA_ASSERT(NULL != timeBuffer);

    SetLastError(0);
    GetLocalTime(&SystemTime); // no error is returned
    lastError = GetLastError(); 

    switch (lastError)
    {
    case 0:
         result = OSA_EOK;
        break;
    case EINVAL:
        result = OSA_EPARAM;
        break;
    default:
        result = OSA_ENOK;
        break;
    }

    OSA_HANDLE_ERRNO(lastError);
    
    timeBuffer->Year         =  SystemTime.wYear;
    timeBuffer->Month        =  SystemTime.wMonth;
    timeBuffer->DayOfWeek    =  SystemTime.wDayOfWeek;
    timeBuffer->Day          =  SystemTime.wDay;
    timeBuffer->Hour         =  SystemTime.wHour;
    timeBuffer->Minute       =  SystemTime.wMinute;
    timeBuffer->Second       =  SystemTime.wSecond;
    timeBuffer->Milliseconds =  SystemTime.wMilliseconds;

    return result;
}

/* Sets the system date and time from a structured local time buffer. */

OsaErr 
OsaTimeSetLocalTime(OsaTime* timeBuffer)
{
    OsaErr result = OSA_EOK;
    int parametersCorrect = TRUE;

    OSA_ASSERT(NULL != timeBuffer);

    parametersCorrect = parametersCorrect && ( 1600 < timeBuffer->Year);
    parametersCorrect = parametersCorrect && (30828 > timeBuffer->Year);
    parametersCorrect = parametersCorrect && (    0 < timeBuffer->Month);
    parametersCorrect = parametersCorrect && (   13 > timeBuffer->Month);
    //  timeBuffer->DayOfWeek  is ignored         
    parametersCorrect = parametersCorrect && (    0 < timeBuffer->Day);
    parametersCorrect = parametersCorrect && (   32 > timeBuffer->Day);
    parametersCorrect = parametersCorrect && (   24 > timeBuffer->Hour);
    parametersCorrect = parametersCorrect && (   60 > timeBuffer->Minute);
    parametersCorrect = parametersCorrect && (   60 > timeBuffer->Second);
    parametersCorrect = parametersCorrect && ( 1000 > timeBuffer->Milliseconds); 

    if(! parametersCorrect)
    {
        result = OSA_EPARAM;
    }
    else 
    {
        BOOL        err;
        INT32       lastError = 0;
        SYSTEMTIME  SystemTime;

        SystemTime.wYear         =  timeBuffer->Year;
        SystemTime.wMonth        =  timeBuffer->Month;
        SystemTime.wDayOfWeek    =  timeBuffer->DayOfWeek;
        SystemTime.wDay          =  timeBuffer->Day;
        SystemTime.wHour         =  timeBuffer->Hour;
        SystemTime.wMinute       =  timeBuffer->Minute;
        SystemTime.wSecond       =  timeBuffer->Second;
        SystemTime.wMilliseconds =  timeBuffer->Milliseconds;

        SetLastError(0);
        err = SetLocalTime(&SystemTime);  
        lastError = GetLastError();

        if (0 == err)
        {
            switch (lastError)
            {
            case EINVAL:
                result = OSA_EPARAM;
                break;
            default:
                result = OSA_ENOK;
                break;
            }
            
            OSA_HANDLE_ERRNO(lastError);
        }
    }
    return result;
}

/* OsaTimeGetSystemTime returns the system time in milliseconds */
UINT32
OsaTimeGetSystemTime(VOID)
{
    return timeGetTime();
}

/* OsaTimeConvertToOsaTime converts the unstructured time to */
/* a structured local time (OsaTime). */

OsaErr
OsaTimeConvertToOsaTime(OsaTime* timeBuffer,
                        time_t   timeUnstructured)
{
    OsaErr result       = OSA_EOK;
    struct tm localTime;

    OSA_ASSERT(NULL != timeBuffer);

    result = OsaTimeConvertToLocalTime(&localTime, timeUnstructured);
    if (OSA_EOK == result)
    {
        timeBuffer->Year         = (UINT16)(localTime.tm_year + 1900);  /* in tm 0 equals the year 1900 */
        timeBuffer->Month        = (UINT16)(localTime.tm_mon + 1);      /* in tm 0 equals January */
        timeBuffer->Day          = (UINT16)(localTime.tm_mday);
        timeBuffer->Hour         = (UINT16)(localTime.tm_hour);
        timeBuffer->Minute       = (UINT16)(localTime.tm_min);
        timeBuffer->Second       = (UINT16)(localTime.tm_sec);
        timeBuffer->Milliseconds = 0;                                   /* not available in time_t (and tm) */
        timeBuffer->DayOfWeek    = (UINT16)(localTime.tm_wday + 1);     /* in tm 0 equals Sunday */
    }

    return result;
}
