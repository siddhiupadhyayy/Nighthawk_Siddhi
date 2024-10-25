/******************************************************************************/
/*                                                                            */
/*   Filename   : OsaMem.h                                                    */
/*                                                                            */
/*   Description: OS abstraction layer interface                              */
/*                OS abstraction interface for memory allocation              */
/*                                                                            */
/*   Copyright  :                                                             */
/*                                                                            */
/******************************************************************************/

#ifndef OSAMEM_H
#define OSAMEM_H

#include "OsaGenTypes.h"

/* OS abstraction interface for memory allocation */

/******************************************************************************/
/*                                                                            */
/*      MEMORY                                                                */
/*                                                                            */
/******************************************************************************/

/* The OsaMemAlloc function allocates a memory block of at least size bytes. */
/* Always check the return from OsaMemAlloc, even if the amount of memory */
/* requested is small. */

OSADLL OsaErr 
OsaMemAlloc(void** mem,
            size_t size);

/* The OsaMemRealloc function changes the size of an allocated memory block */
/* The contents of the block are unchanged up to the shorter of the new and */
/* old sizes, although the new block can be in a different location. Because */
/* the new block can be in a new memory location, the mem pointer returned by */
/* realloc is not guaranteed to be the pointer initially passed through. */
/* OsaMemRealloc does not zero newly allocated memory in the case of buffer growth. */
/* Always check the return from OsaMemRealloc, even if the amount of memory */
/* requested is small. */

OSADLL OsaErr
OsaMemRealloc(void** mem,
              size_t size);

/* The OsaMemCalloc allocates an array in memory with elements */
/* initialized to 0. */
/* Always check the return from OsaMemAlloc, even if the amount of memory */
/* requested is small. */

OSADLL OsaErr
OsaMemCalloc(void** mem,
             size_t count,
             size_t size);

/* The free function deallocates a memory block, that was previously */
/* allocated by OsaMemAlloc, OsaMemRealloc, or OsaMemCalloc. */

OSADLL OsaErr
OsaMemFree(void** mem);

/* TheOsaMemCpy function copies a block of memory to another block. */
/* When the source and destination block overlap, the behavior is undefined. */

OSADLL OsaErr
OsaMemCpy(void*       dst,
          size_t      dstSize,
          const void* src,
          size_t      count);

/* The OsaMemMove function moves only block of memory to another block. */
/*  When the source and destination block overlap, the function makes sure, that */
/*  bytes from source are moved before they are overwritten. Thus the */
/*  destination will be a correct representation of the original source. */
/*  The source may be affected by the move (in case of overlap). */

OSADLL OsaErr
OsaMemMove(void*       dst,
           size_t      dstSize,
           const void* src,
           size_t      count);
#endif  // OSAMEM_H
