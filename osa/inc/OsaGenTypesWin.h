/****************************************************************************/
/*                                                                          */
/*   Filename   : OsaGenTypesWin.h                                          */
/*                                                                          */
/*   Description: OS abstraction layer interface                            */
/*                 OS independent type definitions for WIN32                */
/*                                                                          */
/*   Copyright  :                                                           */
/*                                                                          */
/****************************************************************************/

#ifndef OSAGENTYPESWIN_H
#define OSAGENTYPESWIN_H

#ifndef OSAGENTYPESOS
    #error This header may only be included from OsaGenTypes.h
#endif

/* _MSC_VER 1600 is VS2010 */
#if _MSC_VER >= 1600
    #include <stdint.h>
#endif

#ifdef OSA_INTERNAL
    #include <sys/stat.h>   /* for original struct stat */
#endif

#ifdef OSA_INTERNAL
    #define OSADLL __declspec(dllexport)
#else
    #define OSADLL __declspec(dllimport)
#endif

#if (!defined __cplusplus) && (_MSC_VER <= 1700) /* 1700 is Visual Studio 2012 */
/* MS does not (fully) implement C99 */
/* inline is provided by __inline    */
#define inline __inline
#endif

#ifndef OSA_INTERNAL
    #ifndef UNREFERENCED_PARAMETER
        #define UNREFERENCED_PARAMETER(P) (void)(P)
    #endif /* !UNREFERENCED_PARAMETER */
#endif /* (!defined OSA_INTERNAL) */

#ifndef OSA_INTERNAL
    /* copied from BaseTsd.h */
    #ifndef _BASETSD_H_
        /*! 8 bits signed integer type */
        typedef signed char         INT8;
        /*! 16 bits signed integer type */
        typedef signed short        INT16;
        /*! 32 bits signed integer type */
        typedef signed int          INT32;
        /*! 64 bits signed integer type */
        typedef signed __int64      INT64;

        /*! 8 bits unsigned integer type */
        typedef unsigned char       UINT8;
        /*! 16 bits unsigned integer type */
        typedef unsigned short      UINT16;
        /*! 32 bits unsigned integer type */
        typedef unsigned int        UINT32;
        /*! 64 bits unsigned integer type */
        typedef unsigned __int64    UINT64;
    #endif  /* !_BASETSD_H_ */

    /* copied from WinDef.h */
    #ifndef _WINDEF_
        /*! Boolean type */
        typedef int                 BOOL;
        /*! Unsigned char type */
        typedef unsigned char       UCHAR;
    #endif  /* !_WINDEF_ */

    /* copied from crtdefs.h */
    #ifndef _WCHAR_T_DEFINED
        typedef unsigned short wchar_t;
        #define _WCHAR_T_DEFINED
    #endif  /* !_WCHAR_T_DEFINED */
    /* copied from WinNT.h */
    #ifdef VOID
        #undef VOID
    #endif
    /*! Void type */
    #define                         VOID void
    #ifndef _WINNT_
        /*! Character type */
        typedef char                CHAR;
        /*! Wide character type */
        typedef wchar_t             WCHAR;
        /*! BOOLEAN type */
        typedef unsigned char       BOOLEAN;
    #endif  /* !_WINNT_ */
    /* copied from SourceAnnotations.h */
    #ifndef _SIZE_T_DEFINED
        typedef unsigned int        size_t;
        #define _SIZE_T_DEFINED
    #endif  /* !_SIZE_T_DEFINED */
    
    /* copied from sys/types.h for struct stat */
    #ifndef _DEV_T_DEFINED
        typedef unsigned int _dev_t;    /* device code */
        #define _DEV_T_DEFINED
    #endif
    #ifndef _INO_T_DEFINED
        typedef unsigned short _ino_t;  /* i-node number (not used on DOS) */
        #define _INO_T_DEFINED
    #endif
    #ifndef _OFF_T_DEFINED
        typedef long _off_t;            /* file offset value */
        #define _OFF_T_DEFINED
    #endif

    /* copied from sys/stat.h */
    #ifndef _STAT_DEFINED
        struct stat
        {
            _dev_t     st_dev;
            _ino_t     st_ino;
            unsigned short st_mode;
            short      st_nlink;
            short      st_uid;
            short      st_gid;
            _dev_t     st_rdev;
            _off_t     st_size;
            time_t st_atime;
            time_t st_mtime;
            time_t st_ctime;
        };
        #define _STAT_DEFINED
    #endif  /* !_STAT_DEFINED */

    /* SIZE_MAX can be defined in other headers */
    /* if defined undefine */
    #ifdef SIZE_MAX
        #undef SIZE_MAX
    #endif  /* SIZE_MAX */
    /* copied from limits.h */
    #define SIZE_MAX        (0xffffffff)
#endif  /* !OSA_INTERNAL */

#if (_MSC_VER < 1600) /* _MSC_VER 1600 is VS2010 */
    /* for internal and external use */
    /* definition of minimum and maximum values */
    #define INT8_MIN        ((INT8)0x80)
    #define INT8_MAX        ((INT8)0x7F)
    #define INT16_MIN       ((INT16)0x8000)
    #define INT16_MAX       ((INT16)0x7FFF)
    #define INT32_MIN       ((INT32)0x80000000)
    #define INT32_MAX       ((INT32)0x7FFFFFFF)
    #define INT64_MIN       ((INT64)0x8000000000000000)
    #define INT64_MAX       ((INT64)0x7FFFFFFFFFFFFFFF)
    /* no definition of min for unsigned types, this is alsways 0 */
    #define UINT8_MAX       ((UINT8)0xFF)
    #define UINT16_MAX      ((UINT16)0xFFFF)
    #define UINT32_MAX      ((UINT32)0xFFFFFFFF)
    #define UINT64_MAX      ((UINT64)0xFFFFFFFFFFFFFFFF)
#endif

#endif  /* OSAGENTYPESWIN_H */
