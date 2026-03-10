//
//  AErrors.h
//  ALib
//
//  Created by Giuseppe Coppini on 10/03/26.
//

#ifndef AErrors_h
#define AErrors_h

#include <cstdint>
#include <string>
#include <iostream>
#include <exception>

namespace Alib {

/// ----------------- Base Error Codes -----------------
enum class AErrors : int32_t {
    noError            = 0,
    sourceError        = 1,
    allocationError    = 2,
    invalidOperation   = 3,
    rangeError         = 4,
    typeMismatch       = 5,
    notInitialized     = 6,
    unsupportedFeature = 7,
    ioError            = 8,
    parseError         = 9,
    formatError        = 10,
    runtimeError       = 11
};

/// ----------------- Error Categories -----------------
enum class AErrorCategory : uint16_t {
    general = 0x0000,
    array   = 0x1000,
    shape   = 0x2000,
    tensor  = 0x3000,
    io      = 0x4000,
    parse   = 0x5000
};

/// ----------------- Error Severity -----------------
enum class AErrorSeverity : uint8_t {
    warning,
    recoverable,
    critical
};

/// ----------------- Extended Error Info -----------------
struct AErrorInfoEx {
    AErrors code;
    AErrorCategory category;
    AErrorSeverity severity;
    std::string description;
    std::string suggestion;

    AErrorInfoEx(AErrors c = AErrors::noError,
                 AErrorCategory cat = AErrorCategory::general,
                 AErrorSeverity sev = AErrorSeverity::recoverable,
                 std::string desc = "",
                 std::string sugg = "")
        : code(c), category(cat), severity(sev),
          description(std::move(desc)), suggestion(std::move(sugg)) {}
};

/// ----------------- Exception -----------------
class AException : public std::exception {
    AErrorInfoEx errorInfo;
public:
    explicit AException(AErrorInfoEx info) : errorInfo(std::move(info)) {}
    [[nodiscard]] const char* what() const noexcept override { return errorInfo.description.c_str(); }
    [[nodiscard]] const AErrorInfoEx& info() const noexcept { return errorInfo; }
};

/// ----------------- Module-specific aliases -----------------
namespace ArrayErrors {
    constexpr AErrors indexOutOfBounds = AErrors::rangeError;
    constexpr AErrors emptyArray       = AErrors::invalidOperation;
    constexpr AErrors shapeMismatch    = AErrors::typeMismatch;
}

namespace ShapeErrors {
    constexpr AErrors invalidDim       = AErrors::rangeError;
    constexpr AErrors uninitialized    = AErrors::notInitialized;
}

namespace TensorErrors {
    constexpr AErrors iteratorError    = AErrors::runtimeError;
    constexpr AErrors sizeMismatch     = AErrors::rangeError;
}

/// ----------------- Helper functions -----------------
[[nodiscard]] const char* getErrorName(AErrors error) noexcept;
[[nodiscard]] const char* getCategoryName(AErrorCategory cat) noexcept;

inline void printError(const AErrorInfoEx& e) {
    std::cerr << "[" << getCategoryName(e.category) << "] "
              << getErrorName(e.code) << ": " << e.description;
    if(!e.suggestion.empty()) std::cerr << " (Hint: " << e.suggestion << ")";
    std::cerr << "\n";
}

/// ----------------- Hybrid error handler -----------------
inline void handleError(const AErrorInfoEx& e) {
    if(e.severity == AErrorSeverity::critical) {
        throw AException(e); // blocca il flusso
    } else {
        printError(e);       // logga ma continua
    }
}

} // namespace Alib

#endif // AErrors_h
