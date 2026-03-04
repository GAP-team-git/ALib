#include "___VARIABLE_ClassName___.h"

namespace Alib {

// ----------------- Constructors / Destructor -----------------
___VARIABLE_ClassName:identifier___::___VARIABLE_ClassName:identifier___() 
    : p_oid(generateOID()) {}

___VARIABLE_ClassName:identifier___::___VARIABLE_ClassName:identifier___(const ___VARIABLE_ClassName:identifier___& other) { m_copy(other); }
___VARIABLE_ClassName:identifier___::___VARIABLE_ClassName:identifier___(___VARIABLE_ClassName:identifier___&& other) noexcept { m_move(std::move(other)); }

// Assignment
___VARIABLE_ClassName:identifier___& ___VARIABLE_ClassName:identifier___::operator=(const ___VARIABLE_ClassName:identifier___& other) { 
    if (*this != other) m_copy(other); 
    return *this; 
}
___VARIABLE_ClassName:identifier___& ___VARIABLE_ClassName:identifier___::operator=(___VARIABLE_ClassName:identifier___&& other) noexcept { 
    if (*this != other) m_move(std::move(other)); 
    return *this; 
}

// Clone
std::unique_ptr<___VARIABLE_ClassName:identifier___> ___VARIABLE_ClassName:identifier___::clone() const { return m_clone(); }

// Identity / validity
const std::string& ___VARIABLE_ClassName:identifier___::Oid() const noexcept { return p_oid; }
std::string ___VARIABLE_ClassName:identifier___::getClassName() const { return "___VARIABLE_ClassName:identifier___"; }
bool ___VARIABLE_ClassName:identifier___::valid() const noexcept { return p_valid; }
void ___VARIABLE_ClassName:identifier___::setValid(bool valid) { p_valid = valid; }
void ___VARIABLE_ClassName:identifier___::validate() { p_valid = true; }
void ___VARIABLE_ClassName:identifier___::invalidate() { p_valid = false; }

// Error
AErrors ___VARIABLE_ClassName:identifier___::getError() const { return p_error; }
const char* ___VARIABLE_ClassName:identifier___::getErrorName() const {
    switch(p_error) {
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
void ___VARIABLE_ClassName:identifier___::setError(AErrors err) { p_error = err; }
void ___VARIABLE_ClassName:identifier___::resetError() { p_error = AErrors::noError; }

// Comparison
bool ___VARIABLE_ClassName:identifier___::operator==(const ___VARIABLE_ClassName:identifier___& other) const { return m_compare(other); }
bool ___VARIABLE_ClassName:identifier___::operator!=(const ___VARIABLE_ClassName:identifier___& other) const { return !(*this == other); }

// Stream
std::ostream& operator<<(std::ostream& os, const ___VARIABLE_ClassName:identifier___& obj) { return obj.toStream(os); }
std::istream& operator>>(std::istream& is, ___VARIABLE_ClassName:identifier___& obj) { return obj.fromStream(is); }

std::ostream& ___VARIABLE_ClassName:identifier___::toStream(std::ostream& os) const { return m_toStream(os); }
std::istream& ___VARIABLE_ClassName:identifier___::fromStream(std::istream& is) { return m_fromStream(is); }

// Binary I/O
bool ___VARIABLE_ClassName:identifier___::write(const std::string& fileName) const {
    std::ofstream os(fileName,std::ios::binary);
    if(!os.good()) return false;
    m_write(os);
    return os.good();
}
bool ___VARIABLE_ClassName:identifier___::read(const std::string& fileName) {
    std::ifstream is(fileName,std::ios::binary);
    if(!is.good()) return false;
    m_read(is);
    return is.good();
}

// Description
std::ostream& ___VARIABLE_ClassName:identifier___::printDescription(std::ostream& os) const { return m_printDescription(os); }

// ----------------- Static helper -----------------
std::string ___VARIABLE_ClassName:identifier___::generateOID() {
    static thread_local std::mt19937_64 rng(
        std::chrono::high_resolution_clock::now().time_since_epoch().count()
    );
    std::uniform_int_distribution<uint64_t> dist(0, 9999999999ULL);
    std::ostringstream oss;
    for(int i=0;i<3;++i){
        oss << std::setw(10)<<std::setfill('0')<<dist(rng);
        if(i!=2) oss << ".";
    }
    return oss.str();
}

} // namespace Alib