// AArray.h
// ALib
//
// Created by Giuseppe Coppini on 10/03/26.
// Advanced AArray<T> with view/slice, transpose, reshape, broadcasting
// 2026-03-10

#ifndef AARRAY_H
#define AARRAY_H

#include <vector>
#include <memory>
#include <iostream>
#include <initializer_list>
#include <numeric>
#include <algorithm>
#include <stdexcept>
#include <functional>
#include <cmath>

#include "AObject.h"
#include "AShape.h"
#include "Abroadcast.h"

namespace Alib {

// Forward declarations
template<typename T> class AArray;
template<typename T> class ATensorIteratorGeneric;

template<typename T>
class AArrayProxy {
private:
    AArray<T>* p_array;
    std::vector<size_t> p_index;
public:
    AArrayProxy(AArray<T>* arr, const std::vector<size_t>& idx)
        : p_array(arr), p_index(idx) {}

    AArrayProxy<T> operator[](size_t i){
        auto next=p_index; next.push_back(i);
        return AArrayProxy<T>(p_array,std::move(next));
    }

    operator T() const { return p_array->at(p_index); }

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

    // Scalar constructor
    AArray(const AShape& shape, const T& value)
        : p_shape(shape), p_dataPtr(std::make_shared<std::vector<T>>(p_shape.totalSize(), value)) {}

    // -------------------
    // Copy / Move
    // -------------------
    AArray(const AArray& other){ m_copy(other); }
    AArray(AArray&& other) noexcept { m_move(std::move(other)); }
    AArray& operator=(const AArray& other) { if(this!=&other) m_copy(other); return *this; }
    AArray& operator=(AArray&& other) noexcept { if(this!=&other) m_move(std::move(other)); return *this; }

    virtual std::string getClassName() const noexcept override { return "AArray"; }
    [[nodiscard]] size_t type_id() const noexcept override { return type_id_of<AArray<T>>(); }
    bool is_contiguous() const noexcept { return p_shape.is_contiguous(); }
    size_t size() const noexcept { return p_dataPtr->size(); }

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
    AShape& getShape() noexcept { return p_shape; }
    const AShape& getShape() const noexcept { return p_shape; }

    // -------------------
    // ND access
    // -------------------
    AArrayProxy<T> operator[](size_t i) { return AArrayProxy<T>(this,{i}); }
    const AArrayProxy<T> operator[](size_t i) const { return AArrayProxy<T>(const_cast<AArray<T>*>(this),{i}); }

    T& at(const std::vector<size_t>& idx)
    {
        if(p_customAt) return p_customAt(idx);
        if(idx.size()!=p_shape.rank()) throw std::invalid_argument("at: rank mismatch");
        size_t flat=p_offset;
        const auto& strides = p_shape.strides();
        for(size_t i=0;i<idx.size();++i) flat += idx[i]*strides[i];
        return (*p_dataPtr)[flat];
    }

    const T& at(const std::vector<size_t>& idx) const
    {
        if(idx.size()!=p_shape.rank()) throw std::invalid_argument("at: rank mismatch");
        size_t flat=p_offset;
        const auto& strides = p_shape.strides();
        for(size_t i=0;i<idx.size();++i) flat += idx[i]*strides[i];
        return (*p_dataPtr)[flat];
    }

    T* raw() noexcept { return p_dataPtr->data(); }
    const T* raw() const noexcept { return p_dataPtr->data(); }

    template<typename... Idx>
    T& operator()(Idx... idxs) {
        static_assert((std::is_integral_v<Idx> && ...), "All indices must be integral types");
        return at(std::vector<size_t>{static_cast<size_t>(idxs)...});
    }

    template<typename... Idx>
    const T& operator()(Idx... idxs) const {
        static_assert((std::is_integral_v<Idx> && ...), "All indices must be integral types");
        return at(std::vector<size_t>{static_cast<size_t>(idxs)...});
    }

