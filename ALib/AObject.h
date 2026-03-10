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

//
// Class AObject is the base class in Alibe and implements the interface of main classe.
// It is and abstract class with two pure virtual function:
//
// [[nodiscard]] virtual std::unique_ptr<AObject> clone() const = 0;        // polymorphic clone
// virtual size_t type_id() const noexcept = 0;
// [[nodiscard]] virtual bool m_compare(const AObject&) const noexcept = 0; // object comparison for equality,
//                                                                          // should implement id based comparison
// [[nodiscard]] virtual bool m_allEqual(const AObject&) const noexcept = 0;// should implement deep value based comparison
// [[nodiscard]] virtual bool m_allClose(const AObject&, double eps) const noexcept = 0; // should implement deep value based
//                                                                                       // comparison with tolerance
//
// These must be implemented in all derived classes.
//
// Comparison (and hashing) is identity-based (not value-based). Objects with equal OID  return equal.
// This is correct in data-less object. Derived object where value-base equality makes sanse
// need to reimplement content-based comparison, e.g. in AArray and other classes whoso objects are
// characterized by related data values.
//
// Other functions that should be always reimplemented include:
// virtual std::string getClassName() const { return "AObject"; }
//
//
class AObject {
public:
    // Constructors / destructor
    AObject() noexcept : p_oid(generateOID()) {}
    virtual ~AObject() = default;

    AObject(const AObject& other) { m_copy(other); }
    AObject(AObject&& other) noexcept { m_move(std::move(other)); }

    
    virtual size_t type_id() const noexcept = 0;

    [[nodiscard]] bool operator==(const AObject& other) const noexcept {
        if (type_id() != other.type_id()) return false;
        return m_compare(other);
    }
    
    // Assignment
    AObject& operator=(const AObject& other) { if (*this != other) m_copy(other); return *this; }
    AObject& operator=(AObject&& other) noexcept { if (*this != other) m_move(std::move(other)); return *this; }

    // ----------------- Polymorphism -----------------
    [[nodiscard]] virtual std::unique_ptr<AObject> clone() const = 0;
    [[nodiscard]] virtual std::size_t hash() const { return std::hash<std::string>{}(p_oid); }
    
    // -------------------
    // String interface (debug / inspection)
    // -------------------
    [[nodiscard]] virtual std::string str() const {
        std::string s = getClassName()+"(Version["+ std::to_string(p_version) + "];OID["+p_oid+"];valid[" + (p_valid?"true":"false") +"];AError["+ errorName() + "])";
    
        return s;
    }

    // ----------------- Identity & validity -----------------
    [[nodiscard]] const std::string& Oid() const noexcept { return p_oid; }
    virtual std::string getClassName() const noexcept { return "AObject"; }

    [[nodiscard]] bool valid() const noexcept { return p_valid; }
    void setValid(bool valid) noexcept { p_valid = valid; }
    void validate() noexcept { p_valid = true; }     // mnemonic alias
    void invalidate() noexcept { p_valid = false; }  // mnemonic alias

    // ----------------- Error management -----------------
    [[nodiscard]] AErrors getError() const noexcept { return p_error; }
    [[nodiscard]] const char* errorName() const noexcept { return ::Alib::getErrorName(p_error); }
    void setError(AErrors err) noexcept { p_error = err; }
    void resetError() noexcept { p_error = AErrors::noError; }

    
    // ----------------- Stream operators -----------------
    friend std::ostream& operator<<(std::ostream& os, const AObject& obj) { return obj.toStream(os); }
    friend std::istream& operator>>(std::istream& is, AObject& obj) { return obj.fromStream(is); }

    virtual std::ostream& toStream(std::ostream& os) const { return m_toStream(os); }
    virtual std::istream& fromStream(std::istream& is) { return m_fromStream(is); }

    // ----------------- Binary I/O -----------------
    [[nodiscard]] virtual bool write(const std::string& fileName) const {
        std::ofstream os(fileName, std::ios::binary);
        if (!os.good()) return false;
        return m_write(os);
    }

    [[nodiscard]] virtual bool read(const std::string& fileName) {
        std::ifstream is(fileName, std::ios::binary);
        if (!is.good()) return false;
        return m_read(is);
    }
    
    // ----- Description adn Status -----
    virtual std::ostream& printDescription(std::ostream& os) const { return m_printDescription(os); }
    virtual std::ostream& printStatus(std::ostream& os) const { return m_printStatus(os); }

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
    
    [[nodiscard]] virtual bool m_compare(const AObject&) const noexcept = 0;
    [[nodiscard]] virtual bool m_allEqual(const AObject&) const noexcept = 0;
    [[nodiscard]] virtual bool m_allClose(const AObject&, double eps) const noexcept = 0;
    
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

    

    [[nodiscard]] virtual bool m_isValid() const noexcept { return p_valid; }

    virtual std::ostream& m_printDescription(std::ostream& os) const {
        os << getClassName()+"{" << std::endl;
        m_print(os);
        os << std::endl;
        m_printStatus(os);
        os << std::endl <<"}";
        return os;
    }

    virtual std::ostream& m_printStatus(std::ostream& os) const {
        os << "Valid: " << p_valid << std::endl << "Error: " << errorName();
        return os;
    }

    // ----------------- Text I/O -----------------
    virtual std::ostream& m_print(std::ostream& os) const {
        os << "Version: " << p_version<< std::endl << "Oid: "<< p_oid;
        return os;
    }

    virtual std::istream& m_parse(std::istream& is) {
        std::string oid;
        uint32_t version = 0;
        is >> version >> oid ;
        if (is.good()) {
            p_version = version;
            p_oid = std::move(oid);
        }
        return is;
    }

    // ----------------- Binary I/O -----------------
    // todo: manage endianity
    [[nodiscard]] virtual bool m_write(std::ostream& os) const {
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

    [[nodiscard]] virtual bool m_read(std::istream& is) {
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

//
// convenience Class for AObject debugging
//
class RAObject: public AObject{
public:
    [[nodiscard]] virtual std::unique_ptr<AObject> clone() const override {return std::make_unique<RAObject>(*this);}
    virtual size_t type_id() const noexcept override { return type_id_of<RAObject>();}
    
    virtual std::string getClassName() const noexcept override { return "RAObject"; }
private:
    [[nodiscard]] virtual bool m_compare(const AObject& obj) const noexcept override {
        if(this->Oid() == obj.Oid())return true;
        return false;}
    [[nodiscard]] virtual bool m_allEqual(const AObject& obj) const noexcept override{
        return m_compare(obj);
    }
    [[nodiscard]] virtual bool m_allClose(const AObject& obj, double eps) const noexcept override {
        return m_compare(obj);
    }
};


} // namespace Alib

#endif // AOBJECT_H
