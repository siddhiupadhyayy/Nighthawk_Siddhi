/****************************************************************************/
/*                                                                          */
/*   Filename   : OsaThreadsWin.c                                           */
/*                                                                          */
/*   Description: OS abstraction layer                                      */
/*                OS abstraction for threads and tasks for Windows          */
/*                                                                          */
/*   Copyright  :                                                           */
/*                                                                          */
/****************************************************************************/
#define OSA_INTERNAL

#include <stdio.h>

#include "OsaError.h"
#include "OsaThreads.h"
#include "OsaSync.h"

static HANDLE       GetThreadHandle(OsaThreadId threadId);
static VOID         SetThreadName(OsaThreadId ThreadID, const CHAR* ThreadName);
static int          WinPrio(UINT8 osaPrio);
static UINT8        OsaPrio(int   winPrio);

const OsaThreadId   OsaThreadIdSelf    = 0;
const size_t        MINIMAL_STACK_SIZE = (12 * 1024u); /* 12 kb */


/* Creates a thread to execute within the virtual address space of the calling process. */

OsaErr
OsaThreadCreate(const CHAR *        threadName,
                OsaThreadStartFunc  startFunc,
                VOID *              startParam,
                UINT8               priority,
                size_t              stackSize,
                OsaThreadId *       threadId)
{
    HANDLE hThread;
    OsaErr status = OSA_ENOK;

	/* TODO: implement OSA assert later */
    /* startParam may be NULL ! */

    OSA_ASSERT(threadName != NULL);
    OSA_ASSERT(threadId   != NULL);

    stackSize = max(MINIMAL_STACK_SIZE, stackSize);                            
       
    (VOID)_set_errno(0);

    hThread = CreateThread(NULL,                                /* Security attributes : default                */
                           stackSize,                           /* default stack size                           */
                           (LPTHREAD_START_ROUTINE) startFunc,  /* thread function                              */ 
                           (LPVOID)startParam,                  /* argument to thread                           */ 
                           0,                                   /* The thread runs immediately after creation.  */
                           (LPDWORD)threadId);                  /* returns the thread identifier                */

    status = (hThread == NULL) ? OSA_ENOK : OSA_EOK;

    OSA_HANDLE_ERRNO(OSA_GET_ERR);                                            

    if (status == OSA_EOK)
    {
        status = OsaThreadSetPriority(*threadId, priority);

        if (status == OSA_EOK)
        {
            SetThreadName(*threadId, threadName);

            CloseHandle(hThread);
        }
        else
        {
             OSA_HANDLE_ERRNO(OSA_GET_ERR);   
                       
            (void) OsaThreadDelete(*threadId);

            *threadId = (OsaThreadId) -1;
        }
    }

   return status;
}


/* Tterminates a thread. */

OsaErr
OsaThreadDelete(OsaThreadId threadId)
{
	OsaErr status = OSA_EEXIST;

	HANDLE hThread = GetThreadHandle(threadId);

	if (hThread != NULL)
	{
		BOOL result = FALSE;

		(VOID)_set_errno(0);

		result = TerminateThread(hThread, 0);

        OSA_HANDLE_ERRNO(OSA_GET_ERR);                                        

		status = (result) ? OSA_EOK : OSA_ENOK;

		CloseHandle(hThread);
	}

	return status;
}


/* Sets the priority value for the specified thread. */

OsaErr
OsaThreadSetPriority(OsaThreadId threadId,
                     UINT8 priority)
{
    OsaErr status = OSA_EEXIST;

    HANDLE hThread = GetThreadHandle(threadId);

    if (hThread != NULL)
    {
        BOOL result = FALSE;

        int winprio = WinPrio(priority);

       (VOID)_set_errno(0);

         result = SetThreadPriority(hThread, winprio);

        OSA_HANDLE_ERRNO(OSA_GET_ERR);   

        status = (result) ? OSA_EOK : OSA_ENOK;

        CloseHandle(hThread);
    }

    return status;
}


/* Retrieves the priority value for the specified thread. */

OsaErr
OsaThreadGetPriority(OsaThreadId threadId,
                     UINT8*      osaPrio)
{
    OsaErr status = OSA_EEXIST;

    int WinThreadPrio = THREAD_PRIORITY_LOWEST;

    HANDLE hThread = GetThreadHandle(threadId);

    OSA_ASSERT(osaPrio != NULL);

    if (hThread != NULL)
    {
        (VOID)_set_errno(0);

        WinThreadPrio = GetThreadPriority(hThread);

        OSA_HANDLE_ERRNO(OSA_GET_ERR);            

        status = (WinThreadPrio == THREAD_PRIORITY_ERROR_RETURN) ? OSA_ENOK : OSA_EOK;

        CloseHandle(hThread);
    }
 
    *osaPrio = OsaPrio(WinThreadPrio);
   
    return status;
}


