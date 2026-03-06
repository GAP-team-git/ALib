//
//  AShape.h
//  ALib
//
//  Created by Giuseppe Coppini on 03/03/26.
//

// AShape.h
#ifndef ASHAPE_H
#define ASHAPE_H

#include <vector>
#include <numeric>
#include <stdexcept>
#include <algorithm>
#include <cstddef>
#include <iostream>

namespace Alib {

class AShape {
public:
    AShape() = default;
    explicit AShape(const std::vector<size_t>& dims) : p_dims(dims) { calcStrides(); }

    size_t rank() const noexcept { return p_dims.size(); }
    const std::vector<size_t>& dims() const noexcept { return p_dims; }
    const std::vector<size_t>& strides() const noexcept { return p_strides; }

    void setDims(const std::vector<size_t>& dims) { p_dims = dims; calcStrides(); }
    void setStrides(const std::vector<size_t>& s) { p_strides = s; }

    size_t totalSize() const {
        return std::accumulate(p_dims.begin(), p_dims.end(), 1ULL, std::multiplies<size_t>());
    }

    size_t flatIndex(const std::vector<size_t>& idx) const {
        if(idx.size() != rank())
            throw std::runtime_error("AShape::flatIndex rank mismatch");

        size_t offset = 0;
        for(size_t i=0;i<p_dims.size();++i){
            if(idx[i] >= p_dims[i])
                throw std::out_of_range("AShape::flatIndex index out of bounds");
            offset += idx[i]*p_strides[i];
        }
        return offset;
    }

    // -------------------
    // Slice support
    // -------------------
    AShape sliceShape(const std::vector<size_t>& start,
                      const std::vector<size_t>& stop,
                      const std::vector<size_t>& step) const {
        if(start.size()!=rank() || stop.size()!=rank() || step.size()!=rank())
            throw std::invalid_argument("sliceShape: size mismatch");
        std::vector<size_t> newDims(rank());
        for(size_t i=0;i<rank();++i){
            if(step[i]==0) throw std::invalid_argument("sliceShape: step=0");
            newDims[i] = (stop[i]<=start[i]?0:(stop[i]-start[i]+step[i]-1)/step[i]);
        }
        return AShape(newDims);
    }

    size_t flatIndexFromSlice(size_t idx, const std::vector<size_t>& start, const std::vector<size_t>& step) const {
        size_t linear = 0;
        for(int d=int(rank())-1; d>=0; --d){
            size_t cur = idx % dims()[d];
            linear += (start[d] + cur*step[d])*strides()[d];
            idx /= dims()[d];
        }
        return linear;
    }

    // -------------------
    // Transpose
    // -------------------
    void transpose(const std::vector<size_t>& axes) {
        if(axes.size()!=rank()) throw std::invalid_argument("transpose: axes mismatch");
        std::vector<size_t> newDims(rank());
        for(size_t i=0;i<rank();++i) newDims[i]=p_dims[axes[i]];
        p_dims=newDims;
        calcStrides();
    }

    // -------------------
    // Broadcasting
    // -------------------
    static AShape broadcast(const AShape& a, const AShape& b){
        size_t r1=a.rank(), r2=b.rank();
        size_t rmax=std::max(r1,r2);
        std::vector<size_t> dims(rmax,1);
        for(size_t i=0;i<rmax;++i){
            size_t da=(i<rmax-r1?1:a.p_dims[i-(rmax-r1)]);
            size_t db=(i<rmax-r2?1:b.p_dims[i-(rmax-r2)]);
            if(da!=db && da!=1 && db!=1) throw std::invalid_argument("broadcast: incompatible shapes");
            dims[i]=std::max(da,db);
        }
        return AShape(dims);
    }

    bool operator==(const AShape& other) const { return p_dims==other.p_dims; }
    bool operator!=(const AShape& other) const { return !(*this==other); }

private:
    std::vector<size_t> p_dims;
    std::vector<size_t> p_strides;

    void calcStrides(){
        p_strides.resize(rank());
        if(rank()==0) return;
        p_strides[rank()-1]=1;
        for(int i=int(rank())-2;i>=0;--i)
            p_strides[i] = p_strides[i+1]*p_dims[i+1];
    }
};

} // namespace Alib
#endif
