/*****************************************************************************/
/*                                                                           */
/*   Filename   : OsaList.c                                                  */
/*                                                                           */
/*   Description: OS abstraction layer helper                                */
/*                OS abstraction interface for lists                         */
/*                                                                           */
/*   Copyright  :                                                            */
/*                                                                           */
/*****************************************************************************/

#define OSA_INTERNAL

#include "OsaList.h"
#include "OsaMem.h"

/*
 *  First node in a double linked list.
 */
#define OSAFIRST node.next

/*
 *  Last node in a double linked list.
 */
#define OSALAST  node.prev

/*****************************************************************************/

/* Initialize a list. */
OsaErr
OsaListInit(OsaList* osaList)
{
    OSA_ASSERT(NULL != osaList);

    osaList->count    = 0;
    osaList->OSAFIRST = NULL;
    osaList->OSALAST  = NULL;

    return OSA_EOK;
}

/* Add a node to a list. */
OsaErr
OsaListAdd(OsaList* osaList,
           OsaNode* osaNode)
{
    OSA_ASSERT(NULL != osaList);
    OSA_ASSERT(NULL != osaNode);

    return OsaListInsert(osaList, osaList->OSALAST, osaNode);
}

/* Get the number of nodes. */
OsaErr
OsaListCount(const OsaList* osaList,
             INT32*         osaCount)
{
    OSA_ASSERT(NULL != osaList);
    OSA_ASSERT(NULL != osaCount);

    *osaCount = osaList->count;

    return OSA_EOK;
}

/* Delete a node from a list. */
OsaErr
OsaListDelete(OsaList* osaList,
              OsaNode* osaNode)
{
    INT32  index    = 0;
    OsaErr osaError = OSA_EOK;

    OSA_ASSERT(NULL != osaList);
    OSA_ASSERT(NULL != osaNode);

    osaError = OsaListFind(osaList, osaNode, &index);

    if (-1 != index)
    {
        /* Is osaNode first node in the list? */
        if ((NULL == osaNode->prev))
        {
            /* Yes, so update the list header to point to next node of osaNode
                as the first node in the list.*/
            osaList->OSAFIRST = osaNode->next;
        }
        else
        {
            /* No, so update the previous node of OsaNode to point to the next
                node of osaNode.*/
            osaNode->prev->next = osaNode->next;
        }

        /* Is osaNode last node in the list? */
        if (NULL == osaNode->next) 
        {
            /* Yes, so update the list header to point to the previous node of
                osaNode as the last node in the list.*/ 
            osaList->OSALAST = osaNode->prev;
        }
        else
        {
            /* No, so update the next node of OsaNode to point to the previous
                node of OsaNode.*/
            osaNode->next->prev = osaNode->prev;
        }

        osaNode->next = NULL;
        osaNode->prev = NULL;
        osaList->count--;
    }
    else
    {
        osaError = OSA_EPARAM;
    }

    return osaError;
}

/* Find first node in a list. */
OsaErr
OsaListFirst(OsaList*  osaList,
             OsaNode** osaFirstNode)
{
    OSA_ASSERT(NULL != osaList);
    OSA_ASSERT(NULL != osaFirstNode);

    *osaFirstNode = osaList->OSAFIRST;

    return (*osaFirstNode != NULL) ? OSA_EOK : OSA_EEXIST;
}

/* Insert node in a list. */
OsaErr
OsaListInsert(OsaList* osaList,
              OsaNode* osaPrevNode,
              OsaNode* osaNode)
{
    INT32    index       = 0;
    OsaErr   osaError    = OSA_EOK;

    OSA_ASSERT(NULL != osaList);
    OSA_ASSERT(NULL != osaNode);

    if (NULL != osaPrevNode)
    {
        osaError = OsaListFind(osaList, osaPrevNode, &index);
    }

    if (-1 != index)
    {
        OsaNode* osaNextNode = NULL;

        /* When previous node is NULL, ... */
        if (NULL == osaPrevNode)
        {
            /* ... add new node to the front of the list.*/
            /* Obtain the current first node */
            osaNextNode = osaList->OSAFIRST;

            /* The new node must be first node in the list */
            osaList->OSAFIRST = osaNode;
        }
        else
        {
            /* ... add new node after osaPrevNode. */
            /* Obtain the node after the osaPrevNode */
            osaNextNode = osaPrevNode->next;

            /* osaPrevNode must point to new node */
            osaPrevNode->next = osaNode;
        }

        /* When osaNextNode is NULL, ... */
        if (NULL == osaNextNode)
        {
            /* ... the new node must be last in the list */
            osaList->OSALAST = osaNode;
        }
        else
        {
            /* ... next node must point back to the new (inserted) node.*/
            osaNextNode->prev = osaNode;
        }

        /* Update the next and prev pointers of the new node.*/
        osaNode->next = osaNextNode;
        osaNode->prev = osaPrevNode;

        /* Update count of the list header.*/
        osaList->count++;
    }
    else
    {
        osaError = OSA_EPARAM;
    }

    return osaError;
}

