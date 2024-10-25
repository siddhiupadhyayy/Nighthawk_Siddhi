/****************************************************************************/
/*                                                                          */
/*   Filename   : OsaMsqQWin.c                                              */
/*                                                                          */
/*   Description: OS abstraction layer                                      */
/*                OS abstraction for message qeueus allocation for Windows  */
/*                                                                          */
/*   Copyright  :                                                           */
/*                                                                          */
/****************************************************************************/

#define OSA_INTERNAL

#include <windows.h>
#include "OsaError.h"
#include "OsaMsgQ.h"
#include "OsaMem.h"
#include "OsaString.h"

const char mailslotNamePrefix[] = "\\\\.\\mailslot\\";
                      
/*
 *      Creates a message queue
 */
OsaErr
OsaMsgQCreate(UINT32        maxMsgs,
              size_t        maxMsgSize,
              const CHAR*   name,
              OsaMsgQId*    msgQId)
{
    OsaErr      error       = OSA_ENOK;
    size_t      nameLength  = 0;
    DWORD       lastError   = 0;

    OSA_ASSERT(name   != NULL);
    OSA_ASSERT(msgQId != NULL);

    UNREFERENCED_PARAMETER(maxMsgs);

    *msgQId = NULL;
    nameLength = strlen(name);
    if (0 < nameLength)
    {
        /* create the fullname of the mailslot */
        CHAR*       fullName        = NULL;
        size_t      fullNameLength  = sizeof(mailslotNamePrefix) + nameLength;
        OsaErr      memError        = OsaMemAlloc((VOID**)&fullName, fullNameLength + 1);  
        if (memError == OSA_EOK)
        {
            /* create the mailslot */
            HANDLE  mailslot    = INVALID_HANDLE_VALUE;

            if (OSA_EOK == OsaStrNCpy(fullName, fullNameLength + 1, mailslotNamePrefix, sizeof(mailslotNamePrefix)))
            {
                if (OSA_EOK == OsaStrNCat(fullName, fullNameLength + 1, name, nameLength))
                {
                    SetLastError(0);
                    mailslot = CreateMailslot(fullName,               /* name of the mailslot */
                                              maxMsgSize,             /* maximum message size */
                                              0,                      /* read timeout */
                                              NULL);                  /* security attributes */
                }
            }
            if (mailslot != INVALID_HANDLE_VALUE)
            {
                /* create a write handle */
                HANDLE  writer  = NULL;
                SetLastError(0);
                writer = CreateFile(fullName,               /* the file/mailslot name */
                                    GENERIC_WRITE,          /* access mode */
                                    FILE_SHARE_READ,        /* share mode */
                                    NULL,                   /* security attributes */
                                    OPEN_EXISTING,          /* action to take */
                                    FILE_ATTRIBUTE_NORMAL,  /* device attributes and flags */
                                    NULL);                  /* template file */
                if (writer != INVALID_HANDLE_VALUE)
                {
                    /* create object to be used as OsaMsgQId */
                    memError = OsaMemAlloc((VOID**)msgQId, sizeof(**msgQId));
                    if (memError == OSA_EOK)
                    {
                        (*msgQId)->readHandle   = mailslot;
                        (*msgQId)->writeHandle  = writer;
                        error = OSA_EOK ;
                    }
                    else    /* ! (memError == OSA_EOK)      failed to allocate memory for OsaMsgQId */
                    {
                        (VOID)CloseHandle(writer);
                        (VOID)CloseHandle(mailslot);
                        error = OSA_ENOMEM;
                    }
                }
                else    /* ! (writer != INVALID_HANDLE_VALUE)       failed to create write handle */
                {
                    lastError = GetLastError();
                    (VOID)CloseHandle(mailslot);
                }
            }
            else    /* ! (mailslot != INVALID_HANDLE_VALUE)     failed to create mailslot */
            {
                lastError = GetLastError();
            }
            if (OSA_EOK != OsaMemFree((VOID**)&fullName))
            {
                error = OSA_EMEM;
            }
        }        
        else    /* !  (memError == OSA_EOK)     failed to allocate memory for fullName */
        {
            error = OSA_ENOMEM;
        }
    }
    else    /* ! (0 < nameLength) */
    {
        error = OSA_EPARAM;
    }

    switch (lastError)
    {
        case ERROR_NOT_ENOUGH_MEMORY:
            error = OSA_ENOMEM;
            break;
        case ERROR_ALREADY_EXISTS:
            error = OSA_EPARAM;
            break;
        default:
            /* nothing to do */
            break;
    }

    OSA_HANDLE_GETLASTERR((INT32)lastError);

    return error;
}

