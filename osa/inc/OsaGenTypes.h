/****************************************************************************/
/*                                                                          */
/*   Filename   : OsaGenTypes.h                                             */
/*                                                                          */
/*   Description: OS abstraction layer interface                            */
/*                 OS independent type definitions                          */
/*                                                                          */
/*   Copyright  :                                                           */
/*                                                                          */
/****************************************************************************/

#ifndef OSAGENTYPES_H
#define OSAGENTYPES_H
#include "OsaDef.h"

/*  OS independent type definitions
 *
 *  Details: This file contains the definitions that are common for
 *           OS(s). Identical definitions that are
 *           defined differently between the OS(s) are in the specific
 *           OsaGenTypesOSXXX.h headers.
 *           Definitions that are different can be found in OsaVxw.h and
 *           OsaWin.h
 */

#define OSAGENTYPESOS
#if defined WIN32
#include "OsaGenTypesWin.h"
// TODO: #elif --- use for other OS
#endif
#undef OSAGENTYPESOS

 /* define types for use outside OSA */
#ifndef OSA_INTERNAL
	/* define BOOL values */
#ifdef FALSE
#undef FALSE
#endif /* FALSE */
#define FALSE (0)

#ifdef TRUE
#undef TRUE
#endif  /* TRUE */
#define TRUE (1)

/* define NULL (copied from several headers) */
#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

#endif /* !OSA_INTERNAL */

/* macro to determine the number of elements in an array */
/* Determine the number of elements in an array */
#define ARRAY_COUNT(a)  (sizeof(a) / sizeof(a[0]))

#if defined WIN32
#include "OsaWin.h"
// TODO: #elif --- use for other OS
#else
#error ERROR UNKNOWN OS!
#endif

#include "OsaAssert.h"

/* Error codes the OS abstraction Layer can return      */
/* when adding to this enum, remember to update         */
/* OsaErrStrings in OsaError.c                          */

typedef enum
{
    /*!     There is an error (~OSA_EOK)    */
    OSA_ENOK    = -1,
    /*!     All OK                          */
    OSA_EOK,
    /*!     Object could not be created or invalid object */
    OSA_EOBJ,
    /*!     Parameter error                 */  
    OSA_EPARAM,
    /*!     Unknown object                  */  
    OSA_EEXIST,
    /*!     Timeout                         */
    OSA_ETIME,
    /*!     Not the object owner            */
    OSA_ENOOWNER,
    /*!     Memory allocation error         */
    OSA_ENOMEM,
    /*!     Memory failure (access or free) */
    OSA_EMEM,
    /*!     Parameter range error           */
    OSA_ERANGE,
    /*!     End of file error               */
    OSA_EEOF,
    /*!     Buffer overflow                 */
    OSA_EOFLOW
    /* when adding to this enum, remember to update     */
    /* OsaErrStrings in OsaError.c                      */
} OsaErr;

#endif  /* OSAGENTYPES_H */
