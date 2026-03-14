//
//  AConfig.h
//  ALib
//
//  Created by Giuseppe Coppini on 11/03/26.
//

#ifndef AConfig_h
#define AConfig_h

// ===== Error handling level =====
//
// 0 = no runtime checks
// 1 = only critical errors
// 2 = full debug checks
//

#ifndef ALIB_ERROR_LEVEL
#define ALIB_ERROR_LEVEL 2 // 0 = off, 1 = solo critici, 2 = full debug
#endif

// ===== Optional fine controls =====

#ifndef ALIB_ENABLE_BOUNDS_CHECK
#define ALIB_ENABLE_BOUNDS_CHECK 1
#endif

#ifndef ALIB_ENABLE_SHAPE_CHECK
#define ALIB_ENABLE_SHAPE_CHECK 1
#endif

#endif /* AConfig_h */
