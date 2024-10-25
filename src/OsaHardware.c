/****************************************************************************/
/*                                                                          */
/*   Filename   : OsaHardware.c                                             */
/*                                                                          */
/*   Description: OS abstraction layer                                      */
/*                OS abstraction for unique hardware id                     */
/*                                                                          */
/*   Copyright  :                                                           */
/*                                                                          */
/****************************************************************************/

#define OSA_INTERNAL

#include <stdio.h>
#include "OsaError.h"
#include "OsaIpHelper.h"
#include "OsaHardware.h"
#include "OsaMem.h"
#include "OsaString.h"
#include "OsaFile.h"


static WCHAR adapterFriendlyName[ADAPTER_NAME_MAX_LENGTH] = L"Hospital Network";

/* Returns unique hardware ID string */
OsaErr
OsaUniqueHardwareID(CHAR* hardwareIdString,
                    UCHAR hardwareIdStringLength)
{
    OsaErr error = OSA_ENOK;
    UCHAR macAddress[MAC_ADDRESS_OCTET_COUNT];
    UCHAR charIndex = 0;
    UCHAR octetIndex = 0;
    CHAR localHardwareIdString[UNIQUE_HARDWARE_ID_STRING_LENGTH + 1];

    error = OsaGetMacAddress(adapterFriendlyName, macAddress, sizeof(macAddress));

    /* Translate array of macAdress octet to character array */
    for(octetIndex = 0; ((octetIndex < MAC_ADDRESS_OCTET_COUNT) && (error == OSA_EOK)); octetIndex++)
    {
        /* Read single octet/ 1 byte and convert it into two characters and write into macAddressString array */
        error = OsaFileSPrintF(&localHardwareIdString[charIndex], SINGLE_OCTET_STRING_LENGTH, "%.2x", macAddress[octetIndex]);
        charIndex = charIndex + 2;
    }

    if (OSA_EOK == error)
    {
        error = OsaStrCpy(hardwareIdString, hardwareIdStringLength, localHardwareIdString);
    }
    
    return error;
}

#ifdef UNIT_TEST
/* Get adapter name in use */
OsaErr GetAdpterName(WCHAR* adapterNameInUse, size_t size)
{
    OsaErr error = OsaWcsCpy(adapterNameInUse, size, adapterFriendlyName);
    return error;
}


/* Set new adapter name */
OsaErr SetAdpterName(const WCHAR* newAdapterName)
{
    OsaErr error = OsaWcsCpy(adapterFriendlyName, sizeof(adapterFriendlyName), newAdapterName);
    return error;
}
#endif