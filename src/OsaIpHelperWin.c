/****************************************************************************/
/*                                                                          */
/*   Filename   : OsaIpHelperWin.c                                          */
/*                                                                          */
/*   Description: OS abstraction layer                                      */
/*                OS abstraction for IP Helper for Windows                  */
/*                                                                          */
/*   Copyright  :                                                           */
/*                                                                          */
/****************************************************************************/

#define OSA_INTERNAL

#include <stdio.h>
#include <winsock2.h>
#include <Iphlpapi.h>

#include "OsaError.h"
#include "OsaIpHelper.h"
#include "OsaMem.h"
#include "OsaString.h"


static PIP_ADAPTER_ADDRESSES pToListOfAdapterInfo = NULL;

/* Returns MAC address associated with Adapter friendly name */
OsaErr
OsaGetMacAddress(const WCHAR*     pAdapterFriendlyName, 
                 UCHAR*           pMacAddress,
                 size_t           macAddrLength)
{
    OsaErr error = OSA_ENOK;
    ULONG retVal = ERROR_SUCCESS;
    /* Set the flags to pass to GetAdaptersAddresses */
    ULONG flags = GAA_FLAG_INCLUDE_PREFIX;
    /* IPv4 family */
    ULONG family = AF_INET;
    
    IP_ADAPTER_ADDRESSES adapterInformation;
    ULONG outBufLen = sizeof(ULONG);
    PIP_ADAPTER_ADDRESSES pCurrAddresses = NULL;
    UCHAR adapterfound = FALSE;
    
    /* Check parameter passed to function */
    if ((pAdapterFriendlyName == NULL) || (pMacAddress == NULL))
    {
        error = OSA_EPARAM;
    }
    else
    {
#ifdef UNIT_TEST
        UNREFERENCED_PARAMETER(adapterInformation);
        UNREFERENCED_PARAMETER(family);
        UNREFERENCED_PARAMETER(flags);
        UNREFERENCED_PARAMETER(outBufLen);
        error = OSA_EOK;
#else
        /* Pass too small buffer size to get the size needed into the outBufLen variable */
        retVal = GetAdaptersAddresses(family, flags, NULL, &adapterInformation, &outBufLen);

        /* Check guaranteed return value ERROR_BUFFER_OVERFLOW */
        if (retVal == ERROR_BUFFER_OVERFLOW)
        {
            /* Allocate sufficient memory */
            error = OsaMemAlloc((void**)&pToListOfAdapterInfo, outBufLen);
        }
#endif
        /* No issue in memory allocation */
        if (error == OSA_EOK )
        {
#ifdef UNIT_TEST
            retVal = ERROR_SUCCESS;
#else
            /* Read all adapter information */
            retVal = GetAdaptersAddresses(family, flags, NULL, pToListOfAdapterInfo, &outBufLen);
#endif
            if ((retVal == ERROR_SUCCESS) && (pToListOfAdapterInfo != NULL))
            {
                /* Point to first adapter information in the list */
                pCurrAddresses = pToListOfAdapterInfo;
                while ((pCurrAddresses != NULL) && (adapterfound == FALSE))
                {
                    error = OSA_ENOK;
                    /* wide strings compare */
                    if (wcscmp(pAdapterFriendlyName, pCurrAddresses->FriendlyName) == 0)
                    {
                        error = OsaMemCpy(pMacAddress, macAddrLength, (const UCHAR*)pCurrAddresses->PhysicalAddress, pCurrAddresses->PhysicalAddressLength);
                        adapterfound = TRUE;
                    }
                    /* Travel to next adapter information */
                    pCurrAddresses = pCurrAddresses->Next;
                }
            }
            else
            {
                error = OSA_ENOK;
            }
        }
#ifndef UNIT_TEST
        (void)OsaMemFree((void**)&pToListOfAdapterInfo);
#endif
    }
    return error;
}


#ifdef UNIT_TEST

/*
 *      Sets pointer to list of adapter info used by OsaGetMacAddress; 
 *      The list is populated for unit testing
 */
OsaErr
SetPointerToListOfAdapterInfoForTest(const void* pToListOfTestAdapterInfo)
{
    OsaErr retVal = OSA_ENOK;
    if (pToListOfTestAdapterInfo != NULL)
    {
        pToListOfAdapterInfo = (PIP_ADAPTER_ADDRESSES)pToListOfTestAdapterInfo;
        retVal = OSA_EOK;
    }
    return retVal;
}

#endif