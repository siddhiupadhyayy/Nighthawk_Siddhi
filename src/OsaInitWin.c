/****************************************************************************/
/*                                                                          */
/*   Filename   : OsaInitWin.c                                              */
/*                                                                          */
/*   Description: OS abstraction layer                                      */
/*                OS abstraction initialization implementation Windows      */
/*                                                                          */
/*   Copyright  :                                                           */
/*                                                                          */
/****************************************************************************/

#define OSA_INTERNAL

#include <stdio.h>
#include <Windows.h>
#include "OsaGenTypes.h"
#include "OsaInit.h"
#include "OsaError.h"
#include "OsaFile.h"
#include "OsaSocket.h"

/* Library initialization status */
static BOOL LocalInitParamsInitialized = FALSE;

/* The initialization data */
static OsaInitLib LocalInitParams = { NULL, NULL, FALSE, FALSE };

/*  Function pointer to the system invalid paramater handler */
static _invalid_parameter_handler  OldInvalidParameterHandler = NULL;

static const UINT SystemTimeResolution = 1u;

/* Initializes the Osa library using the OsaInitLib struct. */
OsaErr
OsaInit(const OsaInitLib* init)
{
    OsaErr status = OSA_EOK;
    double dummy = 1.0;           /*  Needed to initialize floating point module. */
                                  /*  Bug in Visual Studio 2010??? */
    UNREFERENCED_PARAMETER(dummy);                              
    
    OSA_ASSERT(init != NULL);

    /* initialize 'constant' file streams */
    OsaFileStdIn.File  = stdin;
    OsaFileStdOut.File = stdout;
    OsaFileStdErr.File = stderr;

    LocalInitParams = *init;

    if (LocalInitParams.InvalidParCallback != NULL)
    {
        OldInvalidParameterHandler = _set_invalid_parameter_handler(OsaInvalidParameterHandler);
    }

    if (LocalInitParams.InitSocketLib)
    {
        status = OsaSocketStartup();
    }

    LocalInitParamsInitialized = TRUE;

    /* set the interval for timeGetTime() */
    if (OSA_EOK == status)
    {
        if (TIMERR_NOERROR != timeBeginPeriod(SystemTimeResolution))
        {
            status = OSA_ENOK;
        }
    }

    return status;
}

/* Finalizes the Osa library. */
OsaErr
OsaExit(void)
{
    OsaErr status= OSA_EOK;

    if (LocalInitParamsInitialized)
    {
        if (LocalInitParams.InvalidParCallback != NULL)
        {
            _set_invalid_parameter_handler(OsaInvalidParameterHandler);

            OldInvalidParameterHandler = NULL;
        }

        if (LocalInitParams.InitSocketLib)
        {
            status = OsaSocketCleanup();
        }
    }

    /* unset the interval for timeGetTime() */
    (VOID)timeEndPeriod(SystemTimeResolution);

    return status;
}

/* Getter for LocalInitParamsInitialized */
BOOL
OsaInitIsInitialized(void)
{
    return LocalInitParamsInitialized;
}

/* Getter for ErrorCallback */
OsaErrorCallbackFunc
OsaInitGetOsaErrorCallbackFunc(void)
{
    OSA_ASSERT(LocalInitParamsInitialized);

    return LocalInitParams.ErrorCallback;
}

/* Getter for InvalidParCallback */
OsaInvalidParCallbackFunc
OsaInitGetInvalidParCallback(void)
{
    OSA_ASSERT(LocalInitParamsInitialized);

    return LocalInitParams.InvalidParCallback;
}

/* Getter for exitOnErr */
BOOL
OsaInitExitOnErr(void)
{
    OSA_ASSERT(LocalInitParamsInitialized);

    return LocalInitParams.ExitOnErr;
}