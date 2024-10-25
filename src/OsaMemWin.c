/****************************************************************************/
/*                                                                          */
/*   Filename   : OsaMemWin.c                                               */
/*                                                                          */
/*   Description: OS abstraction layer                                      */
/*                OS abstraction for memory allocation for Windows          */
/*                                                                          */
/*   Copyright  :                                                           */
/*                                                                          */
/****************************************************************************/

#define OSA_INTERNAL

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "OsaError.h"
#include "OsaMem.h"

/*
 *      Allocates n bytes of memory
 */
OsaErr
OsaMemAlloc(VOID**    mem,
            size_t    size)
{
    INT32 err = 0;
	OSA_ASSERT(mem != NULL);

    (VOID)_set_errno(err);

    *mem = malloc(size);          

    (VOID)_get_errno(&err);

	OSA_HANDLE_ERRNO(err);

    if ((0 != err) && (NULL != *mem))
    {
        free(*mem);
        *mem = NULL;
    }

    return (NULL != *mem) ? OSA_EOK : OSA_ENOMEM;
}

/*
 *      Reallocates n bytes of memory
 */
OsaErr
OsaMemRealloc(VOID**    mem,
              size_t    size)
{
    OsaErr       returnValue = OSA_EOK;
    INT32        err         = 0;
    const void*  oldPointer  = *mem;

	OSA_ASSERT(mem != NULL);

    (VOID)_set_errno(err);

    *mem = realloc(*mem, size);      

    (VOID)_get_errno(&err);

	OSA_HANDLE_ERRNO(err);

    if (0 != err)
    {
        returnValue = OSA_ENOMEM;
    }

    /* realloc behavior (no failure)                        */
    /* old P    | size  | new P behavior                    */
    /*------------------------------------------------------*/
    /*  NULL    |    0  | !NULL not valid for use           */
    /*  NULL    |   >0  | !NULL like malloc                 */
    /* !NULL    |    0  |  NULL like free                   */
    /* !NULL    |   >0  | !NULL resize                      */

    if ((NULL == oldPointer) && (NULL != *mem) && (0 != err)) /* Error when behave like malloc */
    {
        free(*mem);
        *mem = NULL;
        returnValue = OSA_ENOMEM;
    }

    if ((NULL != oldPointer) && (0 == size) && (NULL != *mem)) /* Check behave like free */
    {
        free(*mem);
        *mem = NULL;
        returnValue = OSA_ENOMEM;
    }

    if ((NULL != oldPointer) && (0 < size) && (NULL == *mem)) /* Check resize */
    {
        returnValue = OSA_ENOMEM;
    }

    return returnValue;
}

/*
 *      The OsaMemCalloc allocates an array in memory with elements initialized to 0.
 */
OsaErr
OsaMemCalloc(VOID**    mem,
             size_t    count,
             size_t    size)
{
    INT32 err = 0;

	OSA_ASSERT(mem != NULL);
    
    (VOID)_set_errno(err);

    *mem = calloc(count, size);      

    (VOID)_get_errno(&err);

	OSA_HANDLE_ERRNO(err);

    if ((0 != err) && (NULL != *mem))
    {
        free(*mem);
        *mem = NULL;
    }

    return (NULL != *mem) ? OSA_EOK : OSA_ENOMEM;
}

/*
 *      Frees the allocated block of memory previously allocated
 *      by OsaMemAlloc, OsaMemRealloc, or OsaMemCalloc.
 */
OsaErr
OsaMemFree(VOID**    mem)
{
    INT32 err = 0;
    OsaErr result;

	OSA_ASSERT( mem != NULL);
    OSA_ASSERT(*mem != NULL);
    
    (VOID)_set_errno(err);

    free(*mem);
    
    (VOID)_get_errno(&err);
    
	OSA_HANDLE_ERRNO(err);

    result = (err == 0) ? OSA_EOK : OSA_ENOK;

    *mem = NULL;

    return result;
}

/*
 *      Copies a block of memory
 */
OsaErr
OsaMemCpy(VOID*         dst,
          size_t        dstSize,
          const VOID*   src,
          size_t        count)
{
    OsaErr  error                           = OSA_EOK;
    errno_t errorWin                        = ERROR_SUCCESS;

	OSA_ASSERT(NULL != dst);
    OSA_ASSERT(NULL != src);

    (VOID)_set_errno(0);

    errorWin = memcpy_s(dst, dstSize, src, count);
    
	OSA_HANDLE_ERRNO(errorWin);

    if (ERROR_SUCCESS != errorWin)
    {
        switch (errorWin)
        {
            case EINVAL:
                error = OSA_EPARAM;
                break;
            case ERANGE:
                error = OSA_ERANGE;
                break;
            default:
                error = OSA_ENOK;
                break;
        }
    }
    return error;
}

/*
 *      Moves a block of memory
 */
OsaErr
OsaMemMove(VOID*        dst,
           size_t       dstSize,
           const VOID*  src,
           size_t       count)
{
    OsaErr  error       = OSA_EOK;
    errno_t errorWin    = ERROR_SUCCESS;

	OSA_ASSERT(NULL != dst);
    OSA_ASSERT(NULL != src);

    (VOID)_set_errno(0);

    errorWin = memmove_s(dst, dstSize, src, count);
    
	OSA_HANDLE_ERRNO(errorWin);
    
    if (ERROR_SUCCESS != errorWin)
    {
        switch (errorWin)
        {
            case EINVAL:
                error = OSA_EPARAM;
                break;
            case ERANGE:
                error = OSA_ERANGE;
                break;
            default:
                error = OSA_ENOK;
                break;
        }
    }
    return error;
}
