/******************************************************************************/
/*                                                                            */
/*   Filename   : OsaHardware.h                                               */
/*                                                                            */
/*   Description: OS abstraction layer interface                              */
/*                OS abstraction interface to read unique hardware id         */
/*                                                                            */
/*   Copyright  :                                                             */
/*                                                                            */
/******************************************************************************/

#ifndef OSAHARDWARE_H
#define OSAHARDWARE_H

#include "OsaGenTypes.h"

#define ADAPTER_NAME_MAX_LENGTH                         (50)
#define MAC_ADDRESS_OCTET_COUNT                         (6)
#define UNIQUE_HARDWARE_ID_STRING_LENGTH                (MAC_ADDRESS_OCTET_COUNT * 2)
#define SINGLE_OCTET_STRING_LENGTH                      (3)

/* Retrieve a pointer to the unique hardware ID */
/*  This routine will return a pointer to the unique hardware ID */
/*  of the subsystem it runs. */
OSADLL OsaErr
OsaUniqueHardwareID(CHAR* hardwareIdString,
                    UCHAR hardwareIdStringLength);


#ifdef UNIT_TEST
/* Retrieves adapter name which is currently in use */
/*  This routine will copy adapter name in use to destination buffer pointer */
/*  passed to this function */
OSADLL OsaErr  
GetAdpterName(WCHAR* adapterNameInUse, size_t size);


/* Updates adapter name */
/*  This routine will set new adapter name to be used by OsaUniqueHardwareID to */
/*  to get unique hardware Id */
OSADLL OsaErr 
SetAdpterName(const WCHAR* newAdapterName);
#endif /* #ifdef UNIT_TEST */

#endif  /* OSAHARDWARE_H */