/*
 *      Opens a message queue id for writing to an existing queue
 */
OsaErr 
OsaMsgQOpen(CHAR*       name,
            OsaMsgQId*  msgQId)
{
    DWORD       lastError   = 0;
    OsaErr      error       = OSA_ENOK;
    size_t      nameLength  = 0;

    OSA_ASSERT(name   != NULL);
    OSA_ASSERT(msgQId != NULL);

    *msgQId = NULL;
    nameLength = strlen(name);
    if (0 < nameLength)
    {
        /* create the fullname of the mailslot */
        CHAR*       fullName        = NULL;
        size_t      fullNameLength  = sizeof(mailslotNamePrefix) + nameLength;
        OsaErr      memError        = OsaMemAlloc((VOID**)&fullName, fullNameLength + 1);
        if (memError == OSA_EOK)
        {
            /* create a write handle */
            HANDLE  writer  = INVALID_HANDLE_VALUE;
            if (OSA_EOK == OsaStrNCpy(fullName, fullNameLength + 1, mailslotNamePrefix, sizeof(mailslotNamePrefix)))
            {
                if (OSA_EOK == OsaStrNCat(fullName, fullNameLength + 1, name, nameLength))
                {
                    SetLastError(0);
                    writer = CreateFile(fullName,               /* the file/mailslot name */
                                        GENERIC_WRITE,          /* access mode */
                                        FILE_SHARE_READ,        /* share mode */
                                        NULL,                   /* security attributes */
                                        OPEN_EXISTING,          /* action to take */
                                        FILE_ATTRIBUTE_NORMAL,  /* device attributes and flags */
                                        NULL);                  /* template file */
                }
            }
            if (writer != INVALID_HANDLE_VALUE)
            {
                /* create object to be used as OsaMsgQId */
                memError = OsaMemAlloc((VOID**)msgQId, sizeof(**msgQId));
                if (memError == OSA_EOK)
                {
                    (*msgQId)->readHandle   = NULL;
                    (*msgQId)->writeHandle  = writer;
                    error = OSA_EOK;
                }
                else    /* ! (memError == OSA_EOK)      failed to allocate memory for OsaMsgQId */
                {
                    (VOID)CloseHandle(writer);
                    error = OSA_ENOMEM;
                }
            }
            else    /* ! (writer != INVALID_HANDLE_VALUE)       failed to create mailslot */
            {
                lastError = GetLastError();
            }
            if (OSA_EOK != OsaMemFree((VOID**)&fullName))
            {
                error = OSA_EMEM;
            }
        }
        else    /* ! (memError == OSA_EOK)      failed to allocate memory for fullName */
        {
            error = OSA_ENOMEM;
        }
    }
    else    /* ! (0 < nameLength) */
    {
        error = OSA_EPARAM;
    }

    switch (lastError)
    {
        case ERROR_FILE_NOT_FOUND:
            error = OSA_EEXIST;
            break;
        case ERROR_NOT_ENOUGH_MEMORY:
            error = OSA_ENOMEM;

            OSA_HANDLE_GETLASTERR((INT32)lastError);
            break;
        default:
            OSA_HANDLE_GETLASTERR((INT32)lastError);
            break;
    }

    return error;
}

/*
 *      Deletes a message queue
 */
