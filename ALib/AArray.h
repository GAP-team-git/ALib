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

#include "AArrayBase.h"
#include "AArrayView.h"
#include "AAssert.h"
#include "AObject.h"
#include "AErrors.h"
#include "AShape.h"
#include "Abroadcast.h"
#include "AtensorIteratorGeneric.h"


namespace Alib {

// Forward declarations

template<typename T> class ATensorIteratorGeneric;


template<typename T>
class AArray : public AArrayBase<T> {
public:
    
    using Base = AArrayBase<T>;

    // -------------------
    // Constructors
    // -------------------
    AArray() = default;
    explicit AArray(const std::vector<size_t>& dims) {
        shape()=dims;
        pp_dataPtr = std::make_shared<std::vector<T>>(shape().totalSize());
    }
    explicit AArray(const AShape& shape){
        this->p_shape=shape;
        pp_dataPtr=std::make_shared<std::vector<T>>(Base::shape().totalSize());
    }
    template<typename... Dims,
        typename = std::enable_if_t<(std::conjunction_v<std::is_integral<Dims>...>)>>
    explicit AArray(Dims... dims){
        this->p_shape=AShape(std::vector<size_t>{static_cast<size_t>(dims)...});
        pp_dataPtr=std::make_shared<std::vector<T>>(shape().totalSize());
    }
    explicit AArray(const std::initializer_list<size_t>& dims){
        shape()=dims;
        pp_dataPtr(std::make_shared<std::vector<T>>(shape().totalSize()));
    }
    // Scalar constructor
    AArray(const AShape& shape, const T& value){
        Base::shape()=shape;
        pp_dataPtr(std::make_shared<std::vector<T>>(Base::shape().totalSize(),value));
    }

// AShape shape() noexcept{return Base::shape()};

    virtual T* raw() noexcept override{
           return pp_dataPtr->data();
    }

    virtual  const T* raw() const noexcept override{
           return pp_dataPtr->data();
    }
    
    
    // -------------------
    // Copy / Move
    // -------------------
    AArray(const AArray& other){ m_copy(other); }
    AArray(AArray&& other) noexcept { m_move(std::move(other)); }
    AArray& operator=(const AArray& other) { if(this!=&other) m_copy(other); return *this; }
    AArray& operator=(AArray&& other) noexcept { if(this!=&other) m_move(std::move(other)); return *this; }

    virtual std::string getClassName() const noexcept override { return "AArray"; }
    [[nodiscard]] size_t type_id() const noexcept override { return type_id_of<AArray<T>>(); }
    bool is_contiguous() const noexcept { return shape().is_contiguous(); }
    size_t size() const noexcept { return pp_dataPtr->size(); }

    // -------------------
    // Shape access
    // -------------------
    const AShape& shape() const noexcept { return Base::p_shape; }
    AShape& getShape() noexcept { return Base::p_shape; }
    const AShape& getShape() const noexcept { return Base::p_shape; }


    template<typename... Idx>
    T& operator()(Idx... idxs) {
        ALIB_ASSERT((std::is_integral_v<Idx> && ...), AErrorInfoEx(
                                                                   AErrors::typeMismatch,
                                                                   AErrorCategory::general,
                                                                   AErrorSeverity::critical,
                                                                   "operator(): All indices must be integral types",
                                                                   "Check parameter call"
                                                                   ));
        return Base::at(std::vector<size_t>{static_cast<size_t>(idxs)...});
    }

    template<typename... Idx>
    const T& operator()(Idx... idxs) const {
        ALIB_ASSERT((std::is_integral_v<Idx> && ...), AErrorInfoEx(
                                                                   AErrors::typeMismatch,
                                                                   AErrorCategory::general,
                                                                   AErrorSeverity::critical,
                                                                   "operator(): All indices must be integral types",
                                                                   "Check parameter call"
                                                                   ));
        return Base::at(std::vector<size_t>{static_cast<size_t>(idxs)...});
    }

    
    // -------------------
    // Slice / transpose / reshape
    // -------------------

