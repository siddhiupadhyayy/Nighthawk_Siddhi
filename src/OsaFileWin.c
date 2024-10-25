/******************************************************************************/
/*                                                                            */
/* Filename OsaFileWin.c                                                      */
/*                                                                            */
/* Description: OS abstraction layer                                          */
/*              OS abstraction for file handling for Windows                  */
/*                                                                            */
/* Copyright :                                                                */
/*                                                                            */
/******************************************************************************/

#define OSA_INTERNAL

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
#include "OsaError.h"
#include "OsaFile.h"

OSADLL EXTERNAL_CONST OsaFile OsaFileStdIn  = {0};
OSADLL EXTERNAL_CONST OsaFile OsaFileStdOut = {0};
OSADLL EXTERNAL_CONST OsaFile OsaFileStdErr = {0};


/******************************************************************************/

static BOOL
ConvertFileOpenMode(const CHAR* mode, 
                    int* flagsOsfHandle, 
                    unsigned int* dwDesiredAccess, 
                    unsigned int* dwCreationDisposition)
{
    BOOL returnValue = FALSE;

    
    OSA_ASSERT(NULL != mode);
    OSA_ASSERT(NULL != flagsOsfHandle);
    OSA_ASSERT(NULL != dwDesiredAccess);
    OSA_ASSERT(NULL != dwCreationDisposition);
    
    if (strlen(mode) != 0)
    {   
        if (strstr(mode, "a") != NULL)
        {
            if (strstr(mode, "a+") != NULL)
            {
                *flagsOsfHandle = _O_RDWR | _O_APPEND | _O_CREAT;
                *dwDesiredAccess = GENERIC_READ | GENERIC_WRITE | FILE_APPEND_DATA;
                *dwCreationDisposition = OPEN_ALWAYS;
                returnValue = TRUE;
            }
            else
            {
                *flagsOsfHandle = _O_WRONLY | _O_APPEND | _O_CREAT;
                *dwDesiredAccess = GENERIC_WRITE | FILE_APPEND_DATA;
                *dwCreationDisposition = OPEN_ALWAYS;
                returnValue = TRUE;
            }
        }
        else if (strstr(mode, "r") != NULL)
        {
            if (strstr(mode, "r+") != NULL)
            {
                *flagsOsfHandle = _O_RDWR;
                *dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
                *dwCreationDisposition = OPEN_EXISTING;
                returnValue = TRUE;
            }
            else
            {
                *flagsOsfHandle = _O_RDONLY;
                *dwDesiredAccess = GENERIC_READ;
                *dwCreationDisposition = OPEN_EXISTING;
                returnValue = TRUE;
            }
        }
        else if (strstr(mode, "w") != NULL)
        {
            if (strstr(mode, "w+") != NULL)
            {
                *flagsOsfHandle = _O_RDWR | _O_CREAT | _O_TRUNC;
                *dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
                *dwCreationDisposition = CREATE_ALWAYS;
                returnValue = TRUE;
            }
            else
            {
                *flagsOsfHandle = _O_WRONLY | _O_CREAT | _O_TRUNC;
                *dwDesiredAccess = GENERIC_WRITE;
                *dwCreationDisposition = CREATE_ALWAYS;
                returnValue = TRUE;
            }
        }
        
        if (strstr(mode, "b") != NULL)
        {
            *flagsOsfHandle |= _O_BINARY;
        }
        else 
        {
            //Default a file open is in textmode, so add the text flag
            *flagsOsfHandle |= _O_TEXT;
        }
    }

    return returnValue;
}

/******************************************************************************/
/* Open a file. */

