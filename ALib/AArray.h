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

inline size_t mapIndex(const std::vector<size_t>& idx,
                       const std::vector<size_t>& shape,
                       const std::vector<size_t>& strides)
{
    size_t offset = 0;
    size_t dimOffset = shape.size() - idx.size();
    for(size_t i = 0; i < idx.size(); ++i)
    {
        size_t dimSize = shape[i + dimOffset];
        size_t cur = (dimSize == 1 ? 0 : idx[i]);
        offset += cur * strides[i + dimOffset];
    }
    return offset;
}

class BroadcastIterator {
    std::vector<size_t> idx;
    const std::vector<size_t>& dims;
    bool done = false;

public:
    BroadcastIterator(const std::vector<size_t>& dims_)
        : idx(dims_.size(),0), dims(dims_) {}

    const std::vector<size_t>& index() const { return idx; }

    bool next() {
        if(done) return false;
        for(int i = int(idx.size()) - 1; i >= 0; --i) {
            if(++idx[i] < dims[i]) return true;
            idx[i] = 0;
        }
        done = true;
        return false;
    }

    bool hasNext() const { return !done; }
};

template<typename T> class AArray;


template<typename T>
class AArrayProxy {

private:

    AArray<T>* p_array;
    std::vector<size_t> p_index;

public:
    
    using value_type = T;
    

    AArrayProxy(AArray<T>* arr, const std::vector<size_t>& idx)
        : p_array(arr), p_index(idx) {}
    // Chainable operator[]
    AArrayProxy<T> operator[](size_t i) {
        auto next = p_index;
        next.push_back(i);
        return AArrayProxy<T>(p_array, std::move(next));
    }

    // Read conversion
    operator T() const {
        return p_array->at(p_index); // implement at(indices) to compute flat index
    }

    // Assignment from scalar
    AArrayProxy<T>& operator=(const T& value) {
        p_array->at(p_index) = value;
        return *this;
    }
    

    // conversione quando l'indice è completo
    operator T&()
    {
        if(p_index.size()!=p_array->shape().rank())
            throw std::runtime_error("Incomplete index");

        size_t flat = p_array->shape().flatIndex(p_index);
        return (*p_array->dataPtr())[flat];
    }

    operator const T&() const
    {
        if(p_index.size()!=p_array->shape().rank())
            throw std::runtime_error("Incomplete index");

        size_t flat = p_array->shape().flatIndex(p_index);
        return (*p_array->dataPtr())[flat];
    }
};

template<typename T>
class AArray : public AObject {
public:
    

    using value_type = T;
    using iterator = T*;
    using const_iterator = const T*;

    // -------------------------
    // Constructors
    // -------------------------
        AArray() = default;

        explicit AArray(const std::vector<size_t>& dims)
            : p_shape(dims),
              p_dataPtr(std::make_shared<std::vector<T>>(p_shape.totalSize())),
              p_offset(0)
        {}

        explicit AArray(const AShape& shape)
            : p_shape(shape),
              p_dataPtr(std::make_shared<std::vector<T>>(shape.totalSize())),
              p_offset(0)
        {}

        template<typename... Dims,
                 typename = std::enable_if_t<(std::conjunction_v<std::is_integral<Dims>...>)>>
        explicit AArray(Dims... dims)
            : p_shape(std::vector<size_t>{static_cast<size_t>(dims)...}),
              p_dataPtr(std::make_shared<std::vector<T>>(p_shape.totalSize())),
              p_offset(0)
        {}

        explicit AArray(std::initializer_list<size_t> dims)
            : p_shape(std::vector<size_t>(dims)),
              p_dataPtr(std::make_shared<std::vector<T>>(p_shape.totalSize())),
              p_offset(0)
        {}
    // -------------------------
    // Copy / Move
    // -------------------------
    AArray(const AArray&) = default;
        AArray(AArray&&) noexcept = default;
        AArray& operator=(const AArray&) = default;
        AArray& operator=(AArray&&) noexcept = default;
    
    // -------------------------
        // Clone / Hash
        // -------------------------

        std::unique_ptr<AObject> clone() const override
        {
            return std::make_unique<AArray<T>>(*this);
        }

        size_t hash() const override
        {
            size_t h = 0;
            for(const auto& v : *p_dataPtr)
                h ^= std::hash<T>{}(v) + 0x9e3779b9 + (h<<6) + (h>>2);
            return h;
        }


    // -------------------------
        // Shape
        // -------------------------

