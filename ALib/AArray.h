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
private:
    AArray<T>* p_array;
    std::vector<size_t> p_index;

public:
    AArrayProxy(AArray<T>* arr, const std::vector<size_t>& idx)
        : p_array(arr), p_index(idx) {}

    // chainable operator[]
    AArrayProxy<T> operator[](size_t i){
        auto next=p_index; next.push_back(i);
        return AArrayProxy<T>(p_array,std::move(next));
    }

    // conversion to scalar
    operator T() const { return p_array->at(p_index); }

    // assignment
    AArrayProxy<T>& operator=(const T& value) {
        p_array->at(p_index)=value;
        return *this;
    }
};

template<typename T>
class AArray : public AObject {
public:
    using value_type=T;

    // -------------------
    // Constructors
    // -------------------
    AArray() = default;
    explicit AArray(const std::vector<size_t>& dims)
        : p_shape(dims), p_dataPtr(std::make_shared<std::vector<T>>(p_shape.totalSize())) {}
    explicit AArray(const AShape& shape)
        : p_shape(shape), p_dataPtr(std::make_shared<std::vector<T>>(p_shape.totalSize())) {}
    template<typename... Dims,
             typename = std::enable_if_t<(std::conjunction_v<std::is_integral<Dims>...>)>>
    explicit AArray(Dims... dims)
        : p_shape(std::vector<size_t>{static_cast<size_t>(dims)...}),
          p_dataPtr(std::make_shared<std::vector<T>>(p_shape.totalSize())) {}

    explicit AArray(const std::initializer_list<size_t>& dims)
        : p_shape(dims), p_dataPtr(std::make_shared<std::vector<T>>(p_shape.totalSize())) {}

    // -------------------
    // Copy / Move
    // -------------------
    AArray(const AArray& other){ m_copy(other); }
    AArray(AArray&& other) noexcept { m_move(std::move(other)); }
    AArray& operator=(const AArray& other) { if(this!=&other) m_copy(other); return *this; }
    AArray& operator=(AArray&& other) noexcept { if(this!=&other) m_move(std::move(other)); return *this; }

    // -------------------
    // Clone / Hash
    // -------------------
    std::unique_ptr<AObject> clone() const override { return std::make_unique<AArray<T>>(*this); }
    size_t hash() const override {
        size_t h=0;
        for(const auto& v:*p_dataPtr) h ^= std::hash<T>{}(v)+0x9e3779b9+(h<<6)+(h>>2);
        return h;
    }

    // -------------------
    // Shape access
    // -------------------
    const AShape& shape() const noexcept { return p_shape; }

    // -------------------
    // ND access
    // -------------------
    AArrayProxy<T> operator[](size_t i) { return AArrayProxy<T>(this,{i}); }
    const AArrayProxy<T> operator[](size_t i) const { return AArrayProxy<T>(const_cast<AArray<T>*>(this),{i}); }

    T& at(const std::vector<size_t>& idx) {
        if(idx.size()!=p_shape.rank()) throw std::invalid_argument("at: rank mismatch");
        return p_dataPtr->at(p_shape.flatIndex(idx));
    }
    const T& at(const std::vector<size_t>& idx) const {
        if(idx.size()!=p_shape.rank()) throw std::invalid_argument("at: rank mismatch");
        return p_dataPtr->at(p_shape.flatIndex(idx));
    }

    template<typename... Idx>
    T& operator()(Idx... idx){
        return at(std::vector<size_t>{static_cast<size_t>(idx)...});
    }
    template<typename... Idx>
    const T& operator()(Idx... idx) const {
        return at(std::vector<size_t>{static_cast<size_t>(idx)...});
    }

    // -------------------
    // Slice
    // -------------------
    AArray<T> slice(const std::vector<size_t>& start,
                    const std::vector<size_t>& stop,
                    const std::vector<size_t>& step={}) const {
        std::vector<size_t> sstep = step.empty()? std::vector<size_t>(p_shape.rank(),1):step;
        AShape newShape = p_shape.sliceShape(start,stop,sstep);
        AArray<T> result(newShape);
        for(size_t i=0;i<result.size();++i){
            size_t srcIdx=p_shape.flatIndexFromSlice(i,start,sstep);
            (*result.p_dataPtr)[i]=(*p_dataPtr)[srcIdx];
        }
        return result;
    }

    // -------------------
    // Reshape
    // -------------------
    void reshape(const std::vector<size_t>& newDims){
        size_t newTotal=std::accumulate(newDims.begin(),newDims.end(),1ULL,std::multiplies<size_t>());
        if(newTotal != size()) throw std::invalid_argument("reshape: total mismatch");
        p_shape.setDims(newDims);
    }

