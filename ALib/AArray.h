//
//  AArray.h
//  ALib
//
//  Created by Giuseppe Coppini on 23/03/26.
//

#ifndef AARRAY_H
#define AARRAY_H

#include "AArrayBase.h"
#include "AErrors.h"
#include "AExpr.h"
#include "ATensorIterator.h"
#include <memory>
#include <vector>
#include <numeric>
#include <functional>
#include <algorithm>

namespace Alib {

template<typename T>
class AArray : public AArrayBase<T>, public AExpr<T,AArray<T>> {
public:
    using Base = AArrayBase<T>;
    
    // -------------------
    // Constructors
    // -------------------
    AArray() = default;
    explicit AArray(const std::vector<size_t>& dims){
        Base::p_shape = AShape(dims);
        pp_dataPtr = std::make_shared<std::vector<T>>(Base::p_shape.totalSize());
    }
    explicit AArray(const AShape& shape){
        Base::p_shape = shape;
        pp_dataPtr = std::make_shared<std::vector<T>>(Base::p_shape.totalSize());
    }
    
    
    T* raw() noexcept override { return pp_dataPtr->data(); }
    const T* raw() const noexcept override { return pp_dataPtr->data(); }
    size_t size() const noexcept { return pp_dataPtr->size(); }
    [[nodiscard]] size_t type_id() const noexcept override { return type_id_of<AArray<T>>(); }
    const AShape& shape() const { return Base::shape(); }
    bool is_contiguous() const noexcept { return shape().is_contiguous(); }
    
    // -------------------
    // operator[]
    // -------------------
    T operator[](size_t i) const { return (*pp_dataPtr)[i]; }
    T& operator[](size_t i) { return (*pp_dataPtr)[i]; }
    
    std::vector<size_t> unravel_index(size_t flat) const
    {
        return shape().unravel_index(flat);
    }
    
    std::unique_ptr<AObject> clone() const override
    {
        
        AArray<T> out(this->p_shape);
        
        
        std::vector<AArrayBase<T>*> arrays =
        {const_cast<AArray<T>*>(this),&out};
        
        ATensorIterator<T> it(arrays);
        
        it.forEach([](auto& p, size_t)
                   {
            *p[1] = *p[0];
        });
        
        return std::make_unique<AArray<T>>(out);
    }
    // -------------------
    // Comparison
    // -------------------
    
    [[nodiscard]] std::string str() const override {
        std::string s = getClassName() + "(rank=" + std::to_string(shape().rank())+ ", type=" + typeid(T).name() + ", dims=[";
        for (size_t i = 0; i < shape().dims().size(); ++i) {
            s += std::to_string(shape().dims()[i]);
            if (i + 1 < shape().dims().size()) s += ",";
        }
        s += "], size=" + std::to_string(shape().totalSize())+ ", byte_size=" + std::to_string(shape().totalSize()*sizeof(T)) +  " strides=[";
        for (size_t i = 0; i < shape().strides().size(); ++i) {
            s += std::to_string(shape().strides()[i]);
            if (i + 1 < shape().strides().size()) s += ",";
        }
        
        s += "], offset=" + std::to_string(shape().offset()) + ", contiguous=" + (shape().is_contiguous()?"true":"false") + ")";
        return s;
    }
    
    [[nodiscard]] virtual bool m_compare(const AObject& obj) const noexcept override { return m_allEqual(obj); }
    
    [[nodiscard]] virtual bool m_allEqual(const AObject& obj) const noexcept override {
        auto other = dynamic_cast<const AArray<T>*>(&obj);
        if(!other) return false;
        if(this==other) return true;
        if(shape().dims()!=other->shape().dims()) return false;
        
        if(is_contiguous() && other->is_contiguous())
            return std::equal(raw(), raw()+size(), other->raw());
        
        std::vector<AArrayBase<T>*> arrays={const_cast<AArray<T>*>(this), const_cast<AArray<T>*>(other)};
        ATensorIterator<T> it(arrays);
        bool eq=true;
        it.forEach([&eq](auto& ptrs,size_t){ if(*ptrs[0]!=*ptrs[1]) eq=false; });
        return eq;
    }
    
    [[nodiscard]] virtual bool m_allClose(const AObject& obj,double eps) const noexcept override {
        auto other = dynamic_cast<const AArray<T>*>(&obj);
        if(!other) return false;
        if(this==other) return true;
        if(shape().dims()!=other->shape().dims()) return false;
        
        std::vector<AArrayBase<T>*> arrays={const_cast<AArray<T>*>(this), const_cast<AArray<T>*>(other)};
        ATensorIterator<T> it(arrays);
        bool close=true;
        it.forEach([eps,&close](auto& ptrs,size_t){ if(std::fabs(*ptrs[0]-*ptrs[1])>eps) close=false; });
        return close;
    }
    
    
    AArrayBase<T>& fill(T scalar)
    {
        std::vector<AArrayBase<T>*> arrays = {this};
        
        ATensorIterator<T> it(arrays);
        
        it.forEach([&](auto& p,size_t){*p[0] = scalar;});
        
        return *this;
    }
    