OsaErr
OsaFileFOpen(OsaFile*    file,
             const CHAR* filename,
             const CHAR* mode)
{
    OsaErr  result     = OSA_EOK;
    errno_t localErrno = 0;
    int flags_open_osfhandle = 0;
    unsigned int createfile_dwDesiredAccess = 0;
    unsigned int createfile_dwCreationDisposition = 0;

    OSA_ASSERT(NULL != file);
    OSA_ASSERT(NULL != filename);
    OSA_ASSERT(NULL != mode);

    _set_errno(0);

    if ((0 < strlen(filename)) &&
        (ConvertFileOpenMode(mode, &flags_open_osfhandle, &createfile_dwDesiredAccess, &createfile_dwCreationDisposition) == TRUE))
    {
        HANDLE fh = CreateFileA(filename,
                                createfile_dwDesiredAccess,
                                FILE_SHARE_READ,
                                NULL,
                                createfile_dwCreationDisposition,
                                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
                                NULL);

        if (fh != INVALID_HANDLE_VALUE)
        {
            int fd = _open_osfhandle((intptr_t)fh, flags_open_osfhandle);
            
            if (fd != -1)
            {
                file->File = _fdopen(fd, mode);

                if (file->File == NULL)
                {                    
                    (VOID) _get_errno(&localErrno);

                    _close(fd);
                    
                    if(EINVAL == localErrno)
                    {
                        result = OSA_EPARAM;
                    }
                    else
                    {
                        OSA_HANDLE_ERRNO(localErrno);
                        result = OSA_ENOK;
                    }
                }
            }
            else
            {
                file->File = NULL;
                CloseHandle(fh);
            }
        }
        else
        {
            UINT32 errorValue = GetLastError();
            
            file->File = NULL;         

            switch (errorValue)
            {
                case ERROR_INVALID_PARAMETER:
                    result = OSA_EPARAM;
                    break;
                // default, but added explicitly for clarity
                case ERROR_FILE_NOT_FOUND: // no break
                case ERROR_PATH_NOT_FOUND: // no break
                default:

                    OSA_HANDLE_GETLASTERR(errorValue);

                    result = OSA_ENOK;
                    break;
            }
        }
    }
    else
    {
        file->File = NULL;
        result = OSA_EPARAM;
    }

    return result;
}

/******************************************************************************/
/* Reassign a file pointer. */

OsaErr
OsaFileFReOpen(OsaFile*    file,
               const CHAR* filename,
               const CHAR* mode,
               OsaFile     stream)
{
    OsaErr  result     = OSA_EOK;
    errno_t localErrno = 0;

    OSA_ASSERT(NULL != file);
    OSA_ASSERT(NULL != filename);
    OSA_ASSERT(NULL != mode);
    OSA_ASSERT(NULL != stream.File);

    _set_errno(0);
    if (0 != freopen_s(&file->File, filename, mode, stream.File))
    {
        (VOID) _get_errno(&localErrno);
    }

    if (localErrno != 0)
    {
        result = OSA_ENOK;

        if(EINVAL == localErrno)
        {
            result = OSA_EPARAM;
        }

        if (ENOENT != localErrno) /* not "No such file or directory." */
        {
            OSA_HANDLE_ERRNO(localErrno);
        }
    }

    return result;
}

/******************************************************************************/
/* Reads data from a stream. */

OsaErr
OsaFileFRead(void*   buffer,
             size_t  bufferSize,
             size_t  elementSize,
             size_t  count,
             OsaFile stream ,
             size_t* numberOfElements)
{
    OsaErr  result     = OSA_EOK;
    errno_t localErrno = 0;
    size_t  size;

    OSA_ASSERT(NULL != buffer);
    OSA_ASSERT(NULL != stream.File);
    OSA_ASSERT(NULL != numberOfElements);

    _set_errno(0);
    size = fread_s(buffer, bufferSize, elementSize, count, stream.File);
    (VOID) _get_errno(&localErrno);

    if (size < count)
    {
        int err = feof(stream.File);

        if (err != 0)
        {
            result = OSA_EEOF;
        }
        else
        {
            result = OSA_ENOK;

            if (EINVAL == localErrno)
            {
                result = OSA_EPARAM;
            }

            OSA_HANDLE_ERRNO(localErrno);
        }
    }

    *numberOfElements = size;

    return result;
}

/******************************************************************************/
/* Writes data to a stream. */

OsaErr
OsaFileFWrite(const void* buffer,
              size_t      size,
              size_t      count,
              OsaFile     stream ,
              size_t  *   numberOfElements)
{
    OsaErr  result = OSA_EOK;
    errno_t localErrno;
    size_t  writeSize;

    OSA_ASSERT(NULL != buffer);
    OSA_ASSERT(NULL != stream.File);
    OSA_ASSERT(NULL != numberOfElements);

    _set_errno(0);
    writeSize = fwrite(buffer, size, count, stream.File);

    if (0 != size)
    {
        {
            if (writeSize < count)
            {
                result = OSA_ENOK;

                (VOID) _get_errno(&localErrno);

                if (EINVAL == localErrno)
                {
                    result = OSA_EPARAM;
                }

                OSA_HANDLE_ERRNO(localErrno);
            }
            *numberOfElements = writeSize;
        }
    }
    else
    {
        result = OSA_EPARAM;
    }

    return result;
}

