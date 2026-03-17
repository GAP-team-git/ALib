//
//  AArrayBase.h
//  ALib
//
//  Created by Giuseppe Coppini on 14/03/26.
//

#ifndef AArrayBase_h
#define AArrayBase_h

#include "AObject.h"
#include "AShape.h"

namespace Alib {

template<typename T> class AArray;
template<typename T> class ATensorIteratorGeneric;


template<typename T>
class AArrayBase : public AObject
{
public:

    using value_type = T;

    virtual ~AArrayBase() = default;

    virtual T* raw() noexcept = 0;
    virtual const T* raw() const noexcept = 0;

    T* data() noexcept { return raw() + p_offset; }
    const T* data() const noexcept { return raw() + p_offset; }

    const AShape& shape() const noexcept { return p_shape; }

    size_t size() const noexcept
    {
        return p_shape.totalSize();
    }

    size_t rank() const noexcept
    {
        return p_shape.rank();
    }

    bool is_contiguous() const noexcept
    {
        return p_shape.is_contiguous();
    }

    // ND access

    T& at(const std::vector<size_t>& idx)
    {
        size_t flat = p_offset;

        const auto& strides = p_shape.strides();

        for(size_t i=0;i<idx.size();++i)
            flat += idx[i]*strides[i];

        return raw()[flat];
    }

    const T& at(const std::vector<size_t>& idx) const
    {
        size_t flat = p_offset;

        const auto& strides = p_shape.strides();

        for(size_t i=0;i<idx.size();++i)
            flat += idx[i]*strides[i];

        return raw()[flat];
    }

    template<typename... Idx>
    T& operator()(Idx... idx)
    {
        return at({static_cast<size_t>(idx)...});
    }

    template<typename... Idx>
    const T& operator()(Idx... idx) const
    {
        return at({static_cast<size_t>(idx)...});
    }

    // materialize (copy view -> array)

    AArray<T> materialize() const
    {
        AArray<T> out(p_shape);

        std::vector<AArrayBase<T>*> arrays =
        {const_cast<AArrayBase<T>*>(this), &out};

        ATensorIteratorGeneric<T> it(arrays);

        it.forEach([](std::vector<T*> p,size_t)
        {
            *p[1] = *p[0];
        });

        return out;
    }
    
    AArray<T> operator+=(const AArrayBase<T>& rhs)
    {
        AArray<T> result(this->shape());

        std::vector<AArrayBase<T>*> arrays =
        {const_cast<AArrayBase<T>*>(this),
         const_cast<AArrayBase<T>*>(&rhs),
         &result};

        ATensorIteratorGeneric<T> it(arrays);

        it.forEach([](std::vector<T*> p,size_t)
        {
            *p[2] = *p[0] + *p[1];
        });

        return result;
    }
    AArray<T> operator-=(const AArrayBase<T>& rhs)
    {
        AArray<T> result(this->shape());

        std::vector<AArrayBase<T>*> arrays =
        {const_cast<AArrayBase<T>*>(this),
         const_cast<AArrayBase<T>*>(&rhs),
         &result};

        ATensorIteratorGeneric<T> it(arrays);

        it.forEach([](std::vector<T*> p,size_t)
        {
            *p[2] = *p[0] - *p[1];
        });

        return result;
    }

    AArray<T> operator*=(const AArrayBase<T>& rhs)
    {
        AArray<T> result(this->shape());

        std::vector<AArrayBase<T>*> arrays =
        {const_cast<AArrayBase<T>*>(this),
         const_cast<AArrayBase<T>*>(&rhs),
         &result};

        ATensorIteratorGeneric<T> it(arrays);

        it.forEach([](std::vector<T*> p,size_t)
        {
            *p[2] = *p[0] * *p[1];
        });

        return result;
    }

    AArray<T> operator/=(const AArrayBase<T>& rhs)
    {
        AArray<T> result(this->shape());

        std::vector<AArrayBase<T>*> arrays =
        {const_cast<AArrayBase<T>*>(this),
         const_cast<AArrayBase<T>*>(&rhs),
         &result};

        ATensorIteratorGeneric<T> it(arrays);

        it.forEach([](std::vector<T*> p,size_t)
        {
            *p[2] = *p[0] / *p[1];
        });

        return result;
    }


    AArrayBase<T>& operator+=(const T scalar)
    {
        std::vector<AArrayBase<T>*> arrays = {this};

        ATensorIteratorGeneric<T> it(arrays);

        it.forEach([&](std::vector<T*> p,size_t)
        {
            *p[0] += scalar;
        });

        return *this;
    }
    
    AArrayBase<T>& operator-=(T scalar)
    {
        std::vector<AArrayBase<T>*> arrays = {this};

        ATensorIteratorGeneric<T> it(arrays);

        it.forEach([&](std::vector<T*> p,size_t)
        {
            *p[0] -= scalar;
        });

        return *this;
    }
    
    AArrayBase<T>& operator*=(T scalar)
    {
        std::vector<AArrayBase<T>*> arrays = {this};

        ATensorIteratorGeneric<T> it(arrays);

        it.forEach([&](std::vector<T*> p,size_t)
        {
            *p[0] *= scalar;
        });

        return *this;
    }
    
    AArrayBase<T>& operator/=(T scalar)
    {
        std::vector<AArrayBase<T>*> arrays = {this};

        ATensorIteratorGeneric<T> it(arrays);

        it.forEach([&](std::vector<T*> p,size_t)
        {
            *p[0] /= scalar;
        });

        return *this;
    }
    
protected:

    AShape p_shape;
    size_t p_offset = 0;
    

};

} //end namespace ALib

#endif /* AArrayBase_h */
