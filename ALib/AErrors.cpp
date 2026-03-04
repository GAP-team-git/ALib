//
//  AErrors.cpp
//  ALib
//
//  Created by Giuseppe Coppini on 27/02/26.
//

#include "AErrors.h"

namespace Alib{

[[nodiscard]] const char* getErrorName(AErrors error) noexcept {
    switch(error) {
        case AErrors::noError: return "noError";
        case AErrors::sourceError: return "sourceError";
        case AErrors::allocationError: return "allocationError";
        case AErrors::invalidOperation: return "invalidOperation";
        case AErrors::rangeError: return "rangeError";
        case AErrors::typeMismatch: return "typeMismatch";
        case AErrors::notInitialized: return "notInitialized";
        case AErrors::unsupportedFeature: return "unsupportedFeature";
        case AErrors::ioError: return "ioError";
        case AErrors::parseError: return "parseError";
        case AErrors::formatError: return "formatError";
        case AErrors::runtimeError: return "runtimeError";
        default: return "unknownError";
    }
}
} // Alib namespace end