    AArrayView<T> slice(
        const std::vector<size_t>& start,
        const std::vector<size_t>& stop,
        const std::vector<size_t>& step) const {
        size_t rank = this->shape().rank();

        std::vector<size_t> sstep =
            step.empty() ? std::vector<size_t>(rank,1) : step;

        std::vector<size_t> newDims(rank);
        std::vector<size_t> newStrides(rank);

        for(size_t d=0; d<rank; ++d)
        {
            newDims[d] =
            (stop[d]-start[d]+sstep[d]-1)/sstep[d];

            newStrides[d] =
            this->shape().strides()[d]*sstep[d];
        }

        AShape newShape(newDims,newStrides);

        size_t newOffset =
            this->p_offset +
            this->shape().flatIndex(start);

        return AArrayView<T>(pp_dataPtr,newShape,newOffset);
    }
    
    AArray<T> transpose(const std::vector<size_t>& axes={}) const {
        size_t rank=shape().rank();
        std::vector<size_t> perm=axes.empty()? std::vector<size_t>(rank):axes;
        if(axes.empty()) for(size_t i=0;i<rank;i++) perm[i]=rank-1-i;
        std::vector<size_t> newDims(rank), newStrides(rank);
        for(size_t i=0;i<rank;i++){
            newDims[i]=shape().dims()[perm[i]];
            newStrides[i]=shape().strides()[perm[i]];
        }
        AArray<T> view;
        view.getShape().setDims(newDims);
        view.getShape().setStrides(newStrides);
        view.pp_dataPtr=pp_dataPtr;
        view.p_shape.setOffset(shape().offset());
        return view;
    }

    void reshape(const std::vector<size_t>& newDims){
        size_t newTotal=std::accumulate(newDims.begin(),newDims.end(),1ULL,std::multiplies<size_t>());
        ALIB_SHAPE_ASSERT(newTotal==size(), AErrorInfoEx(
                                                         ShapeErrors::invalidDim,
                                                         AErrorCategory::array,
                                                         AErrorSeverity::critical,
                                                         "total mismatch",
                                                         "Check rank consistency"
                                                         ));
        shape().setDims(newDims);
    }

    // -------------------
    // Arithmetic in-place / binary
    // -------------------
    std::unique_ptr<AObject> clone() const override
        {

            AArray<T> out(this->p_shape);

        
            std::vector<AArrayBase<T>*> arrays =
            {const_cast<AArray<T>*>(this),&out};

            ATensorIteratorGeneric<T> it(arrays);

            it.forEach([](std::vector<T*> p,size_t)
            {
                *p[1] = *p[0];
            });

            return std::make_unique<AArray<T>>(out);
        }
    // -------------------
    // Comparison
    // -------------------
    [[nodiscard]] virtual bool m_compare(const AObject& obj) const noexcept override { return m_allEqual(obj); }

    [[nodiscard]] virtual bool m_allEqual(const AObject& obj) const noexcept override {
        auto other = dynamic_cast<const AArray<T>*>(&obj);
        if(!other) return false;
        if(this==other) return true;
        if(shape().dims()!=other->shape().dims()) return false;

        if(is_contiguous() && other->is_contiguous())
            return std::equal(raw(), raw()+size(), other->raw());

        std::vector<AArrayBase<T>*> arrays={const_cast<AArray<T>*>(this), const_cast<AArray<T>*>(other)};
        ATensorIteratorGeneric<T> it(arrays);
        bool eq=true;
        it.forEach([&eq](std::vector<T*> ptrs,size_t){ if(*ptrs[0]!=*ptrs[1]) eq=false; });
        return eq;
    }

    [[nodiscard]] virtual bool m_allClose(const AObject& obj,double eps) const noexcept override {
        auto other = dynamic_cast<const AArray<T>*>(&obj);
        if(!other) return false;
        if(this==other) return true;
        if(shape().dims()!=other->shape().dims()) return false;

        std::vector<AArrayBase<T>*> arrays={const_cast<AArray<T>*>(this), const_cast<AArray<T>*>(other)};
        ATensorIteratorGeneric<T> it(arrays);
        bool close=true;
        it.forEach([eps,&close](std::vector<T*> ptrs,size_t){ if(std::fabs(*ptrs[0]-*ptrs[1])>eps) close=false; });
        return close;
    }

protected:
    

