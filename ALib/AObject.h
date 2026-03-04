//
//  GArray.h
//  ALib
//
//  Created by Giuseppe Coppini on 25/02/26.
//  Revised with extended error handling and versioned binary I/O
//
// AObject.h
#ifndef AOBJECT_H
#define AOBJECT_H

#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <sstream>
#include <random>
#include <chrono>
#include <iomanip>
#include <cstdint>
#include <functional>
#include "AErrors.h"
#include "ALIBH.h"

namespace Alib {

class AObject {
public:
    // Constructors / destructor
    AObject() noexcept : p_oid(generateOID()) {}
    virtual ~AObject() = default;

    AObject(const AObject& other) { m_copy(other); }
    AObject(AObject&& other) noexcept { m_move(std::move(other)); }

    // Assignment
    AObject& operator=(const AObject& other) { if (*this != other) m_copy(other); return *this; }
    AObject& operator=(AObject&& other) noexcept { if (*this != other) m_move(std::move(other)); return *this; }

    // ----------------- Polymorphism -----------------
    [[nodiscard("AObject clone")]] virtual std::unique_ptr<AObject> clone() const = 0;
    [[nodiscard("AObject hash")]] virtual std::size_t hash() const { return std::hash<std::string>{}(p_oid); }

    // ----------------- Identity & validity -----------------
    [[nodiscard("AObject Oid")]] const std::string& Oid() const noexcept { return p_oid; }
    virtual std::string getClassName() const { return "AObject"; }

    [[nodiscard("AObject valid")]] bool valid() const noexcept { return p_valid; }
    void setValid(bool valid) noexcept { p_valid = valid; }
    void validate() noexcept { p_valid = true; }
    void invalidate() noexcept { p_valid = false; }

    // ----------------- Error management -----------------
    [[nodiscard("AObject getError")]] AErrors getError() const noexcept { return p_error; }
    [[nodiscard("AObject errorName")]] const char* errorName() const noexcept { return ::Alib::getErrorName(p_error); }
    void setError(AErrors err) noexcept { p_error = err; }
    void resetError() noexcept { p_error = AErrors::noError; }

    // ----------------- Comparison -----------------
    [[nodiscard("AObject operator==")]] bool operator==(const AObject& other) const noexcept { return m_compare(other); }
    [[nodiscard("AObject operator!=")]] bool operator!=(const AObject& other) const noexcept { return !(*this == other); }

    // ----------------- Stream operators -----------------
    friend std::ostream& operator<<(std::ostream& os, const AObject& obj) { return obj.toStream(os); }
    friend std::istream& operator>>(std::istream& is, AObject& obj) { return obj.fromStream(is); }

    virtual std::ostream& toStream(std::ostream& os) const { return m_toStream(os); }
    virtual std::istream& fromStream(std::istream& is) { return m_fromStream(is); }

    // ----------------- Binary I/O -----------------
    [[nodiscard("AObject write")]] virtual bool write(const std::string& fileName) const {
        std::ofstream os(fileName, std::ios::binary);
        if (!os.good()) return false;
        return m_write(os);
    }

    [[nodiscard("AObject read")]] virtual bool read(const std::string& fileName) {
        std::ifstream is(fileName, std::ios::binary);
        if (!is.good()) return false;
        return m_read(is);
    }

    virtual std::ostream& printDescription(std::ostream& os) const { return m_printDescription(os); }

protected:
    // Internal data
    uint32_t p_magic{AOBJ_MAGIC};
    uint32_t p_version{AOBJ_VERSION};
    std::string p_oid;
    bool p_valid{true};
    AErrors p_error{AErrors::noError};

    // ----------------- Utilities -----------------
    static std::string generateOID() {
        static thread_local std::mt19937_64 rng(
            std::chrono::high_resolution_clock::now().time_since_epoch().count()
        );
        std::uniform_int_distribution<uint64_t> dist(0, 9999999999ULL);
        std::ostringstream oss;
        for(int i = 0; i < 3; ++i) {
            oss << std::setw(10) << std::setfill('0') << dist(rng);
            if(i != 2) oss << ".";
        }
        return oss.str();
    }

