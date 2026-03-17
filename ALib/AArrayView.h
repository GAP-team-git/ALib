//
//  AArrayView.h
//  ALib
//
//  Created by Giuseppe Coppini on 14/03/26.
//

#ifndef AArrayView_h
#define AArrayView_h

#include "AArrayBase.h"

namespace Alib {

template<typename T>
class AArrayView : public AArrayBase<T>
{
public:
    using Base = AArrayBase<T>;

        AArrayView() = default;

        AArrayView(
            std::shared_ptr<std::vector<T>> data,
            const AShape& shape,
            size_t offset)
        {
            this->p_shape = shape;
            this->p_offset = offset;
            pp_dataPtr = data;
        }

        T* raw() noexcept override
        {
            return pp_dataPtr->data();
        }

        const T* raw() const noexcept override
        {
            return pp_dataPtr->data();
        }

    [[nodiscard]] virtual bool m_compare(const AObject& obj) const noexcept override { return m_allEqual(obj); }

    [[nodiscard]] virtual bool m_allEqual(const AObject& obj) const noexcept override {
        auto other = dynamic_cast<const AArrayView<T>*>(&obj);
        if(!other) return false;
        if(this==other) return true;
        

      //  if(is_contiguous() && other->is_contiguous())
        //    return std::equal(raw(), raw()+size(), other->raw());

        std::vector<AArrayBase<T>*> arrays={const_cast<AArrayView<T>*>(this), const_cast<AArrayView<T>*>(other)};
        ATensorIteratorGeneric<T> it(arrays);
        bool eq=true;
        it.forEach([&eq](std::vector<T*> ptrs,size_t){ if(*ptrs[0]!=*ptrs[1]) eq=false; });
        return eq;
    }

    [[nodiscard]] virtual bool m_allClose(const AObject& obj,double eps) const noexcept override {
        auto other = dynamic_cast<const AArrayView<T>*>(&obj);
        if(!other) return false;
        if(this==other) return true;
        //if(shape().dims()!=other->shape().dims()) return false;

        std::vector<AArrayBase<T>*> arrays={const_cast<AArrayView<T>*>(this), const_cast<AArrayView<T>*>(other)};
        ATensorIteratorGeneric<T> it(arrays);
        bool close=true;
        it.forEach([eps,&close](std::vector<T*> ptrs,size_t){ if(std::fabs(*ptrs[0]-*ptrs[1])>eps) close=false; });
        return close;
    }

    std::unique_ptr<AObject> clone() const override
        {

            AArrayView<T> out(this->pp_dataPtr,this->p_shape,this->p_offset);

        
            std::vector<AArrayBase<T>*> arrays =
            {const_cast<AArrayView<T>*>(this),&out};

            ATensorIteratorGeneric<T> it(arrays);

            it.forEach([](std::vector<T*> p,size_t)
            {
                *p[1] = *p[0];
            });

            return std::make_unique<AArrayView<T>>(out);
        }
    
    
    virtual size_t type_id() const noexcept override {return type_id_of<AArrayView<T>>(); }
protected:
    
private:

    std::shared_ptr<std::vector<T>> pp_dataPtr;
    
};

} // end namespace Alib

#endif /* AArrayView_h */
