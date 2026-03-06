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

    T& at(const std::vector<size_t>& idx)
    {
        if(p_customAt)
            return p_customAt(idx);

        if(idx.size()!=p_shape.rank())
            throw std::invalid_argument("at: rank mismatch");

        size_t flat = p_offset;

        const auto& strides = p_shape.strides();

        for(size_t i=0;i<idx.size();++i)
            flat += idx[i]*strides[i];

        return (*p_dataPtr)[flat];
    }

    const T& at(const std::vector<size_t>& idx) const
    {
        if(idx.size()!=p_shape.rank())
            throw std::invalid_argument("at: rank mismatch");

        size_t flat = p_offset;

        const auto& strides = p_shape.strides();

        for(size_t i=0;i<idx.size();++i)
            flat += idx[i]*strides[i];

        return (*p_dataPtr)[flat];
    }
    

    
    // AArray.h - operator() universale sicuro
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

    // Overload per std::vector
    T& operator()(const std::vector<size_t>& idxs) { return at(idxs); }
    const T& operator()(const std::vector<size_t>& idxs) const { return at(idxs); }

    // Overload per std::vector con tipi interi diversi (ad esempio unsigned long)
    template<typename IntVec,
             typename = std::enable_if_t<
                 std::is_same_v<IntVec, std::vector<int>> ||
                 std::is_same_v<IntVec, std::vector<unsigned>> ||
                 std::is_same_v<IntVec, std::vector<unsigned long>> ||
                 std::is_same_v<IntVec, std::vector<long>> ||
                 std::is_same_v<IntVec, std::vector<size_t>>
             >>
    T& operator()(const IntVec& idxs){
        std::vector<size_t> tmp(idxs.begin(), idxs.end());
        return at(tmp);
    }

    template<typename IntVec,
             typename = std::enable_if_t<
                 std::is_same_v<IntVec, std::vector<int>> ||
                 std::is_same_v<IntVec, std::vector<unsigned>> ||
                 std::is_same_v<IntVec, std::vector<unsigned long>> ||
                 std::is_same_v<IntVec, std::vector<long>> ||
                 std::is_same_v<IntVec, std::vector<size_t>>
             >>
    const T& operator()(const IntVec& idxs) const {
        std::vector<size_t> tmp(idxs.begin(), idxs.end());
        return at(tmp);
    }
    
    // -------------------
    // Slice (view)
    // -------------------

    AArray<T> slice(
        const std::vector<size_t>& start,
        const std::vector<size_t>& stop,
        const std::vector<size_t>& step = {}) const
    {
        size_t rank = p_shape.rank();

        if(start.size()!=rank || stop.size()!=rank)
            throw std::invalid_argument("slice rank mismatch");

        std::vector<size_t> sstep =
            step.empty() ? std::vector<size_t>(rank,1) : step;

        std::vector<size_t> newDims(rank);
        std::vector<size_t> newStrides(rank);

        for(size_t d=0; d<rank; ++d)
        {
            newDims[d] =
                (stop[d]<=start[d]) ?
                0 :
                (stop[d]-start[d]+sstep[d]-1)/sstep[d];

            newStrides[d] =
                p_shape.strides()[d] * sstep[d];
        }

        AArray<T> view;

        view.p_shape.setDims(newDims);
        view.p_shape.setStrides(newStrides);

        view.p_dataPtr = p_dataPtr;

        view.p_offset =
            p_offset + p_shape.flatIndex(start);

        return view;
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
    // Transpose (view)
    // -------------------
    AArray<T> transpose(const std::vector<size_t>& axes = {}) const
    {
        size_t rank = p_shape.rank();

        std::vector<size_t> perm;

        if(axes.empty())
        {
            perm.resize(rank);
            for(size_t i=0;i<rank;i++)
                perm[i] = rank-1-i;
        }
        else
            perm = axes;

        std::vector<size_t> newDims(rank);
        std::vector<size_t> newStrides(rank);

        for(size_t i=0;i<rank;i++)
        {
            newDims[i] =
                p_shape.dims()[perm[i]];

            newStrides[i] =
                p_shape.strides()[perm[i]];
        }

        AArray<T> view;

        view.p_shape.setDims(newDims);
        view.p_shape.setStrides(newStrides);

        view.p_dataPtr = p_dataPtr;

        view.p_offset = p_offset;

        return view;
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

    
    void debugPrint(const std::string& name="AArray") const {
        std::cout << "=== Debug " << name << " ===\n";
        std::cout << "Shape: ";
        for(auto d : p_shape.dims()) std::cout << d << " ";
        std::cout << "\nStrides: ";
        for(auto s : p_shape.strides()) std::cout << s << " ";
        std::cout << "\nOffset: " << p_offset << "\n";
        std::cout << "Total elements: " << size() << "\n";

        if(size()>0) {
            std::cout << "First element: " << at(std::vector<size_t>(p_shape.rank(),0)) << "\n";

            std::vector<size_t> lastIdx;
            for(auto d: p_shape.dims()) lastIdx.push_back(d>0?d-1:0);

            std::cout << "Last element: " << at(lastIdx) << "\n";
        }

        std::cout << "====================\n";
    }
    
    void debugPrintND(const std::string& name="AArray", size_t maxElems=20) const {
        std::cout << "=== Debug ND " << name << " ===\n";
        std::cout << "Shape: ";
        for(auto d : p_shape.dims()) std::cout << d << " ";
        std::cout << "\nStrides: ";
        for(auto s : p_shape.strides()) std::cout << s << " ";
        std::cout << "\nTotal elements: " << size() << "\n";

        if(size()==0){
            std::cout << "(empty array)\n====================\n";
            return;
        }

        // ND traversal
        std::vector<size_t> idxND(p_shape.rank(),0);
        size_t count=0;

        auto printValue = [&](size_t flatIdx){
            std::cout << at(idxND);
            ++count;
            if(count >= maxElems){
                std::cout << " ...";
                return true;
            }
            return false;
        };

        bool done=false;
        while(!done){
            done = printValue(p_shape.flatIndex(idxND));
            // increment ND indices
            for(int d=int(p_shape.rank())-1; d>=0; --d){
                idxND[d]++;
                if(idxND[d] < p_shape.dims()[d]){
                    break;
                } else {
                    if(d==0){ done=true; break; }
                    idxND[d]=0;
                }
            }
            if(done) break;
            std::cout << ", ";
        }

        std::cout << "\n====================\n";
        
        
    }
    
    AArray<T> linearCopy() const {
        AArray<T> copy(shape());
        size_t s = size();
        for(size_t i=0;i<s;++i) copy.dataPtr()->at(i)=dataPtr()->at(i);
        return copy;
    }
protected:
    AShape p_shape;
    std::shared_ptr<std::vector<T>> p_dataPtr;
    
    size_t p_offset = 0;
    
  
    // Custom at() for slice views
    mutable std::function<T&(const std::vector<size_t>&)> p_customAt;
    mutable AShape p_customAtShape;

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
    // Broadcasted binary ops (optimized)
    // -------------------
    template<typename Op>
    static AArray<T> broadcastBinaryOp(const AArray<T>& lhs, const AArray<T>& rhs, Op op){
        AShape resultShape = AShape::broadcast(lhs.shape(), rhs.shape());
        AArray<T> result(resultShape);

        size_t rankRes = resultShape.rank();
        size_t rankL = lhs.shape().rank();
        size_t rankR = rhs.shape().rank();

        const auto& lhsDims = lhs.shape().dims();
        const auto& rhsDims = rhs.shape().dims();
        const auto& lhsStrides = lhs.shape().strides();
        const auto& rhsStrides = rhs.shape().strides();
        const auto& resDims = resultShape.dims();

        const auto& lhsData = *lhs.dataPtr();
        const auto& rhsData = *rhs.dataPtr();
        auto& resData = *result.dataPtr();

        std::vector<size_t> idxND(rankRes,0);

        for(size_t i=0; i<result.size(); ++i){
            size_t tmp = i;
            for(int d=int(rankRes)-1; d>=0; --d){
                idxND[d] = tmp % resDims[d];
                tmp /= resDims[d];
            }

            size_t lhsIdx = 0;
            for(size_t d=0; d<rankRes; ++d){
                size_t ld = (d >= rankRes - rankL) ? lhsDims[d-(rankRes-rankL)] : 1;
                size_t ls = (d >= rankRes - rankL) ? lhsStrides[d-(rankRes-rankL)] : 0;
                lhsIdx += (ld==1?0:idxND[d]*ls);
            }

            size_t rhsIdx = 0;
            for(size_t d=0; d<rankRes; ++d){
                size_t rd = (d >= rankRes - rankR) ? rhsDims[d-(rankRes-rankR)] : 1;
                size_t rs = (d >= rankRes - rankR) ? rhsStrides[d-(rankRes-rankR)] : 0;
                rhsIdx += (rd==1?0:idxND[d]*rs);
            }

            resData[i] = op(lhsData[lhsIdx], rhsData[rhsIdx]);
        }

        return result;
    }
};

} // namespace Alib
#endif