/******************************************************************************/
/* Closes a stream. */

OsaErr
OsaFileFClose(OsaFile* stream)
{
    OsaErr  result = OSA_EOK;
    errno_t localErrno;
    int     err ;

    OSA_ASSERT(NULL != stream);
    OSA_ASSERT(NULL != stream->File);

    _set_errno(0);
    err = fclose(stream->File);

    if (err == 0)
    {
        stream->File = NULL;
    }
    else
    {
        result = OSA_ENOK;

        (VOID) _get_errno(&localErrno);

        if (EINVAL == localErrno)
        {
            result = OSA_EPARAM;
        }

        OSA_HANDLE_ERRNO(localErrno);
    }
    return result;
}

/******************************************************************************/
/* Seek a position in a stream */

OSADLL OsaErr
OsaFileFSeek(OsaFile         stream,
             INT32           offset,
             OsaFileSeekEnum origin)
{
    OsaErr result = OSA_EOK;

    errno_t localErrno;
    int     err;
    int     originAnsi;

    OSA_ASSERT(NULL != stream.File);

    switch (origin)
    {
    case OSA_SEEK_BEGIN:
        originAnsi = SEEK_SET;
        break;
    case OSA_SEEK_END:
        originAnsi = SEEK_END;
        break;
    case OSA_SEEK_CURRENT:
        originAnsi = SEEK_CUR;
        break;
    default:
        originAnsi = (int)origin;
        break;
    }

    _set_errno(0);
    err = fseek(stream.File, offset, originAnsi);

    if (err != 0)
    {
        (VOID)_get_errno(&localErrno);
        switch (localErrno)
        {
        case EBADF:     /* fall through */
        case EINVAL:
            result = OSA_EPARAM;
            break;
        default:
            result = OSA_ENOK;
            break;
        }

        OSA_HANDLE_ERRNO(localErrno);
    }
    return result;
}

/******************************************************************************/
/* Get current position in a stream */

OSADLL OsaErr
OsaFileFTell(OsaFile stream,
             size_t* position)
{
    OsaErr result = OSA_EOK;

    errno_t localErrno;
    long    err;

    OSA_ASSERT(NULL != stream.File);
    OSA_ASSERT(NULL != position);

    _set_errno(0);
    err = ftell(stream.File);                                                  

    if (0 <= err)
    {
        *position = (size_t)err;
    }
    else
    {
        *position = 0;
        (VOID)_get_errno(&localErrno);
        switch (localErrno)
        {
        case EINVAL:    /* fall through */            
        case EBADF:
            result = OSA_EPARAM;
            break;
        default:
            result = OSA_ENOK;
            break;
        }

        OSA_HANDLE_ERRNO(localErrno);
    }
    return result;
}

/******************************************************************************/
/* Get end-of-file status of a stream */

OSADLL OsaErr
OsaFileFEof(OsaFile stream,
            BOOL*   atEof)
{
    OsaErr result = OSA_EOK;

    errno_t localErrno;
    int     err;

    OSA_ASSERT(NULL != stream.File);
    OSA_ASSERT(NULL != atEof);

    _set_errno(0);
    err = feof(stream.File);

    if (0 != err)
    {
        *atEof = TRUE;
    }
    else
    {
        *atEof = FALSE;
        (VOID)_get_errno(&localErrno);
        switch (localErrno)
        {
        case NOERROR:
            /* do nothing */
            break;
        case EINVAL:
            result = OSA_EPARAM;

            OSA_HANDLE_ERRNO(localErrno);
            break;
        default:
            result = OSA_ENOK;

            OSA_HANDLE_ERRNO(localErrno);
            break;
        }
    }
    return result;
}

/******************************************************************************/
/* Flush a stream */