    // -------------------
    // Slice / transpose / reshape
    // -------------------
    AArray<T> slice(const std::vector<size_t>& start,
                    const std::vector<size_t>& stop,
                    const std::vector<size_t>& step = {}) const
    {
        size_t rank = p_shape.rank();
        if(start.size()!=rank || stop.size()!=rank)
            throw std::invalid_argument("slice rank mismatch");

        std::vector<size_t> sstep = step.empty() ? std::vector<size_t>(rank,1) : step;
        std::vector<size_t> newDims(rank), newStrides(rank);
        for(size_t d=0; d<rank; ++d){
            newDims[d] = (stop[d]<=start[d]) ? 0 : (stop[d]-start[d]+sstep[d]-1)/sstep[d];
            newStrides[d] = p_shape.strides()[d]*sstep[d];
        }

        AArray<T> view;
        view.p_shape.setDims(newDims);
        view.p_shape.setStrides(newStrides);
        view.p_dataPtr = p_dataPtr;
        view.p_offset = p_offset + p_shape.flatIndex(start);
        return view;
    }

    AArray<T> transpose(const std::vector<size_t>& axes={}) const {
        size_t rank=p_shape.rank();
        std::vector<size_t> perm=axes.empty()? std::vector<size_t>(rank):axes;
        if(axes.empty()) for(size_t i=0;i<rank;i++) perm[i]=rank-1-i;
        std::vector<size_t> newDims(rank), newStrides(rank);
        for(size_t i=0;i<rank;i++){
            newDims[i]=p_shape.dims()[perm[i]];
            newStrides[i]=p_shape.strides()[perm[i]];
        }
        AArray<T> view;
        view.p_shape.setDims(newDims);
        view.p_shape.setStrides(newStrides);
        view.p_dataPtr=p_dataPtr;
        view.p_offset=p_offset;
        return view;
    }

    void reshape(const std::vector<size_t>& newDims){
        size_t newTotal=std::accumulate(newDims.begin(),newDims.end(),1ULL,std::multiplies<size_t>());
        if(newTotal!=size()) throw std::invalid_argument("reshape: total mismatch");
        p_shape.setDims(newDims);
    }

    // -------------------
    // Arithmetic in-place / binary
    // -------------------
    template<typename Op>
    void inplaceBinaryOp(const AArray<T>& rhs, Op op){
        std::vector<AArray<T>*> arrays = {this, const_cast<AArray<T>*>(&rhs)};
        ATensorIteratorGeneric<T> it(arrays);
        it.forEach([&op](std::vector<T*> ptrs, size_t){
            *ptrs[0] = op(*ptrs[0],*ptrs[1]);
        });
    }

    void inplaceBinaryOp(T scalar, const std::function<T(T,T)>& op){
        for(auto& val:*p_dataPtr)
            val = op(val, scalar);
    }

    AArray<T>& operator+=(const AArray<T>& rhs){ inplaceBinaryOp(rhs,[](T x,T y){return x+y;}); return *this;}
    AArray<T>& operator-=(const AArray<T>& rhs){ inplaceBinaryOp(rhs,[](T x,T y){return x-y;}); return *this;}
    AArray<T>& operator*=(const AArray<T>& rhs){ inplaceBinaryOp(rhs,[](T x,T y){return x*y;}); return *this;}
    AArray<T>& operator/=(const AArray<T>& rhs){ inplaceBinaryOp(rhs,[](T x,T y){return x/y;}); return *this;}

    AArray<T>& operator+=(T scalar){ inplaceBinaryOp(scalar,[](T x,T y){return x+y;}); return *this; }
    AArray<T>& operator-=(T scalar){ inplaceBinaryOp(scalar,[](T x,T y){return x-y;}); return *this; }
    AArray<T>& operator*=(T scalar){ inplaceBinaryOp(scalar,[](T x,T y){return x*y;}); return *this; }
    AArray<T>& operator/=(T scalar){ inplaceBinaryOp(scalar,[](T x,T y){return x/y;}); return *this; }

    // -------------------
    // Binary operators (friend)
    // -------------------
    friend AArray<T> operator+(const AArray<T>& lhs, const AArray<T>& rhs){
        AArray<T> result(lhs.shape());
        result.inplaceBinaryOp(rhs, [](T x,T y){return x+y;});
        return result;
    }
    friend AArray<T> operator-(const AArray<T>& lhs, const AArray<T>& rhs){
        AArray<T> result(lhs.shape());
        result.inplaceBinaryOp(rhs, [](T x,T y){return x-y;});
        return result;
    }
    friend AArray<T> operator*(const AArray<T>& lhs, const AArray<T>& rhs){
        AArray<T> result(lhs.shape());
        result.inplaceBinaryOp(rhs, [](T x,T y){return x*y;});
        return result;
    }
    friend AArray<T> operator/(const AArray<T>& lhs, const AArray<T>& rhs){
        AArray<T> result(lhs.shape());
        result.inplaceBinaryOp(rhs, [](T x,T y){return x/y;});
        return result;
    }