OsaErr
OsaMsgQDelete(OsaMsgQId* msgQId)
{
    OsaErr  error       = OSA_EOK;
    DWORD   lastError   = 0;

    OSA_ASSERT(NULL != msgQId);

    /* A valid read handle indicates that we are the owner */
    if ((*msgQId)->readHandle != NULL)  
    {
        /* a write handle must always be present */
        if ((*msgQId)->writeHandle != NULL)
        {
            SetLastError(0);

            if ( ! CloseHandle((*msgQId)->writeHandle))
            {
                lastError = GetLastError();

                switch (lastError)
                {
                case ERROR_INVALID_HANDLE:
                    error = OSA_EOBJ;
                    break;

                default:
                    error = OSA_ENOK;
                    break;
                }

                OSA_HANDLE_GETLASTERR((INT32)lastError);
            }
        }
        else    /* ! ((*msgQId)->writeHandle != NULL) */
        {
            error = OSA_EOBJ;
        }

        SetLastError(0);

        if ( ! CloseHandle((*msgQId)->readHandle))
        {
            lastError = GetLastError();

            switch (lastError)
            {
            case ERROR_INVALID_HANDLE:
                error = OSA_EOBJ;
                break;
            default:
                error = OSA_ENOK;
                break;
            }

            OSA_HANDLE_GETLASTERR((INT32)lastError);
        }
    }
    else    /* ! ((*msgQId)->readHandle != NULL) */
    {
        error = OSA_ENOOWNER;
    }
    if (OSA_EOK == error)
    {
        /* free the memory allocated for the OsaMsgQStruct */
        if (OSA_EOK != OsaMemFree((VOID**)msgQId))
        {
            error = OSA_EMEM;
        }
    }
    return error;
}

/*
 *      Closes a message queue id acquired with OsaMsgQOpen
 */
OsaErr
OsaMsgQClose(OsaMsgQId* msgQId)
{
    OsaErr  error       = OSA_EOK;
    DWORD   lastError   = 0;

    OSA_ASSERT(NULL != msgQId);

    /* the read handle must be NULL */
    if (NULL == (*msgQId)->readHandle)
    {
        /* a write handle must always be present */
        if (NULL != (*msgQId)->writeHandle)
        {
            SetLastError(0);

            if (!CloseHandle((*msgQId)->writeHandle))
            {
                lastError = GetLastError();

                switch (lastError)
                {
                case ERROR_INVALID_HANDLE:
                    error = OSA_EOBJ;
                    break;

                default:
                    error = OSA_ENOK;
                    break;
                }

               OSA_HANDLE_GETLASTERR((INT32)lastError);
            }
        }
        else    /* ! (NULL != (*msgQId)->writeHandle) */
        {
            error = OSA_EOBJ;
        }
    }
    else    /* ! (NULL == (*msgQId)->readHandle) */
    {
        error = OSA_EOBJ;
    }
    if (OSA_EOK == error)
    {
        /* free the memory allocated for the OsaMsgQStruct */
        if (OSA_EOK != OsaMemFree((VOID**)msgQId))
        {
            error = OSA_EMEM;
        }
    }
    return error;
}

/*
 *      Sends a message
 */
OsaErr
OsaMsgQSend(OsaMsgQId       msgQId,
            VOID*           buffer,
            size_t          size,
            UINT32          timeout)
{
    OsaErr  error           = OSA_ENOK;
    DWORD   bytesWritten    = 0;
    DWORD   lastError   = 0;

    OSA_ASSERT(buffer != NULL);

    UNREFERENCED_PARAMETER(timeout);
    if (0 < size)
    {
        if (NULL != msgQId->writeHandle)
        {
            SetLastError(0);

            if (WriteFile(msgQId->writeHandle, buffer, size, &bytesWritten, NULL))
            {
                if (bytesWritten == size)
                {
                    error = OSA_EOK;
                }
            }
            else    /* ! (WriteFile(...)) */
            {
                lastError = GetLastError();

                switch (lastError)
                {
                case ERROR_INVALID_PARAMETER:
                    error = OSA_EPARAM;
                    break;
                case ERROR_INVALID_HANDLE:
                    error = OSA_EOBJ;
                    break;
                default:
                    /* nothing to do */
                    break;
                }

                OSA_HANDLE_GETLASTERR((INT32)lastError);
            }
        }
        else    /* ! (NULL != msgQId->writeHandle) */
        {
            error = OSA_EOBJ;
        }
    }
    else    /* ! (0 < size) */
    {
        error = OSA_EPARAM;
    }

    return error;
}

/*
 *      Receives a message
 */
