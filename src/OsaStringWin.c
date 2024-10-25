/*****************************************************************************/
/*                                                                           */
/*   Filename   : OsaStringWin.c                                             */
/*                                                                           */
/*   Description: OS abstraction layer                                       */
/*                OS abstraction for strings in Windows                      */
/*                                                                           */
/*   Copyright  :                                                            */
/*                                                                           */
/*****************************************************************************/

#define OSA_INTERNAL

#include <windows.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include "OsaError.h"
#include "OsaString.h"

/*
 *      Append a string.
 */
OsaErr
OsaStrCat(CHAR*       dst,
          size_t      dstSize,
          const CHAR* src)
{
    OsaErr                     errorOsa = OSA_EOK;
    errno_t                    errorWin = ERROR_SUCCESS;

    OSA_ASSERT( NULL != src );

    (VOID)_set_errno(0);

    errorWin = strcat_s(dst, dstSize, src);

    switch( errorWin )
    {
        case ERROR_SUCCESS :
            /* OSA_EOK already set.*/
            break;
        case EINVAL :
            errorOsa = OSA_EPARAM;
            break;
        case ERANGE :
            errorOsa = OSA_ERANGE;
            break;
        default :
            errorOsa = OSA_ENOK;
            break;
    }

    OSA_HANDLE_ERRNO(errorWin);

    return errorOsa;
}

/*
 *      Append characters to a string.
 */
OsaErr
OsaStrNCat(CHAR*       dst,
           size_t      dstSize,
           const CHAR* src,
           size_t      count)
{
    OsaErr                     errorOsa = OSA_EOK;
    size_t                     countWin = 0;
    errno_t                    errorWin = ERROR_SUCCESS; 

    OSA_ASSERT( NULL != src );

    countWin = (OSA_TRUNCATE == count) ? _TRUNCATE : count;   

    (VOID)_set_errno(0);

    errorWin = strncat_s(dst, dstSize, src, countWin);

    switch( errorWin )
    {
        case ERROR_SUCCESS :
            /* OSA_EOK already set.*/
            break;
        case STRUNCATE :
            /* Do nothing! 
            Caller requested truncation if required
            As this truncation is windows specific we do not add
            an 'error' state. */
            break;
        case EINVAL :
            errorOsa = OSA_EPARAM;
            break;
        case ERANGE :
            errorOsa = OSA_ERANGE;
            break;
        default :
            errorOsa = OSA_ENOK;
            break;
    }

    OSA_HANDLE_ERRNO(errorWin);

    return errorOsa;
}

/*
 *      Append a wide-character string.
 */
OsaErr
OsaWcsCat(WCHAR*       dst,
          size_t       dstSize,
          const WCHAR* src)
{
    OsaErr                     errorOsa = OSA_EOK;
    errno_t                    errorWin = ERROR_SUCCESS;

    OSA_ASSERT( NULL != src );

    (VOID)_set_errno(0);

    errorWin = wcscat_s(dst, dstSize, src);

    switch( errorWin )
    {
        case ERROR_SUCCESS :
            /* OSA_EOK already set.*/
            break;
        case EINVAL :
            errorOsa = OSA_EPARAM;
            break;
        case ERANGE :
            errorOsa = OSA_ERANGE;
            break;
        default :
            errorOsa = OSA_ENOK;
            break;
    }

    OSA_HANDLE_ERRNO(errorWin);

    return errorOsa;
}

/*
 *      Append wide-characters to a wide-character string.
 */
OsaErr
OsaWcsNCat(WCHAR*       dst,
           size_t       dstSize,
           const WCHAR* src,
           size_t       count)
{
    OsaErr                     errorOsa = OSA_EOK;
    size_t                     countWin = 0;
    errno_t                    errorWin = ERROR_SUCCESS;

    OSA_ASSERT( NULL != src );

    countWin = (OSA_TRUNCATE == count) ? _TRUNCATE : count;    

    (VOID)_set_errno(0);

    errorWin = wcsncat_s(dst, dstSize, src, countWin);

    switch( errorWin )
    {
        case ERROR_SUCCESS :
            /* OSA-EOK already set.*/
            break;
        case STRUNCATE :
            /* Do nothing! 
            Caller requested truncation if required
            As this truncation is windows specific we do not add
            an 'error' state. */
            break;
        case EINVAL :
            errorOsa = OSA_EPARAM;
            break;
        case ERANGE :
            errorOsa = OSA_ERANGE;
            break;
        default :
            errorOsa = OSA_ENOK;
            break;
    }

   OSA_HANDLE_ERRNO(errorWin);

    return errorOsa;
}

/*
 *      Copy a string.
 */
OsaErr
OsaStrCpy(CHAR*       dst,
          size_t      dstSize,
          const CHAR* src)
{
    OsaErr errorOsa  = OSA_EOK;
    errno_t errorWin = 0;

    (VOID)_set_errno(0);

    errorWin = strcpy_s(dst, dstSize, src);

    switch( errorWin )
    {
        case ERROR_SUCCESS :
            /* OSA-EOK already set.*/
            break;
        case EINVAL :
            errorOsa = OSA_EPARAM;
            break;
        case ERANGE :
            errorOsa = OSA_ERANGE;
            break;
        default :
            errorOsa = OSA_ENOK;
            break;
    }

    OSA_HANDLE_ERRNO(errorWin);

    return errorOsa;
}

