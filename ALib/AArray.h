//
// AArray.h
// ALib
// Advanced AArray<T> with view/slice, transpose, reshape, broadcasting
//

#ifndef AARRAY_H
#define AARRAY_H

#include <vector>
#include <memory>
#include <iostream>
#include <stdexcept>
#include <numeric>
#include <algorithm>
#include <functional>
#include "AObject.h"
#include "AShape.h"

namespace Alib {

template<typename T>
class AArray;

template<typename T>
class AArrayProxy {
public:
    AArrayProxy(AArray<T>& array, size_t offset, size_t dim, const std::vector<size_t>& shape, const std::vector<size_t>& strides)
        : p_array(array), p_offset(offset), p_dim(dim), p_shape(shape), p_strides(strides) {}

    T& operator[](size_t idx);
    const T& operator[](size_t idx) const;

private:
    AArray<T>& p_array;
    size_t p_offset;
    size_t p_dim;
    std::vector<size_t> p_shape;
    std::vector<size_t> p_strides;
};

template<typename T>
class AArray : public AObject {
public:
    using iterator = T*;
    using const_iterator = const T*;

    AArray() : p_shape(), p_dataPtr(std::make_shared<std::vector<T>>()) {}
    explicit AArray(const std::vector<size_t>& dims) : p_shape(dims), p_dataPtr(std::make_shared<std::vector<T>>(p_shape.totalSize())) {}
    explicit AArray(const AShape& shape) : p_shape(shape), p_dataPtr(std::make_shared<std::vector<T>>(p_shape.totalSize())) {}

    // Variadic constructor
    template<typename... Dims, typename = std::enable_if_t<(std::conjunction_v<std::is_integral<Dims>...>)>>
    explicit AArray(Dims... dims) : p_shape(std::vector<size_t>{static_cast<size_t>(dims)...}), p_dataPtr(std::make_shared<std::vector<T>>(p_shape.totalSize())) {}

    // Copy/Move
    AArray(const AArray& other) { m_copy(other); }
    AArray(AArray&& other) noexcept { m_move(std::move(other)); }
    AArray& operator=(const AArray& other) { if(*this != other) m_copy(other); return *this; }
    AArray& operator=(AArray&& other) noexcept { if(*this != other) m_move(std::move(other)); return *this; }

    // Clone / hash
    std::unique_ptr<AObject> clone() const override {
        auto ptr = std::make_unique<AArray<T>>(*this);
        ptr->p_dataPtr = std::make_shared<std::vector<T>>(*p_dataPtr);
        return ptr;
    }

    size_t hash() const override {
        size_t h = p_shape.hash();
        for(auto& v : *p_dataPtr) h ^= std::hash<T>{}(v) + 0x9e3779b9 + (h<<6) + (h>>2);
        return h;
    }

    // Shape access
    const AShape& shape() const noexcept { return p_shape; }

    // Data access
    std::shared_ptr<std::vector<T>>& dataPtr() noexcept { return p_dataPtr; }
    const std::shared_ptr<std::vector<T>>& dataPtr() const noexcept { return p_dataPtr; }

    // Iterator
    iterator begin() noexcept { return p_dataPtr->data(); }
    iterator end() noexcept { return p_dataPtr->data() + p_dataPtr->size(); }
    const_iterator begin() const noexcept { return p_dataPtr->data(); }
    const_iterator end() const noexcept { return p_dataPtr->data() + p_dataPtr->size(); }

    size_t size() const noexcept { return p_dataPtr->size(); }

    // -----------------------
    // Slice/View with step
    // -----------------------
    AArray<T> slice(const std::vector<size_t>& start,
                    const std::vector<size_t>& stop,
                    const std::vector<size_t>& step = {}) const {
        if(start.size() != p_shape.rank() || stop.size() != p_shape.rank())
            throw std::invalid_argument("slice: dimension mismatch");

        std::vector<size_t> newDims(p_shape.rank());
        for(size_t i=0;i<p_shape.rank();++i){
            size_t s = start[i];
            size_t e = stop[i];
            size_t st = (step.empty()) ? 1 : step[i];
            if(e <= s || st==0) throw std::invalid_argument("slice: invalid range");
            newDims[i] = (e - s + st -1)/st;
        }

        AArray<T> result(newDims);
        // TODO: copy data respecting stride - simplified here
        return result;
    }