        const AShape& shape() const noexcept { return p_shape; }
        size_t size() const noexcept { return p_shape.totalSize(); }
        size_t rank() const noexcept { return p_shape.rank(); }


    // -------------------------
        // Data access
        // -------------------------

        std::shared_ptr<std::vector<T>>& dataPtr() noexcept { return p_dataPtr; }
        const std::shared_ptr<std::vector<T>>& dataPtr() const noexcept { return p_dataPtr; }

        // -------------------------
        // flat index computation
        // -------------------------

    private:

        size_t flatIndex(const std::vector<size_t>& idx) const
        {
            const auto& strides = p_shape.strides();
            const auto& dims = p_shape.dims();

            if(idx.size() != dims.size())
                throw std::runtime_error("AArray::flatIndex rank mismatch");

            size_t flat = p_offset;

            for(size_t i=0;i<idx.size();++i)
            {
                if(idx[i] >= dims[i])
                    throw std::out_of_range("AArray index out of bounds");

                flat += idx[i] * strides[i];
            }

            return flat;
        }

    public:
    
    
    
    // -------------------------
    // at() ND access
    // -------------------------

    T& at(const std::vector<size_t>& idx)
    {
        return (*p_dataPtr)[flatIndex(idx)];
    }

    const T& at(const std::vector<size_t>& idx) const
    {
        return (*p_dataPtr)[flatIndex(idx)];
    }

    // -------------------------
    // operator()
    // -------------------------

    template<typename... Idx>
    T& operator()(Idx... idx)
    {
        std::array<size_t,sizeof...(Idx)> v{static_cast<size_t>(idx)...};
        return at(std::vector<size_t>(v.begin(),v.end()));
    }

    template<typename... Idx>
    const T& operator()(Idx... idx) const
    {
        std::array<size_t,sizeof...(Idx)> v{static_cast<size_t>(idx)...};
        return at(std::vector<size_t>(v.begin(),v.end()));
    }
    // Proxy subscript operator for ND
    

    AArrayProxy<T> operator[](size_t i)
    {
        return AArrayProxy<T>(this,{i});
    }

    AArrayProxy<T> operator[](size_t i) const
    {
        return AArrayProxy<T>(
            const_cast<AArray<T>*>(this),
            {i}
        );
    }

    // -------------------------
    // at() access by vector index
    // -------------------------
    
    
    
    
    
    // -------------------------
    // Iterators
    // -------------------------
    iterator begin() noexcept { return p_dataPtr->data(); }
    iterator end() noexcept { return p_dataPtr->data()+p_dataPtr->size(); }
    const_iterator begin() const noexcept { return p_dataPtr->data(); }
    const_iterator end() const noexcept { return p_dataPtr->data()+p_dataPtr->size(); }

    // -------------------------
        // reshape
        // -------------------------

        void reshape(const std::vector<size_t>& dims)
        {
            size_t newSize =
                std::accumulate(dims.begin(),dims.end(),1ULL,std::multiplies<size_t>());

            if(newSize != size())
                throw std::runtime_error("reshape size mismatch");

            p_shape.setDims(dims);
        }
    
    // -------------------------
    // transpose (view)
    // -------------------------

    AArray<T> transpose(const std::vector<size_t>& axes = {}) const
    {
        size_t r = p_shape.rank();

        std::vector<size_t> perm;

        if(axes.empty())
        {
            perm.resize(r);
            for(size_t i=0;i<r;i++)
                perm[i] = r-1-i;
        }
        else
            perm = axes;

        std::vector<size_t> newDims(r);
        std::vector<size_t> newStrides(r);

        for(size_t i=0;i<r;i++)
        {
            newDims[i] = p_shape.dims()[perm[i]];
            newStrides[i] = p_shape.strides()[perm[i]];
        }

        AShape tshape(newDims);
        tshape.setStrides(newStrides);

        AArray<T> view;
        view.p_shape = tshape;
        view.p_dataPtr = p_dataPtr;
        view.p_offset = p_offset;

        return view;
    }
    
    // -------------------------
      // slice (view)
      // -------------------------