OSADLL OsaErr
OsaFileFFlush(OsaFile stream)
{
    OsaErr result = OSA_EOK;

    errno_t localErrno;
    int     err;

    OSA_ASSERT(NULL != stream.File);

    _set_errno(0);
    err = fflush(stream.File);

    if (err != 0)
    {
        (VOID)_get_errno(&localErrno);
        switch (localErrno)
        {
        case EINVAL:    /* fall through */            
        case EBADF:
            result = OSA_EPARAM;
            break;
        default:
            result = OSA_ENOK;
            break;
        }

        OSA_HANDLE_ERRNO(localErrno);
    }
    return result;
}

/******************************************************************************/
/* Print formatted data to a stream. */

OsaErr
OsaFileFPrintF(OsaFile    stream,
              const CHAR* format,
              ...)
{
    OsaErr  result     = OSA_EOK;
    va_list argptr;

    OSA_ASSERT(NULL != stream.File);
    OSA_ASSERT(NULL != format);

    va_start(argptr, format);                                                   

    result = OsaFileVFPrintF(stream, format, argptr);

    va_end(argptr);

    return result;
}

/******************************************************************************/
/* Print formatted data to a stream using a pointer to a list of arguments */

OsaErr
OsaFileVFPrintF(OsaFile     stream,
                const CHAR* format,
                va_list     argptr)
{
    OsaErr  result     = OSA_EOK;
    errno_t localErrno = 0;
    int     size;

    OSA_ASSERT(NULL != stream.File);
    OSA_ASSERT(NULL != format);

    _set_errno(0);
    size = vfprintf_s(stream.File, format, argptr);
    (VOID) _get_errno(&localErrno);

    if (size < 0)
    {
        result = OSA_ENOK;

        if (EINVAL == localErrno)
        {
            result = OSA_EPARAM;
        }

        OSA_HANDLE_ERRNO(localErrno);
    }

    return result;
}

/******************************************************************************/
/* Print formatted data to a stdout. */

OsaErr
OsaFilePrintF(const CHAR* format,
              ...)
{
    OsaErr  result     = OSA_EOK;
    va_list argptr;

    OSA_ASSERT(NULL != format);

    va_start(argptr,format);                                                    

    result = OsaFileVPrintF(format, argptr);

    va_end(argptr);

    return result;
}

/******************************************************************************/
/* Print formatted data to a stdout using a pointer to a list of arguments */

OsaErr
OsaFileVPrintF(const CHAR* format,
               va_list     argptr)
{
    OsaErr  result     = OSA_EOK;
    errno_t localErrno = 0;
    int     size;

    OSA_ASSERT(NULL != format);

    _set_errno(0);

    size = vprintf_s(format, argptr);

    (VOID) _get_errno(&localErrno);

    if (size < 0)
    {
        result = OSA_ENOK;

        if (EINVAL == localErrno)
        {
            result = OSA_EPARAM;
        }

        OSA_HANDLE_ERRNO(localErrno);
    }

    return result;
}

/******************************************************************************/
/* Write formatted data to a string. */

OsaErr
OsaFileSPrintF(CHAR*       buffer,
               size_t      sizeOfBuffer,
               const CHAR* format,
               ...)
{
    OsaErr  result     = OSA_EOK;
    va_list argptr;

    OSA_ASSERT(NULL != buffer);
    OSA_ASSERT(NULL != format);

    va_start(argptr, format);                                                   

    result = OsaFileVSPrintF(buffer, sizeOfBuffer, format, argptr);

    va_end(argptr);

    return result;
}

/******************************************************************************/
/* Write formatted data to a string using a pointer to a list of arguments */

OsaErr
OsaFileVSPrintF(CHAR*       buffer,
                size_t      sizeOfBuffer,
                const CHAR* format,
                va_list     argptr)
{
    OsaErr  result     = OSA_EOK;
    errno_t localErrno = 0;
    int     size;
    
    OSA_ASSERT(NULL != buffer);
    OSA_ASSERT(NULL != format);

    _set_errno(0);
    size = vsprintf_s(buffer, sizeOfBuffer, format, argptr);
    (VOID) _get_errno(&localErrno);

    if (size < 0)
    {
        result = OSA_ENOK;

        if (EINVAL == localErrno)
        {
            result = OSA_EPARAM;
        }

        OSA_HANDLE_ERRNO(localErrno);
    }

    return result;
}