/*
 *      Copy characters from one string to another.
 */
OsaErr
OsaStrNCpy(CHAR*       dst,
           size_t      dstSize,
           const CHAR* src,
           size_t      count)
{
    OsaErr errorOsa  = OSA_EOK;
    errno_t errorWin = 0;

    size_t countWin = (OSA_TRUNCATE == count) ? _TRUNCATE : count;  

    (VOID)_set_errno(0);

    errorWin = strncpy_s(dst, dstSize, src, countWin);

    switch( errorWin )
    {
        case ERROR_SUCCESS :
            /* OSA-EOK already set.*/
            break;
        case STRUNCATE :
            /* Do nothing! 
            Caller requested truncation if required
            As this truncation is windows specific we do not add
            an 'error' state. */
            break;
        case EINVAL :
            errorOsa = OSA_EPARAM;
            break;
        case ERANGE :
            errorOsa = OSA_ERANGE;
            break;
        default :
            errorOsa = OSA_ENOK;
            break;
    }

    OSA_HANDLE_ERRNO(errorWin);

    return errorOsa;
}

/*
 *      Copy a wide-character string.
 */
OsaErr
OsaWcsCpy(WCHAR*       dst,
          size_t       dstSize,
          const WCHAR* src)
{
    OsaErr errorOsa  = OSA_EOK;
    errno_t errorWin = 0;

    (VOID)_set_errno(0);

    errorWin = wcscpy_s(dst, dstSize, src);

    switch( errorWin )
    {
        case ERROR_SUCCESS :
            /* OSA-EOK already set.*/
            break;
        case EINVAL :
            errorOsa = OSA_EPARAM;
            break;
        case ERANGE :
            errorOsa = OSA_ERANGE;
            break;
        default :
            errorOsa = OSA_ENOK;
            break;
    }
   
    OSA_HANDLE_ERRNO(errorWin);

    return errorOsa;
}

/*
 *      Copy wide-characters from one string to another.
 */
OsaErr
OsaWcsNCpy(WCHAR*       dst,
           size_t       dstSize,
           const WCHAR* src,
           size_t       count)
{
    OsaErr errorOsa  = OSA_EOK;
    errno_t errorWin = 0;

    size_t countWin = (OSA_TRUNCATE == count) ? _TRUNCATE : count; 

    (VOID)_set_errno(0);

    errorWin = wcsncpy_s(dst, dstSize, src, countWin);

    switch( errorWin )
    {
        case ERROR_SUCCESS :
            /* OSA-EOK already set.*/
            break;
        case STRUNCATE :
            /* Do nothing! 
            Caller requested truncation if required
            As this truncation is windows specific we do not add
            an 'error' state. */
            break;
        case EINVAL :
            errorOsa = OSA_EPARAM;
            break;
        case ERANGE :
            errorOsa = OSA_ERANGE;
            break;
        default :
            errorOsa = OSA_ENOK;
            break;
    }

   OSA_HANDLE_ERRNO(errorWin);

    return errorOsa;
}

/*
 *      Find the next token in a string, re-entrant.
 */
OsaErr
OsaStrTok(CHAR*       token,
          const CHAR* delimiter,
          CHAR**      context,
          CHAR**      nextToken)
{
    OsaErr errorOsa = OSA_EOK;
    int    errorWin = ERROR_SUCCESS;

    //errno is set by strtok_s, so _set_errno and _get_errno are used.
    (VOID)_set_errno(errorWin);
    *nextToken = strtok_s(token, delimiter, context);
    (VOID)_get_errno(&errorWin);

    switch( errorWin )
    {
        case ERROR_SUCCESS :
            /* OSA-EOK already set.*/
            break;
        case EINVAL :
            errorOsa = OSA_EPARAM;
            break;
        default :
            errorOsa = OSA_ENOK;
            break;
    }

    OSA_HANDLE_ERRNO(errorWin);

    return errorOsa;
}

/*
 *      Find the next token in a wide-character string, re-entrant.
 */
OsaErr
OsaWcsTok(WCHAR*       token,
          const WCHAR* delimiter,
          WCHAR**      context,
          WCHAR**      nextToken)
{
    OsaErr errorOsa = OSA_EOK;
    int    errorWin = ERROR_SUCCESS;

    //errno is set by wcstok_s, so _set_errno and _get_errno are used.
    (VOID)_set_errno(errorWin);
    *nextToken = wcstok_s(token, delimiter, context);
    (VOID)_get_errno(&errorWin);

    switch( errorWin )
    {
        case ERROR_SUCCESS :
            /* OSA-EOK already set.*/
            break;
        case EINVAL :
            errorOsa = OSA_EPARAM;
            break;
        default :
            errorOsa = OSA_ENOK;
            break;
    }

    OSA_HANDLE_ERRNO(errorWin);

    return errorOsa;
}