/* Suspends a thread. */

OsaErr 
OsaThreadSuspend(OsaThreadId threadId)
{
    OsaErr status = OSA_EEXIST;

    HANDLE hThread = GetThreadHandle(threadId);
    
    if (hThread != NULL)
    {
        DWORD count = 0;

        (VOID)_set_errno(0);

        count = SuspendThread(hThread);

        OSA_HANDLE_ERRNO(OSA_GET_ERR);  
        status = (count != (DWORD) -1) ? OSA_EOK : OSA_ENOK;

        CloseHandle(hThread);
    }

    return status;
}


/* Resumes a thread. */

OsaErr 
OsaThreadResume(OsaThreadId threadId)
{
    OsaErr status = OSA_EEXIST;

    HANDLE hThread = GetThreadHandle(threadId);

    if (hThread != NULL)
    {
        DWORD count;

        (VOID)_set_errno(0);

        do
        {
            count = ResumeThread(hThread);
        }
        while ((count > 1) && (count != (DWORD) -1));   

        OSA_HANDLE_ERRNO(OSA_GET_ERR);      

        status = (count != (DWORD) -1) ? OSA_EOK : OSA_ENOK;                    
       
        CloseHandle(hThread);
    }    

    return status;
}


/* Suspends the execution of the current thread until the time-out interval elapses. */

OsaErr
OsaThreadSleep(UINT32 interval)
{
    Sleep(interval);

    return OSA_EOK;
}


/* Get the thread id of the current thread. */

OsaErr
OsaThreadGetCurrent(OsaThreadId* threadId)
{
    OsaErr error = OSA_EOK;

    OSA_ASSERT(NULL != threadId);

    *threadId = GetCurrentThreadId();

    return error;
}


/* Verify if the thread id is a valid thread id. */

OsaErr
OsaThreadVerify(OsaThreadId threadId, BOOL* isValid)
{
    OsaErr error = OSA_EOK;
    HANDLE hThread = NULL;

    OSA_ASSERT(NULL != isValid);

    /* always valid for OsaThreadIdSelf */

    if (OsaThreadIdSelf == threadId)
    {
        *isValid = TRUE;
    }
    else
    {
        hThread = OpenThread((DWORD)THREAD_ALL_ACCESS, FALSE, threadId);

        *isValid = (hThread != NULL);

        if (*isValid)
        {
            CloseHandle(hThread);
        }
    }

    return error;
}


/* Returns the thread handle corresponding to the threadId, or NULL in case of error. */

static HANDLE
GetThreadHandle(OsaThreadId threadId)
{
    HANDLE hThread;

    if (threadId == OsaThreadIdSelf)
    {
        threadId = GetCurrentThreadId();
    }

    hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, threadId);       

    return hThread;
}

/* Waits the timeout period for the thread to terminate. */
OsaErr
OsaThreadWaitForTermination(OsaThreadId threadId,
                            UINT32      timeOut)
{
    OsaErr status = OSA_EEXIST;

    HANDLE hThread = GetThreadHandle(threadId);

    if (hThread != NULL)
    {
        status = OsaSyncWaitOne(hThread, timeOut);

        CloseHandle(hThread);
    }

    return status;
}

/* Code from https:// msdn.microsoft.com/en-us/library/xcb2z8hs.aspx */
const UINT32 MS_VC_EXCEPTION = 0x406D1388;

#pragma pack(push,8)

typedef struct tagTHREADNAME_INFO
{
    DWORD dwType;       /* must be 0x1000                        */
    LPCSTR szName;      /* pointer to name (in user addr space)  */
    DWORD dwThreadID;   /* thread ID (-1=caller thread)          */
    DWORD dwFlags;      /* reserved for future use, must be zero */
} THREADNAME_INFO;

#pragma pack(pop)

///*

/* Sets the name of the thread */
static VOID
SetThreadName(OsaThreadId ThreadID, const CHAR* ThreadName)
{
    THREADNAME_INFO info;

    OSA_ASSERT(ThreadName != NULL);

    info.dwType = 0x1000;
    info.szName = ThreadName;
    info.dwThreadID = ThreadID;
    info.dwFlags = 0;

    __try
    {
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(DWORD), (DWORD*)&info );
    }
    __except(EXCEPTION_CONTINUE_EXECUTION)
    {
    }
}


