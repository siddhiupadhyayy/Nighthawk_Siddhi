/****************************************************************************/
/*                                                                          */
/*   Filename   : OsaSyncWin.c                                              */
/*                                                                          */
/*   Description: OS abstraction layer                                      */
/*                OS abstraction for synchronization for Windows            */
/*                                                                          */
/*   Copyright  : Philips Medical Systems Nederland B.V., 2012.             */
/*                                                                          */
/****************************************************************************/

#define OSA_INTERNAL

#include <stdio.h>
#include "OsaError.h"
#include "OsaSync.h"

static OsaErr SemDelete(OsaSyncId syncId);

/*
 *      Creates an unnamed semaphore object.
 */
OsaErr
OsaSemCreate(UINT16       initialValue,
             UINT16       maxValue,
             OsaSyncFlags flags,
             OsaSyncId*   syncId)
{
    OsaErr error = OSA_EOK;

    OSA_ASSERT(syncId != NULL);

    if ((initialValue <= maxValue) && (maxValue > 0))
    {
        switch (flags)
        {
            case OSA_SYNC_FIFO:             /* no break */
            case OSA_SYNC_PRIORITY:         /* no break */
                error = OSA_EOK;            /* flags parameter is valid, but not used */
                break;
            default:
                error = OSA_EPARAM;
                break;
        }

        if (OSA_EOK == error)
        {
            SetLastError(0);

            *syncId = CreateSemaphore(NULL, (LONG)initialValue, (LONG)maxValue, NULL);
            error = (*syncId == 0) ? OSA_EOBJ : OSA_EOK;

            OSA_HANDLE_GETLASTERR(OSA_GET_ERR);
        }
    }
    else
    {
        error = OSA_EPARAM;
    }
    return error;
}

/*
 *      Deletes the specified semaphore object.
 */
OsaErr
OsaSemDelete(OsaSyncId syncId)
{
    return SemDelete(syncId);
}

/*
 *      Performs the 'TAKE' operation on a specified semaphore
 */
OsaErr
OsaSemTake(OsaSyncId syncId,
           UINT32    timeOut)
{
    return OsaSyncWaitOne(syncId, timeOut);
}

/*
 *      Performs the 'GIVE' operation on a specified semaphore.
 */
OsaErr
OsaSemGive(OsaSyncId syncId)
{
    OsaErr result = OSA_ENOK;

    SetLastError(0);

    if (ReleaseSemaphore(syncId, 1, NULL))
    {
        result = OSA_EOK;
    }
    else
    {
        int err = GetLastError();

        // In VxWorks a give is allowed on a free semaphore. Mimic this behaviour here.
        
        if (err == ERROR_TOO_MANY_POSTS)
        {
            result = OSA_EOK;
        }
        else
        {
            OSA_HANDLE_GETLASTERR(err);

            result = OSA_EEXIST;
        }
    }

    return result;
}

/*
 *      Creates an unnamed mutex object.
 */
OsaErr
OsaMutexCreate(OsaSyncFlags flags,
               OsaSyncId*   syncId)
{
    OsaErr error = OSA_EOK;

    
    // OSA_ASSERT(syncId != NULL);

    switch (flags)
    {
        case OSA_SYNC_FIFO:             /* no break */
        case OSA_SYNC_PRIORITY:         /* no break */
        case OSA_SYNC_INVERSION_SAFE:   /* no break */
            error = OSA_EOK;            /* flags parameter is valid, but not used */
            break;
        default:
            error = OSA_EPARAM;
            break;
    }

    if (OSA_EOK == error)
    {
        SetLastError(0);

        *syncId = CreateMutex(NULL, FALSE, NULL);

        if (*syncId == 0)
        {
            OSA_HANDLE_GETLASTERR(OSA_GET_ERR);

            error = OSA_EOBJ;
        }
        else
        {
            error = OSA_EOK;
        }
    }
    return error;
}

/*
 *      Deletes the specified mutex object.
 */
OsaErr
OsaMutexDelete(OsaSyncId syncId)
{
    return SemDelete(syncId);
}

/*
 *      performs the 'TAKE' operation on the specified Mutex object
 */
OsaErr
OsaMutexTake(OsaSyncId syncId,
             UINT32 timeOut)
{
    return OsaSyncWaitOne(syncId, timeOut);
}

/*
 *      performs the 'GIVE' operation on the specified Mutex object
 */
OsaErr
OsaMutexGive(OsaSyncId syncId)
{
    OsaErr  result  = OSA_EOK;
    BOOL    retVal  = FALSE;

    SetLastError(0);

    retVal = ReleaseMutex(syncId);

    if ( ! retVal)
    {
        INT32 lastErr = GetLastError();

        OSA_HANDLE_GETLASTERR(lastErr);

        result = (lastErr == ERROR_NOT_OWNER) ? OSA_ENOOWNER : OSA_EEXIST;
    }

    return result;
}

/*
 *      Performs the take operation on the specified sync object.
 */
OsaErr
OsaSyncWaitOne(OsaSyncId syncId,
               UINT32 timeOut)
{
    UINT32 timeoutWin = 0;
    DWORD  status     = 0;

    OsaErr result = OSA_ENOK;

    switch (timeOut)
    {
    case OSA_NO_WAIT:
        timeoutWin = 0;
        break;

    case OSA_INFINITE:
        timeoutWin = INFINITE;
        break;

    default:
        timeoutWin = timeOut;
        break;
    }

    SetLastError(0);

    status = WaitForSingleObject(syncId, timeoutWin);

    switch (status)
    {
    case WAIT_FAILED:    

        OSA_HANDLE_GETLASTERR(OSA_GET_ERR);
        result = OSA_EEXIST;
        break;

    case WAIT_ABANDONED:    /* access granted, by termination of the owning thread */
    case WAIT_OBJECT_0:

        result = OSA_EOK;
        break;

    case WAIT_TIMEOUT:

        result = OSA_ETIME;
        break;

    default:

        OSA_HANDLE_GETLASTERR(OSA_GET_ERR);
        result = OSA_EEXIST;
        break;
    }

    return result;
}

/*
 *      Deletes the specified sync object.
 */
OsaErr
SemDelete(OsaSyncId syncId)
{
    OsaErr  osaErr      = OSA_EEXIST;
    errno_t localErrno  = 0;
    int     succes;

    SetLastError(0);

    succes = CloseHandle(syncId);
    
    (VOID) _get_errno(&localErrno);

    if ((succes != 0) && (localErrno == 0))
    {
        osaErr = OSA_EOK;
    }

    OSA_HANDLE_GETLASTERR(localErrno);

    return osaErr;
}