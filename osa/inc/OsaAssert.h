/******************************************************************************/
/*                                                                            */
/*   Filename   : OsaAssert.h                                                 */
/*                                                                            */
/*   Description: OS abstraction layer interface                              */
/*                ASSERT MACRO to be used in OSA code, generic for both OS    */
/*                                                                            */
/*   Copyright  :                                                             */
/*                                                                            */
/******************************************************************************/

#ifndef OSAASSERT_H
#define OSAASSERT_H

#ifdef OSA_INTERNAL

/* ASSERT MACRO to be used in OSA code, generic for both OS */
#define OSA_ASSERT(a)             OsaAssert((a), #a, __FILE__, __LINE__)

/*! \brief  Assert function will test the condition, and stop execution if false.
 *
 *  \param[in]  test     Result of tested condition.
 *  \param[in]  cond     The condition as string
 *  \param[in]  file     The filename of the tested condition
 *  \param[in]  line     The line number of the tested condition
 */
void OsaAssert(BOOL test, CHAR* cond, CHAR* file, UINT32 line);

#endif  /* OSA_INTERNAL */

#endif  /* OSAASSERT_H */

