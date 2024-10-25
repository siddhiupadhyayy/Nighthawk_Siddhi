/******************************************************************************/
/*                                                                            */
/*   Filename   : OsaMsgQ.h                                                   */
/*                                                                            */
/*   Description: OS abstraction layer interface                              */
/*                OS abstraction interface for message queues                 */
/*                                                                            */
/*   Copyright  :                                                             */
/*                                                                            */
/******************************************************************************/

#ifndef OSAMSGQ_H
#define OSAMSGQ_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "OsaGenTypes.h"

/* OS abstraction interface for message queues */

/******************************************************************************/
/*                                                                            */
/*      Message Queues                                                        */
/*                                                                            */
/******************************************************************************/

/* Creates a message queue. */

OSADLL OsaErr
OsaMsgQCreate(UINT32      maxMsgs,
              size_t      maxMsgSize,
              const CHAR* name,
              OsaMsgQId*  msgQId);

/* Deletes a message queue. */
/*  If the queue id is acquired with OsaMsgQCreate, the message queue is deleted. */
/*  Do not use this function with id's acquired via OsaMsgQOpen (Win32 only). */
 
OSADLL OsaErr
OsaMsgQDelete(OsaMsgQId* msgQId);

/* Open an id for an existing message queue. */ 
/*            *** WIN32 only! ***            */
/* Open an id for an existing message queue to allow sending messages  */
/* to it. Message can be sent across process boundaries. */
/* This function may returns OSA_ENOK on based on other platforms */

OSADLL OsaErr
OsaMsgQOpen(CHAR*      name,
            OsaMsgQId* msgQId);

/* Close an id for a message queue id acquired with OsaMsgQOpen. */
/*             *** WIN32 only! ***                               */
/*  Close an id for a message queue id acquired with OsaMsgQOpen. */
/*  Do not use this function with id's acquired via OsaMsgQCreate. */
/*  This function may returns OSA_ENOK on based on other platforms */

OSADLL OsaErr
OsaMsgQClose(OsaMsgQId* msgQId);


/* Send a message to a message queue. */

OSADLL OsaErr
OsaMsgQSend(OsaMsgQId msgQId,
            void*     buffer,
            size_t    size,
            UINT32    timeout);

/* Receive a message from a message queue */

OSADLL OsaErr
OsaMsgQReceive(OsaMsgQId msgQId,
               void*     buffer,
               size_t    bufferSize,
               UINT32    timeout,
               size_t*   bytesReceived);

/* Check if there are messages in a message queue */
/*  Indication if one or more messages are available in the message queue. */
/*  This function can only be used by the receiver, not by the sender. */
/*  When a OsaMsgQReceive function would be active when this function is */
/*  called, OsaMsgQReceive will return with zero bytes received. */

OSADLL OsaErr
OsaMsgQNumMsgs(OsaMsgQId msgQId,
               UINT32*   numAvailable);

#endif  /* OSAMSGQ_H */
