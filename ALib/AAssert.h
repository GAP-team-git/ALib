//
//  AAssert.h
//  ALib
//
//  Created by Giuseppe Coppini on 11/03/26.
//

#ifndef AAssert_h
#define AAssert_h

#include "AConfig.h"
#include "AErrors.h"

namespace Alib {

inline void alib_handle_error(const AErrorInfoEx& e)
{
#if ALIB_ERROR_LEVEL == 0
    
    (void)e;
    
#elif ALIB_ERROR_LEVEL == 1
    
    if(e.severity == AErrorSeverity::critical)
        throw AException(e);
    
#elif ALIB_ERROR_LEVEL == 2
    
    if(e.severity == AErrorSeverity::critical)
        throw AException(e);
    else
        printError(e);
    
#endif
}

#define ALIB_ASSERT(cond, err) \
    do { if(!(cond)) Alib::alib_handle_error(err); } while(0)

#if ALIB_ENABLE_BOUNDS_CHECK

#define ALIB_BOUNDS_ASSERT(cond, err) \
    ALIB_ASSERT(cond, err)

#else

#define ALIB_BOUNDS_ASSERT(cond, err) \
    ((void)0)

#endif

#if ALIB_ENABLE_SHAPE_CHECK

#define ALIB_SHAPE_ASSERT(cond, err) \
    ALIB_ASSERT(cond, err)

#else

#define ALIB_SHAPE_ASSERT(cond, err) \
    ((void)0)

#endif


} // namespace Alib

#endif /* AAssert_h */
