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
#include <initializer_list>
#include <numeric>
#include <algorithm>
#include <stdexcept>
#include "AObject.h"
#include "AShape.h"

namespace Alib {

template<typename T> class AArray;

template<typename T>
class AArrayProxy {
    // Puntatore non-owning all'array principale
    AArray<T>* arrayPtr;           // equivale al vecchio p_array
    std::vector<size_t> indices;   // equivale al vecchio p_index

public:
    AArrayProxy(AArray<T>* arr, std::vector<size_t> idx)
        : arrayPtr(arr), indices(std::move(idx)) {}

    // Accesso a livello successivo
    AArrayProxy<T> operator[](size_t i) {
        std::vector<size_t> next = indices;
        next.push_back(i);
        return AArrayProxy<T>(arrayPtr, std::move(next));
    }

    // Conversione a valore o riferimento
    operator T() const {
        if(indices.size() != arrayPtr->shape().rank())
            throw std::runtime_error("Incomplete index in AArrayProxy");
        return arrayPtr->at(indices);
    }

    operator T&() {
        if(indices.size() != arrayPtr->shape().rank())
            throw std::runtime_error("Incomplete index in AArrayProxy");
        return arrayPtr->at(indices);
    }

    operator const T&() const {
        if(indices.size() != arrayPtr->shape().rank())
            throw std::runtime_error("Incomplete index in AArrayProxy");
        return arrayPtr->at(indices);
    }

    // Assegnazione da valore scalare
    AArrayProxy<T>& operator=(const T& value) {
        (*this) = value;  // usa conversion operator a T&
        return *this;
    }
};

template<typename T>
class AArray : public AObject {
public:
    using value_type=T;
    using iterator=T*;
    using const_iterator=const T*;

    // Constructors
    AArray()=default;
    explicit AArray(const std::vector<size_t>& dims)
        : p_shape(dims), p_dataPtr(std::make_shared<std::vector<T>>(p_shape.totalSize())) {}
    explicit AArray(const AShape& shape)
        : p_shape(shape), p_dataPtr(std::make_shared<std::vector<T>>(p_shape.totalSize())) {}
    template<typename... Dims, typename=std::enable_if_t<(std::conjunction_v<std::is_integral<Dims>...>)>>
    explicit AArray(Dims... dims)
        : p_shape(std::vector<size_t>{static_cast<size_t>(dims)...}),
          p_dataPtr(std::make_shared<std::vector<T>>(p_shape.totalSize())) {}
    explicit AArray(const std::initializer_list<size_t>& dims)
        : p_shape(std::vector<size_t>(dims)),
          p_dataPtr(std::make_shared<std::vector<T>>(p_shape.totalSize())) {}

    // Copy/Move
    AArray(const AArray& other){ m_copy(other); }
    AArray(AArray&& other) noexcept { m_move(std::move(other)); }
    AArray& operator=(const AArray& other){ if(this!=&other) m_copy(other); return *this; }
    AArray& operator=(AArray&& other) noexcept{ if(this!=&other) m_move(std::move(other)); return *this; }

    // Clone/Hash
    std::unique_ptr<AObject> clone() const override { return std::make_unique<AArray<T>>(*this); }
    size_t hash() const override {
        size_t h=0;
        for(const auto& v:*p_dataPtr) h ^= std::hash<T>{}(v)+0x9e3779b9 + (h<<6)+(h>>2);
        return h;
    }

    // Shape access
    const AShape& shape() const noexcept { return p_shape; }

    
    // Access by vector index
    T& at(const std::vector<size_t>& idx){ return p_dataPtr->at(p_shape.flatIndex(idx)); }
    const T& at(const std::vector<size_t>& idx) const { return p_dataPtr->at(p_shape.flatIndex(idx)); }

    // operator() ND
    template<typename... Idx>
    T& operator()(Idx... idx) {
        static_assert(sizeof...(Idx) > 0, "Must provide indices");
        if(sizeof...(Idx) != p_shape.rank())
            throw std::invalid_argument("operator(): dim mismatch");

        std::vector<size_t> indices{static_cast<size_t>(idx)...};
        return at(indices);  // at() prende std::vector<size_t>
    }