    // ----------------- Virtual protected methods -----------------
    virtual void m_copy(const AObject& other) {
        p_oid = other.p_oid;
        p_valid = other.p_valid;
        p_error = other.p_error;
    }

    virtual void m_move(AObject&& other) noexcept {
        p_oid = std::move(other.p_oid);
        p_valid = other.p_valid;
        p_error = other.p_error;
    }

    [[nodiscard("AObject m_compare")]] virtual bool m_compare(const AObject& other) const noexcept {
        return p_oid == other.p_oid;
    }

    [[nodiscard("AObject m_isValid")]] virtual bool m_isValid() const noexcept { return p_valid; }

    virtual std::ostream& m_printDescription(std::ostream& os) const {
        os << getClassName() << " [OID=" << p_oid << "]" << std::endl;
        return m_printStatus(os);
    }

    virtual std::ostream& m_printStatus(std::ostream& os) const {
        os << "Valid=" << p_valid << " Error=" << errorName();
        return os;
    }

    // ----------------- Text I/O -----------------
    virtual std::ostream& m_print(std::ostream& os) const {
        os << p_version << std::endl << p_oid << std::endl << p_valid << std::endl << static_cast<int>(p_error);
        return os;
    }

    virtual std::istream& m_parse(std::istream& is) {
        std::string oid;
        bool valid = false;
        int errInt = 0;
        uint32_t version = 0;
        is >> version >> oid >> valid >> errInt;
        if (is.good()) {
            p_version = version;
            p_oid = std::move(oid);
            p_valid = valid;
            p_error = static_cast<AErrors>(errInt);
        }
        return is;
    }

    // ----------------- Binary I/O -----------------
    [[nodiscard("AObject m_write")]] virtual bool m_write(std::ostream& os) const {
        os.write(reinterpret_cast<const char*>(&p_magic), sizeof(p_magic));
        os.write(reinterpret_cast<const char*>(&p_version), sizeof(p_version));

        uint32_t oidLen = static_cast<uint32_t>(p_oid.size());
        os.write(reinterpret_cast<const char*>(&oidLen), sizeof(oidLen));
        if (oidLen > 0) os.write(p_oid.data(), oidLen);

        uint8_t validByte = p_valid ? 1u : 0u;
        os.write(reinterpret_cast<const char*>(&validByte), sizeof(validByte));

        int32_t err = static_cast<int32_t>(p_error);
        os.write(reinterpret_cast<const char*>(&err), sizeof(err));

        return os.good();
    }

    [[nodiscard("AObject m_read")]] virtual bool m_read(std::istream& is) {
        uint32_t magic = 0;
        is.read(reinterpret_cast<char*>(&magic), sizeof(magic));
        if (magic != AOBJ_MAGIC) return false;

        uint32_t version = 0;
        is.read(reinterpret_cast<char*>(&version), sizeof(version));
        if (!is.good()) return false;
        p_version = version;

        uint32_t oidLen = 0;
        is.read(reinterpret_cast<char*>(&oidLen), sizeof(oidLen));
        if (!is.good()) return false;

        std::string oid; oid.resize(oidLen);
        if (oidLen > 0) {
            is.read(oid.data(), oidLen);
            if (!is.good()) return false;
        }

        uint8_t validByte = 0;
        is.read(reinterpret_cast<char*>(&validByte), sizeof(validByte));
        if (!is.good()) return false;

        int32_t err = 0;
        is.read(reinterpret_cast<char*>(&err), sizeof(err));
        if (!is.good()) return false;

        p_oid = std::move(oid);
        p_valid = (validByte != 0);
        p_error = static_cast<AErrors>(err);

        return true;
    }

    virtual std::ostream& m_toStream(std::ostream& os) const { return m_print(os); }
    virtual std::istream& m_fromStream(std::istream& is) { return m_parse(is); }
};

} // namespace Alib

#endif // AOBJECT_H