      AArray<T> slice(const std::vector<size_t>& start,
                      const std::vector<size_t>& stop,
                      const std::vector<size_t>& step = {}) const
      {
          std::vector<size_t> sstep =
              step.empty()? std::vector<size_t>(rank(),1):step;

          AShape newShape = p_shape.sliceShape(start,stop,sstep);

          std::vector<size_t> newStrides = p_shape.strides();

          size_t newOffset = p_offset;

          for(size_t i=0;i<rank();++i)
          {
              newOffset += start[i]*newStrides[i];
              newStrides[i] *= sstep[i];
          }

          newShape.setStrides(newStrides);

          AArray<T> view;
          view.p_shape = newShape;
          view.p_dataPtr = p_dataPtr;
          view.p_offset = newOffset;

          return view;
      }
    
    // -------------------------
     // broadcasting
     // -------------------------

     static AArray<T> broadcastBinaryOp(
         const AArray<T>& A,
         const AArray<T>& B,
         auto op)
     {
         AShape resultShape = AShape::broadcast(A.shape(),B.shape());
         AArray<T> result(resultShape);

         const auto& rDims = resultShape.dims();

         std::vector<size_t> idx(rDims.size(),0);

         for(size_t i=0;i<result.size();++i)
         {
             size_t ai=0, bi=0;

             for(size_t d=0; d<rDims.size(); ++d)
             {
                 size_t aDim = (d>=rDims.size()-A.rank()) ?
                     A.shape().dims()[d-(rDims.size()-A.rank())] : 1;

                 size_t bDim = (d>=rDims.size()-B.rank()) ?
                     B.shape().dims()[d-(rDims.size()-B.rank())] : 1;

                 if(aDim!=1)
                     ai += idx[d]*A.shape().strides()[d-(rDims.size()-A.rank())];

                 if(bDim!=1)
                     bi += idx[d]*B.shape().strides()[d-(rDims.size()-B.rank())];
             }

             (*result.p_dataPtr)[i] =
                 op((*A.p_dataPtr)[A.p_offset+ai],
                    (*B.p_dataPtr)[B.p_offset+bi]);

             for(int d=int(rDims.size())-1; d>=0; --d)
             {
                 if(++idx[d] < rDims[d]) break;
                 idx[d]=0;
             }
         }

         return result;
     }

  

    

    
    // -------------------------
    // Broadcasting helper
    // -------------------------
    static AShape broadcastShape(const AShape& a, const AShape& b) { return AShape::broadcast(a,b); }

    // -------------------------
    // Arithmetic operators (with broadcasting)
    // -------------------------
   
    AArray<T>& operator+=(const AArray<T>& other)
    {
        checkShape(other);
        for(size_t i=0;i<size();++i)
            m_data[i] += other.m_data[i];
        return *this;
    }

    AArray<T>& operator-=(const AArray<T>& other)
    {
        checkShape(other);
        for(size_t i=0;i<size();++i)
            m_data[i] -= other.m_data[i];
        return *this;
    }

    AArray<T>& operator*=(const AArray<T>& other)
    {
        checkShape(other);
        for(size_t i=0;i<size();++i)
            m_data[i] *= other.m_data[i];
        return *this;
    }

    AArray<T>& operator/=(const AArray<T>& other)
    {
        checkShape(other);
        for(size_t i=0;i<size();++i)
            m_data[i] /= other.m_data[i];
        return *this;
    }
    friend AArray<T> operator+(const AArray<T>& lhs, const AArray<T>& rhs) {
        return broadcastBinaryOp(lhs, rhs, [](T x, T y){ return x + y; });
    }

    friend AArray<T> operator-(const AArray<T>& lhs, const AArray<T>& rhs) {
        return broadcastBinaryOp(lhs, rhs, [](T x, T y){ return x - y; });
    }

    friend AArray<T> operator*(const AArray<T>& lhs, const AArray<T>& rhs) {
        return broadcastBinaryOp(lhs, rhs, [](T x, T y){ return x * y; });
    }

    friend AArray<T> operator/(const AArray<T>& lhs, const AArray<T>& rhs) {
        return broadcastBinaryOp(lhs, rhs, [](T x, T y){ return x / y; });
    }
    
protected:
    AShape p_shape;
    std::shared_ptr<std::vector<T>> p_dataPtr;
    size_t p_offset = 0;

    void m_copy(const AArray& other) {
        AObject::m_copy(other);
        p_shape=other.p_shape;
        p_dataPtr=std::make_shared<std::vector<T>>(*other.p_dataPtr);
    }

    void m_move(AArray&& other) {
        AObject::m_move(std::move(other));
        p_shape=std::move(other.p_shape);
        p_dataPtr=std::move(other.p_dataPtr);
    }
};

} // namespace Alib
#endif


