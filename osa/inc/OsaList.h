/******************************************************************************/
/*                                                                            */
/*   Filename   : OsaList.h                                                   */
/*                                                                            */
/*   Description: OS abstraction layer interface                              */
/*                OS abstraction interface for lists                          */
/*                                                                            */
/*   Copyright  :                                                             */
/*                                                                            */
/******************************************************************************/

#ifndef OSALIST_H
#define OSALIST_H

#include "OsaGenTypes.h"

/* OS abstraction interface for lists */
/* This part of OSA implements functions for a double linked list */
/*  NOTE: user is responsible for freeing any memory allocated to nodes */


/* Node for a double linked list. */
/* Contains pointers to next and previous nodes. */
typedef struct OsaNodeType
{
    struct OsaNodeType* next; /*!< Pointer to a next node in the list. Null for last node. */
    struct OsaNodeType* prev; /*!< Pointer to a previous node in the list. Null for first node. */
} OsaNode;

/* Header for a double linked list. */
/* Contains a node and a count. */
typedef struct
{
    OsaNode node;   /* Contains pointers to first and last node in the list. */
    INT32   count;  /* The number of nodes in the list. */
} OsaList;

/*****************************************************************************/

/* Initialize a list. */
/*  Initializes an empty double linked list. */
/*  node.next and node.prev are NULL and count is zero. */
/*  \param[in,out] osaList  Pointer to a list to be initialized. */
OSADLL OsaErr
OsaListInit(OsaList* osaList);

/* Add a node to a list. */
/* Add the node to the end of a double linked list. */
OSADLL OsaErr
OsaListAdd(OsaList* osaList,
           OsaNode* osaNode);

/* Get the number of nodes. */
/*  Report the number of nodes in a double linked list. */
OSADLL OsaErr
OsaListCount(const OsaList* osaList,
             INT32* osaCount);

/* Delete a node from a list. */
/* Deletes the node from a double linked list. */
OSADLL OsaErr
OsaListDelete(OsaList* osaList,
              OsaNode* osaNode);

/* Find first node in a list. */
/* Finds the first node in a double linked list. */
OSADLL OsaErr
OsaListFirst(OsaList*  osaList,
             OsaNode** osaFirstNode);

/* Insert node in a list. */ 
/* Inserts a node to a double linked list, placing it after a specified node. */
OSADLL OsaErr
OsaListInsert(OsaList* osaList,
              OsaNode* osaPrevNode,
              OsaNode* osaNode);

/* Find the last node in a list. */
/* Finds the last node in a double linked list. */
OSADLL OsaErr
OsaListLast(OsaList*  osaList,
            OsaNode** osaLastNode);

/* Find the next node in a list. */
/*  Finds the next node of a node in a double linked list. */
OSADLL OsaErr
OsaListNext(OsaNode*  osaNode,
            OsaNode** osaNextNode);

/* Find the previous node in a list. */
/* Finds the previous node of a node in a double linked list. */
OSADLL OsaErr
OsaListPrev(OsaNode*  osaNode,
            OsaNode** osaPrevNode);

/* Find the Nth node in a list. */
/* Finds the node specified by nrOfNode, where first node is 1, in a double */
/*  linked list. */
OSADLL OsaErr
OsaListNth(OsaList*  osaList,
           INT32     nrOfNode,
           OsaNode** osaNthNode);

/* Find a node in a list. */
/*  Finds the node specified and returns the number of the node in a double */
/*  linked list, where first node is 1. */
OSADLL OsaErr
OsaListFind(OsaList* osaList,
            OsaNode* osaNode,
            INT32*   nrOfNode);

#endif  // OSALIST_H