/* Find the last node in a list. */
OsaErr
OsaListLast(OsaList*  osaList,
            OsaNode** osaLastNode)
{
    OSA_ASSERT(NULL != osaList);
    OSA_ASSERT(NULL != osaLastNode);

    *osaLastNode = osaList->OSALAST;

    return (*osaLastNode != NULL) ? OSA_EOK : OSA_EEXIST;
}

/* Find the next node in a list. */
OsaErr
OsaListNext(OsaNode*  osaNode,
            OsaNode** osaNextNode)
{
    OSA_ASSERT(NULL != osaNode);
    OSA_ASSERT(NULL != osaNextNode);

    *osaNextNode = osaNode->next;

    return (*osaNextNode != NULL) ? OSA_EOK : OSA_EEXIST;
}

/* Find the previous node in a list. */
OsaErr
OsaListPrev(OsaNode*  osaNode,
            OsaNode** osaPrevNode)
{
    OSA_ASSERT(NULL != osaNode);
    OSA_ASSERT(NULL != osaPrevNode);

    *osaPrevNode = osaNode->prev;

    return (*osaPrevNode != NULL) ? OSA_EOK : OSA_EEXIST;
}

/* Find the Nth node in a list. */
OsaErr
OsaListNth(OsaList*  osaList,
           INT32     nrOfNode,
           OsaNode** osaNthNode)
{
    OsaErr error = OSA_EOK;

    OSA_ASSERT(NULL != osaList);
    OSA_ASSERT(NULL != osaNthNode);
    
    if ((nrOfNode > osaList->count) || (nrOfNode <= 0))
    {
        error = OSA_ERANGE;
    }
    else
    {
        if (1 == nrOfNode)
        {
            error = OsaListFirst(osaList, osaNthNode);
        }
        else if (osaList->count == nrOfNode)
        {
            error = OsaListLast(osaList, osaNthNode);
        }
        else
        {
            /* Determine whether to start from OSAFIRST or OSALAST.*/
            BOOL startAtFirst = ((osaList->count - nrOfNode) >= nrOfNode);

            /* Determine number of cycles before the Nth node is found.*/
            INT32 cycles =
                startAtFirst ? (nrOfNode - 1) : (osaList->count - nrOfNode);   

            /* Set tempNode to first or last node.*/
            OsaNode* tempNode =
                startAtFirst ? osaList->OSAFIRST : osaList->OSALAST;           
            
            /* Find the Nth node.*/
            while (cycles > 0)
            {
                /* Depending on where start is go up or down the list.*/
                tempNode = startAtFirst ? tempNode->next : tempNode->prev;   
                cycles--;
            }

            /* Set the Nth node.*/
            *osaNthNode = tempNode;
        }
    }

    return error;
}

/* Find a node in a list. */
OsaErr
OsaListFind(OsaList* osaList,
            OsaNode* osaNode,
            INT32*   nrOfNode)
{
    OsaErr   error = OSA_EOK;
    OsaNode* osaNextNode;
    INT32    index = 1;

    OSA_ASSERT(NULL != osaList);
    OSA_ASSERT(NULL != osaNode);
    OSA_ASSERT(NULL != nrOfNode);

    osaNextNode = osaList->OSAFIRST;

    /* Find the osaNode in osaList.*/
    while ((NULL != osaNextNode) &&      /* end of list reached */
            (OSA_EOK == error) &&        /* error occured       */
            (osaNextNode != osaNode))    /* node not found      */
    {
        error = OsaListNext(osaNextNode, &osaNextNode);
        index++;
    }

    /* set the nrOfNode to -1 if osaNode not found or index when found.*/
    if ((NULL == osaNextNode) || (OSA_EOK != error))
    {
        *nrOfNode = -1;
    }
    else
    {
        *nrOfNode = index;
    }

    return error;
}
