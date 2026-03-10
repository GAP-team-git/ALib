//
//  AErrors.cpp
//  ALib
//
//  Created by Giuseppe Coppini on 10/03/26.
//

#include "AErrors.h"

namespace Alib {

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

[[nodiscard]] const char* getCategoryName(AErrorCategory cat) noexcept {
    switch(cat) {
        case AErrorCategory::general: return "general";
        case AErrorCategory::array:   return "array";
        case AErrorCategory::shape:   return "shape";
        case AErrorCategory::tensor:  return "tensor";
        case AErrorCategory::io:      return "io";
        case AErrorCategory::parse:   return "parse";
        default: return "unknownCategory";
    }
}

} // namespace Alib