    template<typename... Idx>
    const T& operator()(Idx... idx) const {
        static_assert(sizeof...(Idx) > 0, "Must provide indices");
        if(sizeof...(Idx) != p_shape.rank())
            throw std::invalid_argument("operator(): dim mismatch");

        std::vector<size_t> indices{static_cast<size_t>(idx)...};
        return at(indices);
    }
    
    AArrayProxy<T> operator[](size_t i) {
        return AArrayProxy<T>(this, {i});
    }

    AArrayProxy<T> operator[](size_t i) const {
        return AArrayProxy<T>(const_cast<AArray<T>*>(this), {i});
    }


    // Iterators
    iterator begin() noexcept { return p_dataPtr->data(); }
    iterator end() noexcept { return p_dataPtr->data()+p_dataPtr->size(); }
    const_iterator begin() const noexcept { return p_dataPtr->data(); }
    const_iterator end() const noexcept { return p_dataPtr->data()+p_dataPtr->size(); }

    // Size
    size_t size() const noexcept { return p_dataPtr->size(); }

    // Slice / View
    AArray<T> slice(const std::vector<size_t>& start,
                    const std::vector<size_t>& stop,
                    const std::vector<size_t>& step={}) const {
        std::vector<size_t> sstep = step.empty()? std::vector<size_t>(p_shape.rank(),1):step;
        AShape newShape=p_shape.sliceShape(start,stop,sstep);
        AArray<T> result(newShape);
        for(size_t i=0;i<result.size();++i){
            size_t srcIdx=p_shape.flatIndexFromSlice(i,start,sstep);
            (*result.p_dataPtr)[i]=(*p_dataPtr)[srcIdx];
        }
        return result;
    }

    // Broadcast arithmetic inplace
    AArray<T>& operator+=(const AArray<T>& rhs){ inplaceOp(rhs,std::plus<T>{}); return *this; }
    AArray<T>& operator-=(const AArray<T>& rhs){ inplaceOp(rhs,std::minus<T>{}); return *this; }
    AArray<T>& operator*=(const AArray<T>& rhs){ inplaceOp(rhs,std::multiplies<T>{}); return *this; }
    AArray<T>& operator/=(const AArray<T>& rhs){ inplaceOp(rhs,std::divides<T>{}); return *this; }
    AArray<T>& operator+=(const T rhs){ inplaceOp(rhs,std::plus<T>{}); return *this; }
    AArray<T>& operator-=(const T rhs){ inplaceOp(rhs,std::minus<T>{}); return *this; }
    AArray<T>& operator*=(const T rhs){ inplaceOp(rhs,std::multiplies<T>{}); return *this; }
    AArray<T>& operator/=(const T rhs){ inplaceOp(rhs,std::divides<T>{}); return *this; }

    // Binary operators
    friend AArray<T> operator+(const AArray<T>& lhs,const AArray<T>& rhs){ return binaryOp(lhs,rhs,std::plus<T>{}); }
    friend AArray<T> operator-(const AArray<T>& lhs,const AArray<T>& rhs){ return binaryOp(lhs,rhs,std::minus<T>{}); }
    friend AArray<T> operator*(const AArray<T>& lhs,const AArray<T>& rhs){ return binaryOp(lhs,rhs,std::multiplies<T>{}); }
    friend AArray<T> operator/(const AArray<T>& lhs,const AArray<T>& rhs){ return binaryOp(lhs,rhs,std::divides<T>{}); }

    // Reshape
    void reshape(const std::vector<size_t>& newDims){
        size_t newTotal=std::accumulate(newDims.begin(),newDims.end(),1ULL,std::multiplies<size_t>());
        if(newTotal!=size()) throw std::runtime_error("reshape: total size mismatch");
        p_shape.setDims(newDims);
    }

    // Transpose
    AArray<T> transpose(const std::vector<size_t>& axes={}) const{
        size_t r=p_shape.rank();
        std::vector<size_t> perm=r? std::vector<size_t>(r):std::vector<size_t>();
        if(axes.empty()){
            for(size_t i=0;i<r;i++) perm[i]=r-1-i;
        } else perm=axes;

        std::vector<size_t> newShape(r), newStrides(r);
        for(size_t i=0;i<r;i++){
            newShape[i]=p_shape.dims()[perm[i]];
            newStrides[i]=p_shape.strides()[perm[i]];
        }

        AShape tshape(newShape);
        tshape.setStrides(newStrides);

        AArray<T> result;
        result.p_shape=tshape;
        result.p_dataPtr=p_dataPtr; // shared memory
        return result;
    }