/******************************************************************************/
/* Write formatted data to a string. */

OsaErr
OsaFileSNPrintF(CHAR*       buffer,
                size_t      sizeOfBuffer,
                size_t      maxCount,
                const CHAR* format,
                ...)
{
    OsaErr  result     = OSA_EOK;
    va_list argptr;

    OSA_ASSERT(NULL != buffer);
    OSA_ASSERT(NULL != format);

    va_start(argptr, format);                                                 

    result = OsaFileVSNPrintF(buffer, sizeOfBuffer, maxCount, format, argptr);

    va_end(argptr);

    return result;
}

/******************************************************************************/
/* Write formatted data to a string using a pointer to a list of arguments */

OsaErr
OsaFileVSNPrintF(CHAR*       buffer,
                 size_t      sizeOfBuffer,
                 size_t      maxCount,
                 const CHAR* format,
                 va_list     argptr)
{
    OsaErr  result     = OSA_EOK;
    errno_t localErrno = 0;
    int     size;
    
    OSA_ASSERT(NULL != buffer);
    OSA_ASSERT(NULL != format);

    _set_errno(0);
    size = vsnprintf_s(buffer, sizeOfBuffer, maxCount, format, argptr);
    (VOID) _get_errno(&localErrno);

    if (size < 0)
    {
        result = OSA_ENOK;

        if (EINVAL == localErrno)
        {
            result = OSA_EPARAM;
        }

        OSA_HANDLE_ERRNO(localErrno);
    }

    return result;
}

/******************************************************************************/
/* Read a single CHAR from a file. */

OsaErr
OsaFileReadChar(OsaFile stream,
                CHAR*   variable)
{
    OsaErr  result     = OSA_EOK;
    errno_t localErrno = 0;
    int     size;

    OSA_ASSERT(NULL != stream.File);
    OSA_ASSERT(NULL != variable);

    _set_errno(0);
    size = fscanf_s(stream.File, "%c", variable, sizeof(*variable));
    (VOID) _get_errno(&localErrno);

    if (EOF == size)
    {
        result = OSA_EEOF;

        if (EINVAL == localErrno)
        {
            result = OSA_EPARAM;
        }

        OSA_HANDLE_ERRNO(localErrno);
    }
    return result;
}

/******************************************************************************/
/* Read a single string from a file. */
OsaErr
OsaFileReadString(OsaFile stream,
                  CHAR*   variable,
                  size_t  maxLength)
{
    OsaErr  result     = OSA_EOK;
    errno_t localErrno = 0;
    int     size;

    OSA_ASSERT(NULL != stream.File);
    OSA_ASSERT(NULL != variable);

    _set_errno(0);
    size = fscanf_s(stream.File, "%s", variable, maxLength);
    (VOID) _get_errno(&localErrno);

    if (EOF == size)
    {
        result = OSA_EEOF;

        if (EINVAL == localErrno)
        {
            result = OSA_EPARAM;
        }

        OSA_HANDLE_ERRNO(localErrno);
    }

    return result;
}

/******************************************************************************/
/* Read a single line from a file */

OSADLL OsaErr
OsaFileReadLine(OsaFile stream,
                CHAR*   variable,
                size_t  maxLength)
{
    OsaErr  result     = OSA_EOK;
    errno_t localErrno = 0;
    CHAR*   retVal     = NULL;

    OSA_ASSERT(NULL != stream.File);
    OSA_ASSERT(NULL != variable);

    _set_errno(0);
    retVal = fgets(variable, (int)maxLength, stream.File);
    if (NULL == retVal)
    {
        BOOL atEof = FALSE;
        (VOID)_get_errno(&localErrno);
        result = OsaFileFEof(stream, &atEof);
        if ((OSA_EOK == result) && atEof)
        {
            result = OSA_EEOF;
        }
        if (OSA_EEOF != result)
        {
            switch (localErrno)
            {
            case EINVAL:
                result = OSA_EPARAM;
                break;
            default:
                result = OSA_ENOK;
                break;
            }

            OSA_HANDLE_ERRNO(localErrno);
        }
    }
    if ((1 <= maxLength) && (OSA_EOK == result))
    {
        variable[maxLength - 1] = '\0';
    }
    return result;
}

