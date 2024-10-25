/******************************************************************************/
/*                                                                            */
/*   Filename   : Osa.h                                                       */
/*                                                                            */
/*   Description: OS abstraction layer interface                              */
/*                                                                            */
/*   Copyright  :                                                             */
/*                                                                            */
/******************************************************************************/

#ifndef OSA_H
#define OSA_H

/* OS abstraction interface */  
/*  Details: */
/*  The OS abstraction layer is the application interface to access */
/*  functions of the OS (WIN32 or any other) */
/*
/*  When including this file, make sure to comply with the following */
/*  rule: */
/*  - first include OS and standard header files */
/*  - second include Osa.h */
/*  - third include application header files */
/*  This is because types exposed by Osa.h are (re)defined in OSA header */
/*  files. OS and standard header files will cause compiler warnings or */
/*  errors when included after Osa.h. */
  
/*  The interface contains the following sections: */
  
/*  - THREADS */
/*  - SYNCHRONIZATION PRIMITIVES */
/*  - MEMORY */
/*  - MESSAGE QUEUES */
/*  - STRINGS */
/*  - FILE HANDLING */
/*  - SOCKETS */
/*  - LISTS */

#ifdef __cplusplus
extern "C"
{
#endif  /* __cplusplus */

#ifdef WIN32
#ifndef _INC_WINDOWS  /* windows.h is already included */
#include <time.h>     /* for timeval, for OsaTime */
#include <WS2tcpip.h> /* for socket options, for OsaSocket */
#endif /* _INC_WINDOWS */
#endif /* WIN32 */

#ifndef __cplusplus
#ifndef min
#define min(x, y)    (((x) < (y)) ? (x) : (y))
#endif

#ifndef max
#define max(x, y)    (((x) < (y)) ? (y) : (x))
#endif
#endif  /* __cplusplus */

#ifdef OSA_STUB
#include "OsaInit_stub.h"
#include "OsaThreads_stub.h"
#include "OsaSync_stub.h"
#include "OsaMem_stub.h"
#include "OsaMsgQ_stub.h"
#include "OsaString_stub.h"
#include "OsaSocket_stub.h"
#include "OsaSync_stub.h"
#include "OsaFile_stub.h"
#else
#include "OsaGenTypes.h"
#include "OsaInit.h"
#include "OsaError.h"
#include "OsaThreads.h"
#include "OsaSync.h"
#include "OsaMem.h"
#include "OsaMsgQ.h"
#include "OsaString.h"
#include "OsaList.h"
#include "OsaSocket.h"
#include "OsaFile.h"
#include "OsaTime.h"
#include "OsaIpHelper.h"
#include "OsaHardware.h"
#endif // OSA_STUB

#ifdef __cplusplus
}   /* extern "C" */
#endif  /* __cplusplus */

#endif  /* OSA_H */