    AArrayBase<T>& randomize(T p1,T p2)
    {
        std::vector<AArrayBase<T>*> arrays = {this};
        
        ATensorIterator<T> it(arrays);
        static thread_local std::mt19937_64 rng(
                                                std::chrono::high_resolution_clock::now().time_since_epoch().count()
                                                );
        
        std::normal_distribution<> dist(p1,p2);
        
        
        it.forEach([&](auto& p,size_t)
                   {
            *p[0] = dist(rng);
        });
        
        return *this;
    }
    // comparison with tolerance
    virtual std::string getClassName() const noexcept override{return "AArray";}
    
    // -------------------
    // Assignment from expression
    // -------------------
    template<typename Expr>
    AArray& operator=(const AExpr<T,Expr>& expr)
    {
        const Expr& d = expr.derived();
        
        if(this->shape().totalSize() != d.size())
            this->p_shape = d.shape();
        
        if(!pp_dataPtr || pp_dataPtr->size() != d.size())
            pp_dataPtr = std::make_shared<std::vector<T>>(d.size());
        
        for(size_t i=0;i<d.size();++i)
            (*pp_dataPtr)[i] = d[i];
        
        return *this;
    }
    
    
    // -------------------
    // Inplace ops
    // -------------------
    template<typename Expr>
    AArray& operator+=(const AExpr<T,Expr>& rhs)
    {
        const Expr& r = rhs.derived();
        for(size_t i=0;i<r.size();++i)
            (*pp_dataPtr)[i] += r[i];
        return *this;
    }
    
    template<typename Expr>
    AArray& operator-=(const AExpr<T,Expr>& rhs)
    {
        const Expr& r = rhs.derived();
        for(size_t i=0;i<r.size();++i)
            (*pp_dataPtr)[i] -= r[i];
        return *this;
    }
    
    template<typename Expr>
    AArray& operator*=(const AExpr<T,Expr>& rhs)
    {
        const Expr& r = rhs.derived();
        for(size_t i=0;i<r.size();++i)
            (*pp_dataPtr)[i] *= r[i];
        return *this;
    }
    
