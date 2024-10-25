/****************************************************************************/
/*                                                                          */
/*   Filename   : OsaError.c                                                */
/*                                                                          */
/*   Description: OS abstraction layer                                      */
/*                OS abstraction error implementation for all OSes          */
/*                                                                          */
/*   Copyright  :                                                           */
/*                                                                          */
/****************************************************************************/

#define OSA_INTERNAL

#include "OsaError.h"

/* the content of this array must match with the enum OsaErr */
static const CHAR* OsaErrStrings[] = {
    "OSA_ENOK",
    "OSA_EOK",
    "OSA_EOBJ",
    "OSA_EPARAM",
    "OSA_EEXIST",
    "OSA_ETIME",
    "OSA_ENOOWNER",
    "OSA_ENOMEM",
    "OSA_EMEM",
    "OSA_ERANGE",
    "OSA_EEOF",
    "OSA_EOFLOW"
};

static const CHAR* unknown = "unknown";

/*
 *      Get the string for an OsaErr
 */
const CHAR*
OsaErrStr(OsaErr error)
{
    const CHAR* result = unknown;
    INT32 index = (INT32)error - (INT32)OSA_ENOK;
    if ((0 <= index) && ((INT32)(ARRAY_COUNT(OsaErrStrings)) > index))
    {
        result = OsaErrStrings[index];
    }
    return result;
}