    // -------------------
    // Transpose
    // -------------------
    AArray<T> transpose(const std::vector<size_t>& axes={}) const {
        AShape tshape = p_shape;
        if(axes.empty()){
            std::vector<size_t> perm(p_shape.rank());
            for(size_t i=0;i<p_shape.rank();++i) perm[i]=p_shape.rank()-1-i;
            tshape.transpose(perm);
        } else tshape.transpose(axes);
        AArray<T> result;
        result.p_shape = tshape;
        result.p_dataPtr = p_dataPtr; // shared view
        return result;
    }

    // -------------------
    // Arithmetic in-place
    // -------------------
    template<typename Op>
    void inplaceBinaryOp(const AArray<T>& rhs, Op op){
        auto result = broadcastBinaryOp(*this,rhs,op);
        *this = std::move(result);
    }

    AArray<T>& operator+=(const AArray<T>& rhs){ inplaceBinaryOp(rhs,[](T x,T y){return x+y;}); return *this;}
    AArray<T>& operator-=(const AArray<T>& rhs){ inplaceBinaryOp(rhs,[](T x,T y){return x-y;}); return *this;}
    AArray<T>& operator*=(const AArray<T>& rhs){ inplaceBinaryOp(rhs,[](T x,T y){return x*y;}); return *this;}
    AArray<T>& operator/=(const AArray<T>& rhs){ inplaceBinaryOp(rhs,[](T x,T y){return x/y;}); return *this;}

    friend AArray<T> operator+(const AArray<T>& lhs,const AArray<T>& rhs){ return broadcastBinaryOp(lhs,rhs,std::plus<T>()); }
    friend AArray<T> operator-(const AArray<T>& lhs,const AArray<T>& rhs){ return broadcastBinaryOp(lhs,rhs,std::minus<T>()); }
    friend AArray<T> operator*(const AArray<T>& lhs,const AArray<T>& rhs){ return broadcastBinaryOp(lhs,rhs,std::multiplies<T>()); }
    friend AArray<T> operator/(const AArray<T>& lhs,const AArray<T>& rhs){ return broadcastBinaryOp(lhs,rhs,std::divides<T>()); }

    // -------------------
    // Data access
    // -------------------
    std::shared_ptr<std::vector<T>>& dataPtr() noexcept { return p_dataPtr; }
    const std::shared_ptr<std::vector<T>>& dataPtr() const noexcept { return p_dataPtr; }
    size_t size() const noexcept { return p_dataPtr->size(); }

protected:
    AShape p_shape;
    std::shared_ptr<std::vector<T>> p_dataPtr;

    void m_copy(const AArray& other){
        AObject::m_copy(other);
        p_shape=other.p_shape;
        p_dataPtr=std::make_shared<std::vector<T>>(*other.p_dataPtr);
    }
    void m_move(AArray&& other){
        AObject::m_move(std::move(other));
        p_shape=std::move(other.p_shape);
        p_dataPtr=std::move(other.p_dataPtr);
    }

    // -------------------
    // Binary ops with broadcasting
    // -------------------
    template<typename Op>
    static AArray<T> broadcastBinaryOp(const AArray<T>& lhs,const AArray<T>& rhs, Op op){
        AShape resultShape = AShape::broadcast(lhs.shape(),rhs.shape());
        AArray<T> result(resultShape);

        const auto& lhsDims=lhs.shape().dims();
        const auto& rhsDims=rhs.shape().dims();
        const auto& lhsStrides=lhs.shape().strides();
        const auto& rhsStrides=rhs.shape().strides();
        const auto& resDims=resultShape.dims();

        auto& resData = result.p_dataPtr;
        const auto& lhsData = lhs.p_dataPtr;
        const auto& rhsData = rhs.p_dataPtr;

        size_t rankRes = resultShape.rank();
        size_t rankL = lhs.shape().rank();
        size_t rankR = rhs.shape().rank();

        for(size_t i=0;i<resultShape.totalSize();++i){
            size_t idx=i;
            size_t lhsIdx=0, rhsIdx=0;
            for(int d=int(rankRes)-1;d>=0;--d){
                size_t cur = idx % resDims[d];

                size_t ldim = (d>=rankRes-rankL)?lhsDims[d-(rankRes-rankL)]:1;
                size_t rdim = (d>=rankRes-rankR)?rhsDims[d-(rankRes-rankR)]:1;

                size_t lstride=(d>=rankRes-rankL)?lhsStrides[d-(rankRes-rankL)]:0;
                size_t rstride=(d>=rankRes-rankR)?rhsStrides[d-(rankRes-rankR)]:0;

                lhsIdx += (ldim==1?0:cur*lstride);
                rhsIdx += (rdim==1?0:cur*rstride);

                idx/=resDims[d];
            }
            (*resData)[i] = op((*lhsData)[lhsIdx], (*rhsData)[rhsIdx]);
        }

        return result;
    }

};

} // namespace Alib
#endif
