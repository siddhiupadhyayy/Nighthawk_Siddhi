/******************************************************************************/
/*                                                                            */
/*   Filename   : OsaDef.h                                                    */
/*                                                                            */
/*   Description: OS abstraction layer definitions                            */
/*                                                                            */
/*   Copyright  : Philips Medical Systems Nederland B.V., 2015.               */
/*                                                                            */
/******************************************************************************/

#ifndef OSADEF_H
#define OSADEF_H

#ifdef WIN32
    #ifdef OSA_INTERNAL
        #define OSADLL __declspec(dllexport)
    #else
        #define OSADLL __declspec(dllimport)
    #endif
    #else
        #define OSADLL
#endif /* WIN32 */

/* TODO: Define speciffics for other OS(s) as no when we add them later */

#endif /* OSADEF_H */
