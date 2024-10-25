/******************************************************************************/
/*                                                                            */
/*   Filename   OsaFile.h                                                     */
/*                                                                            */
/*   Description: OS abstraction layer                                        */
/*                OS abstraction for file handling for Windows                */
/*                                                                            */
/*   Copyright  :                                                             */
/*                                                                            */
/******************************************************************************/

#ifndef OSAFILE_H
#define OSAFILE_H

#ifdef OSA_INTERNAL /* for FILE type */
#include <stdio.h>
#endif

//#if (defined(VXWORKS) || ((defined SUBSYS_CPU) && (defined SIMNT) && (SUBSYS_CPU == SIMNT)))
//#include <stdarg.h> /* for va_list */
//#endif

#include "OsaGenTypes.h"

#if defined WIN32
#define OSA_FILE_DIR_SEPARATOR "\\"
//#elif defined VXWORKS
//#define OSA_FILE_DIR_SEPARATOR "/"
//#elif ((defined SUBSYS_CPU) && (defined SIMNT) && (SUBSYS_CPU == SIMNT))
//#define OSA_FILE_DIR_SEPARATOR "\\"
#endif

/* details To prevent accidental usage of standard file */
/* functions with an OsaFile type, the FILE* type is hidden */
/* for the application using OSA. Using OsaFile with a */
/* standard file function in the application may crash the */
/* application. */
/* Use non-pointer type in a struct to prevent implicit */
/* casts to FILE*. The type size_t has the same size as a */
/* pointer. */
typedef struct
{
#ifndef OSA_INTERNAL
    /*! File specifier */
    size_t File;
#else
    /*! Use FILE* internally */
    FILE* File;
#endif
} OsaFile;

/* Flags that can be used while seeking in a file */
typedef enum
{
    /* Specifies to seek from the beginning of the file        */
    OSA_SEEK_BEGIN,
    /* Specifies to seek from the current position             */
    OSA_SEEK_CURRENT,
    /* Specifies to seek from the end of the file              */
    OSA_SEEK_END
} OsaFileSeekEnum;

typedef struct stat OsaFileStat;

#ifndef OSA_INTERNAL
#define EXTERNAL_CONST const
#else
#define EXTERNAL_CONST
#endif
OSADLL extern EXTERNAL_CONST OsaFile OsaFileStdIn;
OSADLL extern EXTERNAL_CONST OsaFile OsaFileStdOut;
OSADLL extern EXTERNAL_CONST OsaFile OsaFileStdErr;

/* brief OS abstraction interface for file handling. */
/*  The OsaFile package provides file handling functions. If available, */
/*  secure versions of the underlying ANSI functions are used. */
/*  There are no provisions for handling wide character file names. */

/******************************************************************************/
/*                                                                            */
/*      FILE HANDLING                                                         */
/*                                                                            */
/******************************************************************************/

/* The OsaFileFOpen function opens a file. */
OSADLL OsaErr
OsaFileFOpen(OsaFile*    file,
             const CHAR* filename,
             const CHAR* mode);

/* The OsaFileFReOpen function reassign a file pointer. */
OSADLL OsaErr
OsaFileFReOpen(OsaFile*    file,
               const CHAR* filename,
               const CHAR* mode,
               OsaFile     stream);

/* The OsaFileFRead function reads data from a stream. */
OSADLL OsaErr
OsaFileFRead(void*   buffer,
             size_t  bufferSize,
             size_t  elementSize,
             size_t  count,
             OsaFile stream,
             size_t* numberOfElements);

/* The OsaFileFWrite function writes data to a stream. */
OSADLL OsaErr
OsaFileFWrite(const void* buffer,
              size_t      size,
              size_t      count,
              OsaFile     stream,
              size_t*     numberOfElements );

/* The OsaFileFClose function closes a stream. */
OSADLL OsaErr
OsaFileFClose(OsaFile* stream);

/* The OsaFileFSeek function seeks a position in a stream. */
OSADLL OsaErr
OsaFileFSeek(OsaFile         stream,
             INT32           offset,
             OsaFileSeekEnum origin);