/******************************************************************************/
/* Read a single unsigned int from a stream. */

OsaErr
OsaFileReadUInt(OsaFile stream,
                UINT32* variable)
{
    OsaErr  result     = OSA_EOK;
    errno_t localErrno = 0;
    int     size;

    OSA_ASSERT(NULL != stream.File);
    OSA_ASSERT(NULL != variable);

    _set_errno(0);
    size = fscanf_s(stream.File, "%u", variable);
    (VOID) _get_errno(&localErrno);

    if (EOF == size)
    {
        result = OSA_EEOF;

        if (EINVAL == localErrno)
        {
            result = OSA_EPARAM;
        }

        OSA_HANDLE_ERRNO(localErrno);
    }
    return result; 
}

/******************************************************************************/
/* Read a single int from a stream. */

OsaErr
OsaFileReadInt(OsaFile stream,
               INT32*  variable)
{
    OsaErr  result     = OSA_EOK;
    errno_t localErrno = 0;
    int     size;

    OSA_ASSERT(NULL != stream.File);
    OSA_ASSERT(NULL != variable);

    _set_errno(0);
    size = fscanf_s(stream.File, "%d", variable);
    (VOID) _get_errno(&localErrno);

    if (EOF == size)
    {
        result = OSA_EEOF;

        if (EINVAL == localErrno)
        {
            result = OSA_EPARAM;
        }

        OSA_HANDLE_ERRNO(localErrno);
    }
    return result;
}
 
/******************************************************************************/
/* Read a single double from a stream. */

OsaErr OsaFileReadDouble(OsaFile stream,
                         double* variable)
{
    OsaErr  result          = OSA_EOK;
    errno_t localErrno      = 0;
    int     size;

    OSA_ASSERT(NULL != stream.File);
    OSA_ASSERT(NULL != variable);

    _set_errno(0);
    size = fscanf_s(stream.File, "%lf", variable);
    (VOID) _get_errno(&localErrno);

    if (EOF == size)
    {
        result = OSA_EEOF;

        if (EINVAL == localErrno)
        {
            result = OSA_EPARAM;
        }

        OSA_HANDLE_ERRNO(localErrno);
    }
    return result;
}

/******************************************************************************/
/* Read a single float from a stream. */

OsaErr OsaFileReadFloat(OsaFile stream,
                        float*  variable)
{
    OsaErr  result     = OSA_EOK;
    errno_t localErrno = 0;
    int     size;

    OSA_ASSERT(NULL != stream.File);
    OSA_ASSERT(NULL != variable);

    _set_errno(0);
    size = fscanf_s(stream.File, "%f", variable);
    (VOID) _get_errno(&localErrno);

   if (EOF == size)
   {
        result = OSA_EEOF;

        if (EINVAL == localErrno)
        {
            result = OSA_EPARAM;
        }

        OSA_HANDLE_ERRNO(localErrno);
    }
    return result;
}

/******************************************************************************/
/* Get status information for a stream. */

OSADLL OsaErr
OsaFileFStat(OsaFile      stream,
             OsaFileStat* statInfo)
{
    OsaErr  result     = OSA_EOK;
    errno_t localErrno = 0;
    int     descriptor = 0;
    int     err        = 0;

    OSA_ASSERT(NULL != stream.File);
    OSA_ASSERT(NULL != statInfo);

    _set_errno(0);
    descriptor = _fileno(stream.File);
    (VOID) _get_errno(&localErrno);
    switch (localErrno)
    {
    case NOERROR:
        /* do nothing */
        break;
    case EINVAL:
        result = OSA_EPARAM;
        break;
    default:
        result = OSA_ENOK;
        break;
    }

    /* only execute fstat when _fileno successful */
    if (OSA_EOK == result)
    {
        _set_errno(0);
        err = fstat(descriptor, statInfo);
        if (0 != err)
        {
            (VOID) _get_errno(&localErrno);
            switch (localErrno)
            {
            case EINVAL:
                result = OSA_EPARAM;
                break;
            default:
                result = OSA_ENOK;
                break;
            }

           OSA_HANDLE_ERRNO(localErrno);
        }
    }
    return result;
}