//==========================================================================
//Process priority class         Thread priority level         Base priority
//==========================================================================
//                            THREAD_PRIORITY_IDLE                    1
//                            THREAD_PRIORITY_LOWEST                  2
//                            THREAD_PRIORITY_BELOW_NORMAL            3
//IDLE_PRIORITY_CLASS         THREAD_PRIORITY_NORMAL                  4   
//                            THREAD_PRIORITY_ABOVE_NORMAL            5
//                            THREAD_PRIORITY_HIGHEST                 6
//                            THREAD_PRIORITY_TIME_CRITICAL          15
//
//                            THREAD_PRIORITY_IDLE                    1
//                            THREAD_PRIORITY_LOWEST                  4
//                            THREAD_PRIORITY_BELOW_NORMAL            5
//BELOW_NORMAL_PRIORITY_CLASS THREAD_PRIORITY_NORMAL                  6
//                            THREAD_PRIORITY_ABOVE_NORMAL            7
//                            THREAD_PRIORITY_HIGHEST                 8
//                            THREAD_PRIORITY_TIME_CRITICAL          15
//
//                            THREAD_PRIORITY_IDLE                    1
//                            THREAD_PRIORITY_LOWEST                  6
//                            THREAD_PRIORITY_BELOW_NORMAL            7
//NORMAL_PRIORITY_CLASS       THREAD_PRIORITY_NORMAL                  8
//                            THREAD_PRIORITY_ABOVE_NORMAL            9
//                            THREAD_PRIORITY_HIGHEST                10
//                            THREAD_PRIORITY_TIME_CRITICAL          15
//
//                            THREAD_PRIORITY_IDLE                    1
//                            THREAD_PRIORITY_LOWEST                  8
//                            THREAD_PRIORITY_BELOW_NORMAL            9
//ABOVE_NORMAL_PRIORITY_CLASS THREAD_PRIORITY_NORMAL                 10
//                            THREAD_PRIORITY_ABOVE_NORMAL           11
//                            THREAD_PRIORITY_HIGHEST                12
//                            THREAD_PRIORITY_TIME_CRITICAL          15
//
//                            THREAD_PRIORITY_IDLE                    1
//                            THREAD_PRIORITY_LOWEST                 11
//                            THREAD_PRIORITY_BELOW_NORMAL           12
//HIGH_PRIORITY_CLASS         THREAD_PRIORITY_NORMAL                 13
//                            THREAD_PRIORITY_ABOVE_NORMAL           14
//                            THREAD_PRIORITY_HIGHEST                15
//                            THREAD_PRIORITY_TIME_CRITICAL          15
//
//                            THREAD_PRIORITY_IDLE                   16
//                            THREAD_PRIORITY_LOWEST                 22
//                            THREAD_PRIORITY_BELOW_NORMAL           23
//REALTIME_PRIORITY_CLASS     THREAD_PRIORITY_NORMAL                 24
//                            THREAD_PRIORITY_ABOVE_NORMAL           25
//                            THREAD_PRIORITY_HIGHEST                26
//                            THREAD_PRIORITY_TIME_CRITICAL          31
//

static const UINT8 OsaPrioThresholds [] = { 40, 80, 112, 144, 176, 216 };

static int
WinPrio(UINT8 osaPrio)
{
    INT8 winprio;

    if (osaPrio <= OsaPrioThresholds[0])
    {
        winprio = THREAD_PRIORITY_TIME_CRITICAL;
    }
    else if (osaPrio <= OsaPrioThresholds[1])
    {
        winprio = THREAD_PRIORITY_HIGHEST;
    }
    else if (osaPrio <= OsaPrioThresholds[2])
    {
        winprio = THREAD_PRIORITY_ABOVE_NORMAL;
    }
    else if (osaPrio <= OsaPrioThresholds[3])
    {
        winprio = THREAD_PRIORITY_NORMAL;
    }
    else if (osaPrio <= OsaPrioThresholds[4])
    {
        winprio = THREAD_PRIORITY_BELOW_NORMAL;
    }
    else if (osaPrio <= OsaPrioThresholds[5])
    {
        winprio = THREAD_PRIORITY_LOWEST;
    }
    else
    {
        winprio = THREAD_PRIORITY_IDLE;
    }

    return winprio;
}


/* Round the window priority to the closest Osa priority. */

static UINT8
OsaPrio(int winPrio)
{
    UINT8 osaPrio;

    switch (winPrio)
    {
    case THREAD_PRIORITY_TIME_CRITICAL:     osaPrio = OsaPrioThresholds[0];
                                            break;
    case THREAD_PRIORITY_HIGHEST:           osaPrio = OsaPrioThresholds[1];
                                            break;
    case THREAD_PRIORITY_ABOVE_NORMAL:      osaPrio = OsaPrioThresholds[2];
                                            break;
    case THREAD_PRIORITY_NORMAL:            osaPrio = OsaPrioThresholds[3];
                                            break;
    case THREAD_PRIORITY_BELOW_NORMAL:      osaPrio = OsaPrioThresholds[4];
                                            break;
    case THREAD_PRIORITY_LOWEST:            osaPrio = OsaPrioThresholds[5];
                                            break;
    case THREAD_PRIORITY_IDLE:              osaPrio = UINT8_MAX;
                                            break;
    default:                                osaPrio = UINT8_MAX;
                                            break;
   }

   return osaPrio;
}