    friend AArray<T> operator+(const AArray<T>& lhs, T scalar){
        AArray<T> result(lhs.shape(), scalar);
        ATensorIteratorGeneric<T> it({const_cast<AArray<T>*>(&lhs), &result});
        it.forEach([scalar](std::vector<T*> ptrs, size_t){
            *ptrs[1] = *ptrs[0]+scalar;
        });
        return result;
    }
    friend AArray<T> operator+(T scalar, const AArray<T>& rhs){ return rhs+scalar; }

    friend AArray<T> operator-(const AArray<T>& lhs, T scalar){
        AArray<T> result(lhs.shape(), scalar);
        ATensorIteratorGeneric<T> it({const_cast<AArray<T>*>(&lhs), &result});
        it.forEach([scalar](std::vector<T*> ptrs, size_t){
            *ptrs[1] = *ptrs[0]-scalar;
        });
        return result;
    }
    friend AArray<T> operator-(T scalar, const AArray<T>& rhs){
        AArray<T> result(rhs.shape(), scalar);
        ATensorIteratorGeneric<T> it({const_cast<AArray<T>*>(&rhs), &result});
        it.forEach([scalar](std::vector<T*> ptrs, size_t){
            *ptrs[1] = scalar - *ptrs[0];
        });
        return result;
    }

    friend AArray<T> operator*(const AArray<T>& lhs, T scalar){
        AArray<T> result(lhs.shape(), scalar);
        ATensorIteratorGeneric<T> it({const_cast<AArray<T>*>(&lhs), &result});
        it.forEach([scalar](std::vector<T*> ptrs, size_t){
            *ptrs[1] = *ptrs[0]*scalar;
        });
        return result;
    }
    friend AArray<T> operator*(T scalar, const AArray<T>& rhs){ return rhs*scalar; }

    friend AArray<T> operator/(const AArray<T>& lhs, T scalar){
        AArray<T> result(lhs.shape(), scalar);
        ATensorIteratorGeneric<T> it({const_cast<AArray<T>*>(&lhs), &result});
        it.forEach([scalar](std::vector<T*> ptrs, size_t){
            *ptrs[1] = *ptrs[0]/scalar;
        });
        return result;
    }
    friend AArray<T> operator/(T scalar, const AArray<T>& rhs){
        AArray<T> result(rhs.shape(), scalar);
        ATensorIteratorGeneric<T> it({const_cast<AArray<T>*>(&rhs), &result});
        it.forEach([scalar](std::vector<T*> ptrs, size_t){
            *ptrs[1] = scalar / *ptrs[0];
        });
        return result;
    }
    
    // -------------------
    // Aggregazioni
    // -------------------
    T sum() const {
        if(p_dataPtr->empty()) return T{};
        if(is_contiguous())
            return std::accumulate(p_dataPtr->begin(), p_dataPtr->end(), T{});
        
        T total{};
        std::vector<AArray<T>*> arrays = {const_cast<AArray<T>*>(this)};
        ATensorIteratorGeneric<T> it(arrays);
        it.forEach([&total](std::vector<T*> ptrs, size_t){ total += *ptrs[0]; });
        return total;
    }

    double mean() const {
        if(size()==0) return 0.0;
        return static_cast<double>(sum()) / static_cast<double>(size());
    }

    T min() const {
        if(p_dataPtr->empty()) throw std::runtime_error("min() on empty array");
        if(is_contiguous())
            return *std::min_element(p_dataPtr->begin(), p_dataPtr->end());
        
        T m = std::numeric_limits<T>::max();
        std::vector<AArray<T>*> arrays = {const_cast<AArray<T>*>(this)};
        ATensorIteratorGeneric<T> it(arrays);
        it.forEach([&m](std::vector<T*> ptrs, size_t){ if(*ptrs[0]<m) m=*ptrs[0]; });
        return m;
    }

