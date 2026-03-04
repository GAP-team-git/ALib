#ifndef ___VARIABLE_ClassName:identifier___h
#define ___VARIABLE_ClassName:identifier___h

#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <sstream>
#include <random>
#include <chrono>
#include <iomanip>
#include <cstdint>
#include "AObject.h"

namespace Alib {

class ___VARIABLE_ClassName:identifier___ {
public:
    // Constructors / Destructor
    ___VARIABLE_ClassName:identifier___();
    virtual ~___VARIABLE_ClassName:identifier___() = default;
    ___VARIABLE_ClassName:identifier___(const ___VARIABLE_ClassName:identifier___& other);
    ___VARIABLE_ClassName:identifier___(___VARIABLE_ClassName:identifier___&& other) noexcept;

    // Assignment
    ___VARIABLE_ClassName:identifier___& operator=(const ___VARIABLE_ClassName:identifier___& other);
    ___VARIABLE_ClassName:identifier___& operator=(___VARIABLE_ClassName:identifier___&& other) noexcept;

    // Clone
    virtual std::unique_ptr<___VARIABLE_ClassName:identifier___> clone() const;

    // Identity / validity
    const std::string& Oid() const noexcept;
    virtual std::string getClassName() const;

    bool valid() const noexcept;
    void setValid(bool valid);
    void validate();
    void invalidate();

    // Error management
    AErrors getError() const;
    const char* getErrorName() const;
    void setError(AErrors err);
    void resetError();

    // Comparison
    bool operator==(const ___VARIABLE_ClassName:identifier___& other) const;
    bool operator!=(const ___VARIABLE_ClassName:identifier___& other) const;

    // Stream operators
    friend std::ostream& operator<<(std::ostream& os, const ___VARIABLE_ClassName:identifier___& obj);
    friend std::istream& operator>>(std::istream& is, ___VARIABLE_ClassName:identifier___& obj);

    virtual std::ostream& toStream(std::ostream& os) const;
    virtual std::istream& fromStream(std::istream& is);

    // Binary I/O
    [[nodiscard("AObject write")]] virtual bool write(const std::string& fileName) const;
    [[nodiscard("AObject read")]] virtual bool read(const std::string& fileName);

    virtual std::ostream& printDescription(std::ostream& outputStream) const;

protected:
    // Internal data
    uint32_t p_magic{AOBJ_MAGIC};
    uint32_t p_version{AOBJ_VERSION};
    std::string p_oid;
    bool p_valid{true};
    AErrors p_error{AErrors::noError};

    static std::string generateOID();

    // Virtual methods for inheritance
    virtual void m_copy(const ___VARIABLE_ClassName:identifier___& other);
    virtual void m_move(___VARIABLE_ClassName:identifier___&& other);

    virtual std::unique_ptr<___VARIABLE_ClassName:identifier___> m_clone() const;
    virtual bool m_compare(const ___VARIABLE_ClassName:identifier___& other) const;
    virtual bool m_isValid() const;

    virtual std::ostream& m_printDescription(std::ostream& os) const;
    virtual std::ostream& m_printStatus(std::ostream& os) const;

    virtual std::ostream& m_print(std::ostream& os) const;
    virtual std::istream& m_parse(std::istream& is);

    virtual bool m_write(std::ostream& os) const;
    virtual bool m_read(std::istream& is);

    virtual std::ostream& m_toStream(std::ostream& os) const;
    virtual std::istream& m_fromStream(std::istream& is);
};

} // namespace Alib

#endif // ___VARIABLE_ClassName:identifier___h
