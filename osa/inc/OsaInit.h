/****************************************************************************/
/*                                                                          */
/*   Filename   : OsaInit.h                                                 */
/*                                                                          */
/*   Description: OS abstraction layer interface                            */
/*                The library initialization                                */
/*                                                                          */
/*   Copyright  :                                                           */
/*                                                                          */
/****************************************************************************/

#ifndef OSAINIT_H
#define OSAINIT_H    

#include "OsaGenTypes.h"

/* Callback prototype for error handling callback */
typedef void (*OsaErrorCallbackFunc)(const CHAR* string);

/* Callback prototype for invalid parameter callback */
typedef void (*OsaInvalidParCallbackFunc)(const CHAR* string);

/* Osa library initialization data structure */
typedef struct
{
    OsaErrorCallbackFunc            ErrorCallback;
    OsaInvalidParCallbackFunc       InvalidParCallback;
    BOOL                            ExitOnErr;
    BOOL                            InitSocketLib;
} OsaInitLib;

/* Initializes the Osa library using the OsaInitLib struct. */
OSADLL OsaErr
OsaInit(const OsaInitLib* init);

/* Finalizes the Osa library. */
OSADLL OsaErr
OsaExit(void);

#ifdef OSA_INTERNAL

/* Retrieves initialization status */
/*  Returns TRUE if Osa library is initialized */
BOOL
OsaInitIsInitialized(void);

/* Retrieves OsaErrorCallbackFunc */
/* Returns OsaErrorCallbackFunc pointer */
OsaErrorCallbackFunc
OsaInitGetOsaErrorCallbackFunc(void);

#ifdef WIN32
/* Retrieves OsaInvalidParCallbackFunc */
/* Returns OsaInvalidParCallbackFunc pointer */
OsaInvalidParCallbackFunc
OsaInitGetInvalidParCallback(void);
#endif

#ifdef WIN32
/* Retrieves ExitOnErr status */
/* Returns ExitOnErr */
BOOL
OsaInitExitOnErr(void);
#endif  /* WIN32 */

#endif  /* OSA_INTERNAL */

#endif  /* OSAINIT_H */
