//
//  AErrors.h
//  ALib
//
//  Created by Giuseppe Coppini on 27/02/26.
//

#ifndef AErrors_h
#define AErrors_h
/// ----------------- Error Handling -----------------
/// AErrors enum with a concise but extensible set of base errors
///
///
///
#include <iostream>

namespace Alib {

enum class AErrors : int32_t {
    noError            = 0,  // No error
    sourceError        = 1,  // File/stream I/O error
    allocationError    = 2,  // Memory or resource allocation failure
    invalidOperation   = 3,  // Operation not allowed
    rangeError         = 4,  // Value out of range or invalid index
    typeMismatch       = 5,  // Incompatible type
    notInitialized     = 6,  // Object not initialized or not ready
    unsupportedFeature = 7,  // Feature not implemented
    ioError            = 8,  // I/O error
    parseError         = 9,  // Parsing failed
    formatError        = 10, // Data does not match expected format
    runtimeError       = 11  // Generic runtime error
};

[[nodiscard]] const char* getErrorName(AErrors error) noexcept;

/// Optional helper structure to attach metadata or descriptions to errors
struct AErrorInfo {
    AErrors code;
    std::string description;
    
    AErrorInfo(AErrors c = AErrors::noError, std::string desc = "")
    : code(c), description(std::move(desc)) {}
};

/// ----------------- Memo for future extensibility -----------------
/// Error handling can be extended by:
/// 1) Adding new error codes to GErrors enum, with descriptive names.
/// 2) Using GErrorInfo (or a derived structure) to associate metadata such as:
///    - Human-readable description
///    - Severity levels (e.g., warning, critical, recoverable)
///    - Categories (e.g., userError, systemError, ioError, parseError)
///    - Recovery hints or suggested actions
/// 3) Overriding error handling in derived classes for context-specific behavior:
///    - Logging
///    - Exception throwing
///    - UI messages or fallback strategies
/// 4) Integrating versioned Text I/O or binary I/O to serialize/deserialize
///    extended error metadata, ensuring backward/forward compatibility.

} // ALib namespace end

#endif // AErrors_h end