    void m_copy(const AArray& other){ AObject::m_copy(other); this->p_shape=other.shape(); pp_dataPtr=std::make_shared<std::vector<T>>(*other.pp_dataPtr); }
    void m_move(AArray&& other){ AObject::m_move(std::move(other)); this->p_shape=std::move(other.shape()); pp_dataPtr=std::move(other.pp_dataPtr); }
    
    [[nodiscard]] virtual bool m_write(std::ostream& os) const override{
        os.write(reinterpret_cast<const char*>(&(AObject::p_magic)), sizeof(AObject::p_magic));
        if (AObject::p_magic != AOBJ_MAGIC) return false;
        os.write(reinterpret_cast<const char*>(&(AObject::p_version)), sizeof(AObject::p_version));
        if(!shape().write(os))return false;
        const T* dataPtr = raw();
        if(dataPtr==nullptr) return false;
        if(is_contiguous()){
          int64_t  datasize= shape().totalSize()*sizeof(T);
          os.write(reinterpret_cast<const char*>(dataPtr), datasize);
        }
        else return false;

        return os.good();
    }

    [[nodiscard]] virtual bool m_read(std::istream& is) override {
        uint32_t magic = 0;
        is.read(reinterpret_cast<char*>(&magic), sizeof(magic));
        if (magic != AOBJ_MAGIC) return false;
        AObject::p_magic = magic;
        
        uint32_t version = 0;
        is.read(reinterpret_cast<char*>(&version), sizeof(version));
        if (!is.good()) return false;
        AObject::p_version = version;
        
        AShape newShape;
        
        if(!newShape.read(is)) return false;
       
        getShape()=newShape;
        
        
        pp_dataPtr = std::make_shared<std::vector<T>>();
        pp_dataPtr->resize(shape().totalSize());
        is.read(reinterpret_cast<char*>(pp_dataPtr->data()), shape().totalSize()*sizeof(T));
        if(!is.good())return false;
        return true;
    }

    
private:
    std::shared_ptr<std::vector<T>> pp_dataPtr;
    template<typename U>
    friend class AArrayView;
};

// -------------------
// Binary ops
// -------------------

template<typename T> AArray<T> operator+(const AArrayBase<T>& lhs,const AArrayBase<T>& rhs){

    AArray<T> out(lhs.shape());
    
    std::vector<AArrayBase<T>*> arrays =
    {const_cast<AArrayBase<T>*>(&lhs),
     const_cast<AArrayBase<T>*>(&rhs),
     &out};

    ATensorIteratorGeneric<T> it(arrays);

    it.forEach([](std::vector<T*> p,size_t)
    {
        *p[2] = *p[0] + *p[1];
    });

    return out;
}

template<typename T> AArray<T> operator-(const AArrayBase<T>& lhs,const AArrayBase<T>& rhs){

    AArray<T> out(lhs.shape());

    std::vector<AArrayBase<T>*> arrays =
    {const_cast<AArrayBase<T>*>(&lhs),
     const_cast<AArrayBase<T>*>(&rhs),
     &out};

    ATensorIteratorGeneric<T> it(arrays);

    it.forEach([](std::vector<T*> p,size_t)
    {
        *p[2] = *p[0] - *p[1];
    });

    return out;
}

template<typename T> AArray<T> operator*(const AArrayBase<T>& lhs, const AArrayBase<T>& rhs){

    AArray<T> out(lhs.shape());

    std::vector<AArrayBase<T>*> arrays =
    {const_cast<AArrayBase<T>*>(&lhs),
     const_cast<AArrayBase<T>*>(&rhs),
     &out};

    ATensorIteratorGeneric<T> it(arrays);

    it.forEach([](std::vector<T*> p,size_t)
    {
        *p[2] = *p[0] * *p[1];
    });

    return out;
}

template<typename T> AArray<T> operator/(const AArrayBase<T>& lhs, const AArrayBase<T>& rhs){

    AArray<T> out(lhs.shape());

    std::vector<AArrayBase<T>*> arrays =
    {const_cast<AArrayBase<T>*>(&lhs),
     const_cast<AArrayBase<T>*>(&rhs),
     &out};

    ATensorIteratorGeneric<T> it(arrays);

    it.forEach([](std::vector<T*> p,size_t)
    {
        *p[2] = *p[0] / *p[1];
    });

    return out;
}



} // namespace Alib
#endif
