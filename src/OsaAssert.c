/******************************************************************************/
/*                                                                            */
/*   Filename   : OsaAssert.c                                                 */
/*                                                                            */
/*   Description: OS abstraction layer                                        */
/*                OS abstraction assert implementation for VxWorks and        */
/*                Windows                                                     */
/*                                                                            */
/*   Copyright  :                                                             */
/*                                                                            */
/******************************************************************************/

#define OSA_INTERNAL

#include "OsaGenTypes.h"
#include "OsaAssert.h"

#ifdef WIN32

#include <stdio.h>

VOID
OsaAssert(BOOL test, CHAR* cond, CHAR* file, UINT32 line)
{
    if ( ! test)
    {
        CHAR  msg[256];
        DWORD tId = GetCurrentThreadId();

        sprintf_s(msg,
                  sizeof(msg),
                  "Assertion failed: %s; task ID: 0x%x; file: %s, line: %d.\a\n",
                  cond,
                  tId,
                  file,
                  line);

        if (IsDebuggerPresent())
        {
            OutputDebugString(msg);
        }
        else
        {
            fputs(msg, stderr);
        }
        exit(1);
    }
}

#else   /* WIN32 */

#if (defined VXWORKS5) || (defined VXWORKS5_SIM)
    #include <stdioLib.h>
#endif
#if (defined VXWORKS7) || (defined VXWORKS7_SIM)
    #include <fioLib.h>
#endif
#include <taskLib.h>

VOID
OsaAssert(BOOL test, CHAR* cond, CHAR* file, UINT32 line)
{
    if ( ! test)
    {
        INT32 tId = taskIdSelf();
        CHAR* name = taskName(tId);

        fdprintf(2,
                 "Assertion failed: %s ; task ID: 0x%x (%s)\n  file: %s, line: %d.  Task suspended\a\n",
                 cond,
                 tId,
                 name,
                 file,
                 line);

        taskSuspend(tId);
    }
}

#endif  /* WIN32 */
