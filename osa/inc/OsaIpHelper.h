/******************************************************************************/
/*                                                                            */
/*   Filename   : OsaIpHelper.h                                               */
/*                                                                            */
/*   Description: OS abstraction layer interface                              */
/*                OS abstraction interface for IP Helper functions on Windows */
/*                                                                            */
/*   Copyright  :                                                             */
/*                                                                            */
/******************************************************************************/

#ifndef OSAIPHELPER_H
#define OSAIPHELPER_H

#include "OsaGenTypes.h"

/* The function returns MAC address associated with friendly name of network adapter */
OSADLL OsaErr
OsaGetMacAddress(const WCHAR*      pAdapterFriendlyName, 
                 UCHAR*            pMacAddress,
                 size_t            macAddrLength);


#ifdef UNIT_TEST
/* This is helper function for unit test */
/* Sets pointer to list of adapter info used by OsaGetMacAddress */
OSADLL OsaErr
SetPointerToListOfAdapterInfoForTest(const void* pToListOfTestAdapterInfo);

#endif

#endif  /* OSAIPHELPER_H */
