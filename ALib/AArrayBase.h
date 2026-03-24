//
//  AArrayBase.h
//  ALib
//
//  Created by Giuseppe Coppini on 20/03/26.
//

#ifndef AARRAYBASE_H
#define AARRAYBASE_H

#include <vector>
#include <memory>
#include <functional>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <iostream>
#include "AObject.h"
#include "AShape.h"
#include "AErrors.h"
#include "AExpr.h"
#include "ABinaryExpr.h"
#include "ATensorIterator.h"


namespace Alib {

template<typename T>
class AArrayBase : public AObject {
public:
    using value_type = T;
    virtual ~AArrayBase() = default;
    
    virtual T* raw() noexcept = 0;
    virtual const T* raw() const noexcept = 0;
    
    
    const AShape& shape() const noexcept { return p_shape; }
    size_t size() const noexcept { return p_shape.totalSize(); }
    size_t rank() const noexcept { return p_shape.rank(); }
    bool is_contiguous() const noexcept { return p_shape.is_contiguous(); }
    
    T& at(const std::vector<size_t>& idx){
        size_t flat = p_offset;
        const auto& strides = p_shape.strides();
        for(size_t i=0;i<idx.size();++i) flat += idx[i]*strides[i];
        return raw()[flat];
    }
    const T& at(const std::vector<size_t>& idx) const {
        size_t flat = p_offset;
        const auto& strides = p_shape.strides();
        for(size_t i=0;i<idx.size();++i) flat += idx[i]*strides[i];
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
    
    T& operator()(const std::vector<size_t>& idx)
    {
        return at(idx);
}

const T& operator()(const std::vector<size_t>& idx) const
{
    return at(idx);
}

    
void print(std::ostream& os,char separator=' '){
    
    size_t total = shape().totalSize();
    auto dims =shape().dims();
    size_t rank = dims.size();
    
    std::vector<size_t> idx(rank, 0);
    
    for (size_t i = 0; i < total; ++i){
        
        os << at(idx) << separator;
        
        for (int d = int(rank) - 1; d >= 0; --d){
            if (++idx[d] < dims[d])break;
            if(i<total-1)os <<std::endl;
            idx[d] = 0;
        }
    }
    os <<std::endl;
    return;
}

void printNDIndex(std::ostream& os,std::vector<size_t> idx, char separator=','){
    os << "[";
    for(size_t i = 0; i< idx.size(); i++){
        if(i<idx.size()-1) os << idx[i] <<separator;
        else os << idx[i];
    }
    os << "]";
}

protected:
AShape p_shape;
size_t p_offset = 0;
};


} // namespace Alib

#endif // !AARRAYBASE_H