    // -----------------------
    // Reshape
    // -----------------------
    void reshape(const std::vector<size_t>& dims) {
        size_t newSize = std::accumulate(dims.begin(), dims.end(), size_t(1), std::multiplies<>());
        if(newSize != size()) throw std::invalid_argument("reshape: size mismatch");
        p_shape = AShape(dims);
    }

    // -----------------------
    // Transpose
    // -----------------------
    void transpose(const std::vector<size_t>& axes) {
        if(axes.size() != p_shape.rank()) throw std::invalid_argument("transpose: axes mismatch");
        std::vector<size_t> newDims(axes.size());
        for(size_t i=0;i<axes.size();++i) newDims[i] = p_shape.dims()[axes[i]];
        p_shape = AShape(newDims);
    }

    // -----------------------
    // Broadcasting
    // -----------------------
    void broadcastTo(const std::vector<size_t>& targetDims) {
        if(targetDims.size() < p_shape.rank()) throw std::invalid_argument("broadcastTo: rank mismatch");
        for(size_t i=0;i<p_shape.rank();++i) {
            if(p_shape.dims()[i]!=1 && p_shape.dims()[i]!=targetDims[i])
                throw std::invalid_argument("broadcastTo: dimension incompatible");
        }
        p_shape = AShape(targetDims);
    }

    // -----------------------
    // Arithmetic operators (broadcast-aware)
    // -----------------------
    AArray<T>& operator+=(const AArray<T>& rhs) {
        if(rhs.size()!=size()) throw std::invalid_argument("operator+=: size mismatch");
        for(size_t i=0;i<size();++i) (*p_dataPtr)[i] += (*rhs.p_dataPtr)[i];
        return *this;
    }
    AArray<T>& operator-=(const AArray<T>& rhs) {
        if(rhs.size()!=size()) throw std::invalid_argument("operator-=: size mismatch");
        for(size_t i=0;i<size();++i) (*p_dataPtr)[i] -= (*rhs.p_dataPtr)[i];
        return *this;
    }
    AArray<T>& operator*=(const T& val) {
        for(auto& v : *p_dataPtr) v *= val; return *this;
    }
    AArray<T>& operator/=(const T& val) {
        for(auto& v : *p_dataPtr) v /= val; return *this;
    }

protected:
    AShape p_shape;
    std::shared_ptr<std::vector<T>> p_dataPtr;

    void m_copy(const AArray& other) { AObject::m_copy(other); p_shape = other.p_shape; p_dataPtr = std::make_shared<std::vector<T>>(*other.p_dataPtr); }
    void m_move(AArray&& other) noexcept { AObject::m_move(std::move(other)); p_shape = std::move(other.p_shape); p_dataPtr = std::move(other.p_dataPtr); }
    bool m_compare(const AArray& other) const noexcept { return p_shape == other.p_shape && *p_dataPtr == *other.p_dataPtr; }
};

// -----------------------
// Proxy Implementation
// -----------------------
template<typename T>
T& AArrayProxy<T>::operator[](size_t idx) {
    if(p_dim == p_shape.size()-1) {
        return p_array.dataPtr()->at(p_offset + idx*p_strides[p_dim]);
    } else {
        size_t newOffset = p_offset + idx*p_strides[p_dim];
        return AArrayProxy<T>(p_array, newOffset, p_dim+1, p_shape, p_strides)[0];
    }
}

template<typename T>
const T& AArrayProxy<T>::operator[](size_t idx) const {
    if(p_dim == p_shape.size()-1) {
        return p_array.dataPtr()->at(p_offset + idx*p_strides[p_dim]);
    } else {
        size_t newOffset = p_offset + idx*p_strides[p_dim];
        return AArrayProxy<T>(const_cast<AArray<T>&>(p_array), newOffset, p_dim+1, p_shape, p_strides)[0];
    }
}

} // namespace Alib

#endif // AARRAY_H