OsaErr 
OsaMsgQReceive(OsaMsgQId    msgQId,
               VOID*        buffer,
               size_t       bufferSize,
               UINT32       timeout,
               size_t*      bytesReceived)
{
    OsaErr  error       = OSA_ENOK;
    DWORD   timeoutWin  = 0;
    DWORD   maxMsgSize  = 0;
    DWORD   nextSize    = 0;
    DWORD   msgCount    = 0;
    DWORD   readTimeout = 0;
    DWORD   lastError   = 0;

    OSA_ASSERT(buffer        != NULL);
    OSA_ASSERT(bytesReceived != NULL);

    if (0 < bufferSize)
    {
        if (NULL != msgQId->readHandle)
        {
            /* convert timeout to timeoutWin */
            switch (timeout)
            {
                case OSA_NO_WAIT:
                    timeoutWin = 0;
                    break;
                case OSA_INFINITE:
                    timeoutWin = INFINITE;
                    break;
                default:
                    timeoutWin = timeout;
                    break;
            }

            SetLastError(0);
            /* get the currently set readTimeout */
            if (GetMailslotInfo(msgQId->readHandle, &maxMsgSize, &nextSize, &msgCount, &readTimeout))
            {
                BOOL    result      = TRUE;
                if (timeoutWin != readTimeout)
                {
                    /* set the new timeout for the mailslot, only for reading */
                    SetLastError(0);
                    result = SetMailslotInfo(msgQId->readHandle, timeoutWin);
                    if (!result)
                    {
                        lastError = GetLastError();
                    }
                }

                if (result)
                {
                    VOID*   readBuffer = buffer;
                    if ((nextSize == MAILSLOT_NO_MESSAGE) || (msgCount == 0)) 
                    {
                          /* we do not know what is coming, prepare for max message size */
                          nextSize = maxMsgSize;
                    }
                    /* read wil fail when the next available message is larger than the supplied buffer */
                    /* create a large enough temporary buffer when the supplied buffer is too small */
                    if (nextSize > bufferSize)
                    {
                        result = (OSA_EOK == OsaMemAlloc(&readBuffer, nextSize));
                    }
                    if (result)
                    {
                        SetLastError(0);
                        result = ReadFile(msgQId->readHandle, readBuffer, nextSize, (LPDWORD)bytesReceived, NULL);
                        if (result)
                        {
                            error = OSA_EOK;
                        }
                        else
                        {
                            lastError = GetLastError();
                        }
                        /* copy data from temporary buffer, truncate excessive data */
                        if (nextSize > bufferSize)
                        {
                            if (OSA_EOK == OsaMemCpy(buffer, bufferSize, readBuffer, bufferSize))
                            {
                                /* free temporary buffer */
                                if (OSA_EOK != OsaMemFree(&readBuffer))
                                {
                                    error = OSA_EMEM;
                                }
                                *bytesReceived = bufferSize;
                            }
                        }
                    }
                    else    /* ! (result)       OsaMemAlloc failed */
                    {
                        error = OSA_ENOMEM;
                    }
                }
            }
            else    /* ! (GetMailslotInfo(...))     GetMailslotInfo failed */
            {
                lastError = GetLastError();
            }

            switch (lastError)
            {
                case ERROR_INVALID_HANDLE:
                    error = OSA_EOBJ;
                    OSA_HANDLE_GETLASTERR((INT32)lastError);
                    break;
                case ERROR_SEM_TIMEOUT:
                    error = OSA_ETIME;
                    /* no OSA_HANDLE_GETLASTERR here */
                    break;
                default:
                    OSA_HANDLE_GETLASTERR((INT32)lastError);
                    break;
            }
        }
        else    /* ! (NULL != msgQId->readHandle) */
        {
            error = OSA_EOBJ;
        }
    }
    else    /* ! (0 < bufferSize) */
    {
        error = OSA_EPARAM;
    }

    return error;
}

/*
 *      Checks if there are messages in the queue
 */
OsaErr
OsaMsgQNumMsgs(OsaMsgQId    msgQId,
               UINT32*      numAvailable)
{
    OsaErr  error       = OSA_ENOK;
    DWORD   lastError   = 0;

    OSA_ASSERT(numAvailable != NULL);

    if (NULL != msgQId->readHandle)
    {
        SetLastError(0);
        if (GetMailslotInfo(msgQId->readHandle, NULL, NULL, (LPDWORD)numAvailable, NULL))
        {
            error = OSA_EOK;
        }
        else
        {
            lastError = GetLastError();

            switch (lastError)
            {
            case ERROR_INVALID_HANDLE:
                error = OSA_EOBJ;
                break;

            default:
                break;
            }
            
            OSA_HANDLE_GETLASTERR((INT32)lastError);
        }
    }
    else    /* ! (NULL != msgQId->readHandle) */
    {
        error = OSA_EOBJ;
    }

    return error;
}