    // Data access
    std::shared_ptr<std::vector<T>>& dataPtr() noexcept { return p_dataPtr; }
    const std::shared_ptr<std::vector<T>>& dataPtr() const noexcept { return p_dataPtr; }

protected:
    AShape p_shape;
    std::shared_ptr<std::vector<T>> p_dataPtr;

private:
    // Copy/Move internals
    void m_copy(const AArray& other){ AObject::m_copy(other); p_shape=other.p_shape; p_dataPtr=std::make_shared<std::vector<T>>(*other.p_dataPtr); }
    void m_move(AArray&& other){ AObject::m_move(std::move(other)); p_shape=std::move(other.p_shape); p_dataPtr=std::move(other.p_dataPtr); }

    // Broadcast helper
    template<typename Op>
    void inplaceOp(const AArray<T>& rhs, Op op){
        AShape resShape = AShape::broadcast(shape(), rhs.shape());
        if(resShape.totalSize()!=size()) throw std::runtime_error("inplaceOp shape mismatch");
        BroadcastIterator it(resShape.dims());
        const auto& lhsDims = shape().dims();
        const auto& lhsStrides = shape().strides();
        const auto& rhsDims = rhs.shape().dims();
        const auto& rhsStrides = rhs.shape().strides();

        auto& lhsData = *p_dataPtr;
        const auto& rhsData = *rhs.p_dataPtr;

        do {
            auto idx=it.index();
            size_t lhsFlat = mapIndex(idx,lhsDims,lhsStrides);
            size_t rhsFlat = mapIndex(idx,rhsDims,rhsStrides);
            lhsData[lhsFlat] = op(lhsData[lhsFlat], rhsData[rhsFlat]);
        } while(it.next());
    }

    template<typename Op>
    static AArray<T> binaryOp(const AArray<T>& lhs,const AArray<T>& rhs, Op op){
        AShape resShape = AShape::broadcast(lhs.shape(), rhs.shape());
        AArray<T> result(resShape);
        BroadcastIterator it(resShape.dims());
        const auto& lhsDims = lhs.shape().dims();
        const auto& lhsStrides = lhs.shape().strides();
        const auto& rhsDims = rhs.shape().dims();
        const auto& rhsStrides = rhs.shape().strides();

        auto& resData = *result.p_dataPtr;
        const auto& lhsData = *lhs.p_dataPtr;
        const auto& rhsData = *rhs.p_dataPtr;

        do {
            auto idx = it.index();
            size_t lhsFlat = mapIndex(idx,lhsDims,lhsStrides);
            size_t rhsFlat = mapIndex(idx,rhsDims,rhsStrides);
            size_t resFlat = result.shape().flatIndex(idx);
            resData[resFlat] = op(lhsData[lhsFlat], rhsData[rhsFlat]);
        } while(it.next());

        return result;
    }

    // Broadcast helpers
    struct BroadcastIterator {
        std::vector<size_t> idx;
        const std::vector<size_t>& dims;
        bool done=false;
        BroadcastIterator(const std::vector<size_t>& dims_): idx(dims_.size(),0), dims(dims_){}
        const std::vector<size_t>& index() const { return idx; }
        bool next(){
            if(done) return false;
            for(int i=int(idx.size())-1;i>=0;--i){
                if(++idx[i]<dims[i]) return true;
                idx[i]=0;
            }
            done=true;
            return false;
        }
    };

    static size_t mapIndex(const std::vector<size_t>& idx,
                           const std::vector<size_t>& shape,
                           const std::vector<size_t>& strides)
    {
        size_t offset=0;
        size_t dimOffset = shape.size() - idx.size();
        for(size_t i=0;i<idx.size();++i){
            size_t dimSize = shape[i+dimOffset];
            size_t cur = (dimSize==1?0:idx[i]);
            offset += cur*strides[i+dimOffset];
        }
        return offset;
    }
};

} // namespace Alib

#endif