    template<typename Expr>
    AArray& operator/=(const AExpr<T,Expr>& rhs)
    {
        const Expr& r = rhs.derived();
        for(size_t i=0;i<r.size();++i)
            (*pp_dataPtr)[i] /= r[i];
        return *this;
    }
    
private:
    std::shared_ptr<std::vector<T>> pp_dataPtr;
};

template<typename T, typename Expr>
AArray<T> eval(const AExpr<T,Expr>& e) {
    const Expr& d = e.derived();
    AArray<T> out(d.shape());
    for(size_t i=0;i<d.size();++i)
        out.raw()[i] = d[i];
    return out;
}

//
// Aggregators
//

// stndard implementation
//template<typename T, typename Expr>
//T sum(const AExpr<T,Expr>& expr)
//{
//    const Expr& d = expr.derived();
//    
//    T total{};
//    for(size_t i=0;i<d.size();++i)
//        total += d[i];
//    
//    return total;
//}
//
//template<typename T, typename Expr>
//double mean(const AExpr<T,Expr>& expr)
//{
//    const Expr& d = expr.derived();
//    return static_cast<double>(sum(expr)) / d.size();
//}
//
//template<typename T, typename Expr>
//T max(const AExpr<T,Expr>& expr)
//{
//    const Expr& d = expr.derived();
//    
//    T m = d[0];
//    for(size_t i=1;i<d.size();++i)
//        if(d[i] > m) m = d[i];
//    
//    return m;
//}
//
//template<typename T, typename Expr>
//T min(const AExpr<T,Expr>& expr)
//{
//    const Expr& d = expr.derived();
//    
//    T m = d[0];
//    for(size_t i=1;i<d.size();++i)
//        if(d[i] < m) m = d[i];
//    
//    return m;
//}
//
//template<typename T, typename Expr>
//size_t argmax(const AExpr<T,Expr>& expr)
//{
//    const Expr& d = expr.derived();
//    
//    size_t idx = 0;
//    T maxv = d[0];
//    
//    for(size_t i=1;i<d.size();++i)
//    {
//        if(d[i] > maxv)
//        {
//            maxv = d[i];
//            idx = i;
//        }
//    }
//    
//    return idx;
//}
//
//template<typename T, typename Expr>
//size_t argmin(const AExpr<T,Expr>& expr)
//{
//    const Expr& d = expr.derived();
//    
//    size_t idx = 0;
//    T minv = d[0];
//    
//    for(size_t i=1;i<d.size();++i)
//    {
//        if(d[i] < minv)
//        {
//            minv = d[i];
//            idx = i;
//        }
//    }
//    
//    return idx;
//}

template<typename T, typename Expr, typename Reducer>
auto reduce(const AExpr<T,Expr>& expr, Reducer r, T init)
{
    const Expr& d = expr.derived();
    T accum = init;

    for(size_t i = 0; i < d.size(); ++i)
        accum = r(accum, d[i]);

    return accum;
}

template<typename T, typename Expr, typename Reducer>
AArray<T> reduce_nd(const AExpr<T,Expr>& expr,
                    Reducer r,
                    T init,
                    const std::vector<size_t>& axes = {})
{
    const Expr& d = expr.derived();
    const AShape& shape = d.shape();
    size_t rank = shape.rank();

    // Se axes vuoto, riduce tutti gli assi
    std::vector<size_t> red_axes = axes.empty() ? std::vector<size_t>(rank) : axes;
    if(axes.empty())
        for(size_t i=0;i<rank;++i) red_axes[i] = i;

    // Shape output
    std::vector<size_t> out_dims;
    for(size_t i=0;i<rank;++i)
        if(std::find(red_axes.begin(), red_axes.end(), i) == red_axes.end())
            out_dims.push_back(shape.dims()[i]);

    AArray<T> out(out_dims.empty() ? std::vector<size_t>{1} : out_dims);
    std::fill(out.raw(), out.raw() + out.size(), init);

    // Iteratore ND
    ATensorIterator<T> it({ &out }, { &d });

    it.forEach([&r](auto& ptrs, size_t){
        *ptrs[0] = r(*ptrs[0], *ptrs[1]);
    });

    return out;
}

template<typename T, typename Expr>
T sum(const AExpr<T,Expr>& expr)
{
    return reduce(expr, [](T a,T b){ return a+b; }, T{});
}

template<typename T, typename Expr>
double mean(const AExpr<T,Expr>& expr)
{
    return static_cast<double>(sum(expr)) / expr.derived().size();
}

template<typename T, typename Expr>
T max(const AExpr<T,Expr>& expr)
{
    return reduce(expr, [](T a,T b){ return a>b?a:b; }, std::numeric_limits<T>::lowest());
}

template<typename T, typename Expr>
T min(const AExpr<T,Expr>& expr)
{
    return reduce(expr, [](T a,T b){ return a<b?a:b; }, std::numeric_limits<T>::max());
}

template<typename T, typename Expr>
size_t argmax(const AExpr<T,Expr>& expr)
{
    const Expr& d = expr.derived();
    size_t idx = 0;
    T best = d[0];

    for(size_t i=1;i<d.size();++i)
        if(d[i] > best)
        {
            best = d[i];
            idx = i;
        }

    return idx;
}

template<typename T, typename Expr>
size_t argmin(const AExpr<T,Expr>& expr)
{
    const Expr& d = expr.derived();
    size_t idx = 0;
    T best = d[0];

    for(size_t i=1;i<d.size();++i)
        if(d[i] < best)
        {
            best = d[i];
            idx = i;
        }

    return idx;
}

template<typename T, typename Expr>
AArray<T> sum_nd(const AExpr<T,Expr>& expr, const std::vector<size_t>& axes = {})
{
    return reduce_nd(expr, [](T a,T b){ return a+b; }, T{0}, axes);
}

template<typename T, typename Expr>
AArray<T> max_nd(const AExpr<T,Expr>& expr, const std::vector<size_t>& axes = {})
{
    return reduce_nd(expr, [](T a,T b){ return std::max(a,b); },
                     std::numeric_limits<T>::lowest(), axes);
}

template<typename T, typename Expr>
AArray<T> min_nd(const AExpr<T,Expr>& expr, const std::vector<size_t>& axes = {})
{
    return reduce_nd(expr, [](T a,T b){ return std::min(a,b); },
                     std::numeric_limits<T>::max(), axes);
}

template<typename T, typename Expr>
AArray<double> mean_nd(const AExpr<T,Expr>& expr, const std::vector<size_t>& axes = {})
{
    auto s = sum_nd(expr, axes);
    size_t count = 1;
    for(auto a: s.shape().dims()) count *= a;
    return s / static_cast<double>(count);
}

template<typename T, typename Expr>
std::vector<size_t> argmax_nd(const AExpr<T,Expr>& expr)
{
    const Expr& d = expr.derived();
    ALIB_ASSERT(d.size() > 0, AErrorInfoEx(
                                           ArrayErrors::emptyArray,
                                           AErrorCategory::array,
                                           AErrorSeverity::critical,
                                           "argmax_nd: empty array",
                                           "Check shape parameters"
                                           ));

    size_t flat = 0;
    T best = d[0];

    for(size_t i = 1; i < d.size(); ++i)
        if(d[i] > best)
        {
            best = d[i];
            flat = i;
        }

    return d.shape().unravel_index(flat);
}

template<typename T, typename Expr>
std::vector<size_t> argmin_nd(const AExpr<T,Expr>& expr)
{
    const Expr& d = expr.derived();
    ALIB_ASSERT(d.size() > 0, AErrorInfoEx(
                                           ArrayErrors::emptyArray,
                                           AErrorCategory::array,
                                           AErrorSeverity::critical,
                                           "argminx_nd: empty array",
                                           "Check shape parameters"
                                           ));

    size_t flat = 0;
    T best = d[0];

    for(size_t i = 1; i < d.size(); ++i)
        if(d[i] < best)
        {
            best = d[i];
            flat = i;
        }

    return d.shape().unravel_index(flat);
}
} // !namespace Alib
#endif // !AARRAY_H