/* The OsaFileFTell function gets the current position in the stream. */
OSADLL OsaErr
OsaFileFTell(OsaFile stream,
             size_t* position);

/* The OsaFileFEof function checks for the end-of-file status of */
/* the stream. */ 
OSADLL OsaErr
OsaFileFEof(OsaFile stream,
            BOOL*   atEof);

/* The OsaFileFFlush function flushes the stream. */
OSADLL OsaErr
OsaFileFFlush(OsaFile stream);

/* The OsaFileFPrintF function writes formatted data to a stream. */
OSADLL OsaErr
OsaFileFPrintF(OsaFile     stream,
               const CHAR* format,
               ...);

/* The OsaFileVFPrintF function writes formatted data to a stream */
/* using a pointer to a list of arguments. */
OSADLL OsaErr
OsaFileVFPrintF(OsaFile     stream,
                const CHAR* format,
                va_list     argptr);

/* The OsaFilePrintF function writes formatted data to stdout. */
OSADLL OsaErr
OsaFilePrintF(const CHAR* format,
              ...);

/* The OsaFileVPrintF function writes formatted data to stdout */
/* using a pointer to a list of arguments. */
OSADLL OsaErr
OsaFileVPrintF(const CHAR* format,
               va_list     argptr);

/* The OsaFileSPrintF function writes formatted data to a string. */
OSADLL OsaErr
OsaFileSPrintF(CHAR*       buffer,
               size_t      sizeOfBuffer,
               const CHAR* format,
               ...);

/* The OsaFileVSPrintF function writes formatted data to a string */
/* using a pointer to a list of arguments. */
OSADLL OsaErr
OsaFileVSPrintF(CHAR*       buffer,
                size_t      sizeOfBuffer,
                const CHAR* format,
                va_list     argptr);

/* The OsaFileSNPrintF function writes formatted data to a string. */
OSADLL OsaErr
OsaFileSNPrintF(CHAR*       buffer,
                size_t      sizeOfBuffer,
                size_t      maxCount,
                const CHAR* format,
                ...);

/* The OsaFileVSNPrintF function writes formatted data to a string. */
/* using a pointer to a list of arguments. */
OSADLL OsaErr
OsaFileVSNPrintF(CHAR*       buffer,
                 size_t      sizeOfBuffer,
                 size_t      maxCount,
                 const CHAR* format,
                 va_list     argptr);

/* The OsaFileReadChar function reads a single CHAR from a stream. */
OSADLL OsaErr
OsaFileReadChar(OsaFile stream,
                CHAR*   variable);

/* The OsaFileReadString function reads a single string from a stream. */
OSADLL OsaErr
OsaFileReadString(OsaFile stream,
                  CHAR*   variable,
                  size_t  maxLength);

/* The OsaFileReadLine functions reads a single line from a stream. */
/* If a line break is encountered, before maxLength characters are read, the */
/* line break is included in the string. */
/* maxLength is the maximum number of characters to return, including the */
/* terminating null character. */
OSADLL OsaErr
OsaFileReadLine(OsaFile stream,
                CHAR*   variable,
                size_t  maxLength);

/* The OsaFileReadInt function reads a single int from a stream. */
OSADLL OsaErr
OsaFileReadInt(OsaFile stream,
               INT32*  variable);

/* The OsaFileReadUInt function reads a single unsigned int from a stream. */
OSADLL OsaErr
OsaFileReadUInt(OsaFile stream,
                UINT32* variable);

/* The OsaFileReadDouble function reads a single double from a stream. */
OSADLL OsaErr
OsaFileReadDouble(OsaFile stream,
                  double* variable);

/* The OsaFileReadFloat function reads a single float from a stream. */
OSADLL OsaErr
OsaFileReadFloat(OsaFile stream,
                 float*  variable);

/* The OsaFileFStat function gets information about a stream */
OSADLL OsaErr
OsaFileFStat(OsaFile      stream,
             OsaFileStat* statInfo);

#endif  // OSAFILE_H