    T max() const {
        if(p_dataPtr->empty()) throw std::runtime_error("max() on empty array");
        if(is_contiguous())
            return *std::max_element(p_dataPtr->begin(), p_dataPtr->end());
        
        T M = std::numeric_limits<T>::lowest();
        std::vector<AArray<T>*> arrays = {const_cast<AArray<T>*>(this)};
        ATensorIteratorGeneric<T> it(arrays);
        it.forEach([&M](std::vector<T*> ptrs, size_t){ if(*ptrs[0]>M) M=*ptrs[0]; });
        return M;
    }
    // -------------------
    // Comparison
    // -------------------
    [[nodiscard]] virtual bool m_compare(const AObject& obj) const noexcept override { return m_allEqual(obj); }

    [[nodiscard]] virtual bool m_allEqual(const AObject& obj) const noexcept override {
        auto other = dynamic_cast<const AArray<T>*>(&obj);
        if(!other) return false;
        if(this==other) return true;
        if(p_shape.dims()!=other->shape().dims()) return false;

        if(is_contiguous() && other->is_contiguous())
            return std::equal(raw(), raw()+size(), other->raw());

        std::vector<AArray<T>*> arrays={const_cast<AArray<T>*>(this), const_cast<AArray<T>*>(other)};
        ATensorIteratorGeneric<T> it(arrays);
        bool eq=true;
        it.forEach([&eq](std::vector<T*> ptrs,size_t){ if(*ptrs[0]!=*ptrs[1]) eq=false; });
        return eq;
    }

    [[nodiscard]] virtual bool m_allClose(const AObject& obj,double eps) const noexcept override {
        auto other = dynamic_cast<const AArray<T>*>(&obj);
        if(!other) return false;
        if(this==other) return true;
        if(p_shape.dims()!=other->shape().dims()) return false;

        std::vector<AArray<T>*> arrays={const_cast<AArray<T>*>(this), const_cast<AArray<T>*>(other)};
        ATensorIteratorGeneric<T> it(arrays);
        bool close=true;
        it.forEach([eps,&close](std::vector<T*> ptrs,size_t){ if(std::fabs(*ptrs[0]-*ptrs[1])>eps) close=false; });
        return close;
    }

protected:
    AShape p_shape;
    std::shared_ptr<std::vector<T>> p_dataPtr;
    size_t p_offset=0;
    mutable std::function<T&(const std::vector<size_t>&)> p_customAt;

    void m_copy(const AArray& other){ AObject::m_copy(other); p_shape=other.p_shape; p_dataPtr=std::make_shared<std::vector<T>>(*other.p_dataPtr); }
    void m_move(AArray&& other){ AObject::m_move(std::move(other)); p_shape=std::move(other.p_shape); p_dataPtr=std::move(other.p_dataPtr); }

    template<typename U>
    friend class ATensorIteratorGeneric;

};

// -------------------
// Generic Tensor Iterator
// -------------------
template<typename T>
class ATensorIteratorGeneric {
public:
    ATensorIteratorGeneric(const std::vector<AArray<T>*>& arrays){
        if(arrays.empty()) throw std::invalid_argument("ATensorIteratorGeneric: no arrays provided");
        pp_arrays=arrays;
        pp_shape=arrays[0]->shape();
        for(size_t i=1;i<arrays.size();++i) pp_shape=AShape::broadcast(pp_shape,arrays[i]->shape());
        pp_rank=pp_shape.rank();
    }

    template<typename Lambda>
    void forEach(Lambda&& fn){
        size_t total=pp_shape.totalSize();
        if(total==0) return;
        std::vector<size_t> idx(pp_rank,0);

        for(size_t i=0;i<total;++i){
            std::vector<T*> ptrs(pp_arrays.size());
            for(size_t a=0;a<pp_arrays.size();++a){
                size_t off=pp_arrays[a]->shape().offset();
                const auto& strides=pp_arrays[a]->shape().strides();
                const auto& dims=pp_arrays[a]->shape().dims();
                size_t offset=pp_rank-dims.size();
                for(size_t d=0;d<dims.size();++d) off += (dims[d]==1?0:idx[d+offset]*strides[d]);
                ptrs[a]=pp_arrays[a]->raw()+off;
            }
            fn(ptrs,i);
            for(int d=int(pp_rank)-1;d>=0;--d){
                idx[d]++;
                if(idx[d]<pp_shape.dims()[d]) break;
                idx[d]=0;
            }
        }
    }

private:
    std::vector<AArray<T>*> pp_arrays;
    AShape pp_shape;
    size_t pp_rank;
};

} // namespace Alib
#endif
