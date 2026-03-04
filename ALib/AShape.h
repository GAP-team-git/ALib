//
//  AShape.h
//  ALib
//
//  Created by Giuseppe Coppini on 03/03/26.
//

#ifndef ASHAPE_H
#define ASHAPE_H

#include <vector>
#include <numeric>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <functional>
#include "AObject.h"

namespace Alib {

class AShape : public AObject {
public:
    AShape() = default;
    explicit AShape(const std::vector<size_t>& dims) : p_dims(dims) { computeStrides(); }
    
    // Variadic constructor
    template<typename... Dims,
             typename = std::enable_if_t<(std::conjunction_v<std::is_integral<Dims>...>)>>
    explicit AShape(Dims... dims) : p_dims({static_cast<size_t>(dims)...}) { computeStrides(); }

    const std::vector<size_t>& dims() const noexcept { return p_dims; }
    const std::vector<size_t>& strides() const noexcept { return p_strides; }

    size_t rank() const noexcept { return p_dims.size(); }
    size_t totalSize() const noexcept {
        return rank() == 0 ? 0 : std::accumulate(p_dims.begin(), p_dims.end(), size_t(1), std::multiplies<>());
    }

    bool operator==(const AShape& other) const noexcept { return p_dims == other.p_dims; }
    bool operator!=(const AShape& other) const noexcept { return !(*this == other); }

    // Clone and hash
    std::unique_ptr<AObject> clone() const override { return std::make_unique<AShape>(*this); }
    size_t hash() const override {
        size_t h = 0;
        for(auto d : p_dims) h ^= std::hash<size_t>{}(d) + 0x9e3779b9 + (h<<6) + (h>>2);
        return h;
    }

    // Stream operators
    std::ostream& toStream(std::ostream& os) const override {
        AObject::toStream(os);
        os << p_dims.size() << " ";
        for(auto d : p_dims) os << d << " ";
        return os;
    }

    std::istream& fromStream(std::istream& is) override {
        AObject::fromStream(is);
        size_t n;
        is >> n;
        p_dims.resize(n);
        for(size_t i=0;i<n;++i) is >> p_dims[i];
        computeStrides();
        return is;
    }

private:
    std::vector<size_t> p_dims;
    std::vector<size_t> p_strides;

    void computeStrides() {
        p_strides.resize(p_dims.size());
        if(p_dims.empty()) return;
        p_strides.back() = 1;
        for(int i = int(p_dims.size()) - 2; i >= 0; --i)
            p_strides[i] = p_strides[i+1] * p_dims[i+1];
    }

    void m_copy(const AShape& other) { AObject::m_copy(other); p_dims = other.p_dims; p_strides = other.p_strides; }
    void m_move(AShape&& other) noexcept { AObject::m_move(std::move(other)); p_dims = std::move(other.p_dims); p_strides = std::move(other.p_strides); }
    bool m_compare(const AShape& other) const noexcept { return p_dims == other.p_dims; }
};

} // namespace Alib
#endif // ASHAPE_H
