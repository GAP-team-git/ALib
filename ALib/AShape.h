//
//  AShape.h
//  ALib
//
//  Created by Giuseppe Coppini on 03/03/26.
//
// AShape.h
#ifndef ASHAPE_H
#define ASHAPE_H
namespace Alib {
class AShape {
public:
    AShape() = default;
    explicit AShape(const std::vector<size_t>& dims)
        : p_dims(dims) { calcStrides(); }

    // Costruttore con stride esplicito (per slice)
    AShape(const std::vector<size_t>& dims, const std::vector<size_t>& strides)
        : p_dims(dims), p_strides(strides) {}

    size_t rank() const noexcept { return p_dims.size(); }
    const std::vector<size_t>& dims() const noexcept { return p_dims; }
    const std::vector<size_t>& strides() const noexcept { return p_strides; }

    void setDims(const std::vector<size_t>& dims) { p_dims = dims; calcStrides(); }
    void setStrides(const std::vector<size_t>& s) { p_strides = s; }

    size_t totalSize() const {
        return std::accumulate(p_dims.begin(), p_dims.end(), 1ULL, std::multiplies<size_t>());
    }

    // indice lineare con strides già corrette
    size_t flatIndex(const std::vector<size_t>& idx) const {
        if(idx.size() != rank())
            throw std::runtime_error("AShape::flatIndex rank mismatch");
        size_t offset = 0;
        for(size_t i=0;i<rank();++i){
            if(idx[i] >= p_dims[i])
                throw std::out_of_range("AShape::flatIndex index out of bounds");
            offset += idx[i]*p_strides[i];
        }
        return offset;
    }

    friend bool operator==(const AShape& a, const AShape& b) {
            return a.p_dims == b.p_dims;
        }

        friend bool operator!=(const AShape& a, const AShape& b) {
            return !(a == b);
        }
    
    // sliceShape aggiornato: restituisce stride reale
    AShape sliceShape(const std::vector<size_t>& start,
                      const std::vector<size_t>& stop,
                      const std::vector<size_t>& step) const {
        if(start.size()!=rank() || stop.size()!=rank() || step.size()!=rank())
            throw std::invalid_argument("sliceShape: size mismatch");

        std::vector<size_t> newDims(rank());
        std::vector<size_t> newStrides(rank());
        for(size_t i=0;i<rank();++i){
            if(step[i]==0) throw std::invalid_argument("sliceShape: step=0");
            newDims[i] = (stop[i]<=start[i]?0:(stop[i]-start[i]+step[i]-1)/step[i]);
            newStrides[i] = p_strides[i]*step[i];  // stride corretto per la slice
        }
        return AShape(newDims, newStrides);
    }
    // -------------------
    // Transpose
    // -------------------
    void transpose(const std::vector<size_t>& axes) {
        if(axes.size() != rank())
            throw std::invalid_argument("transpose: axes size mismatch");

        std::vector<size_t> newDims(rank());
        std::vector<size_t> newStrides(rank());
        for(size_t i = 0; i < rank(); ++i){
            newDims[i] = p_dims[axes[i]];
            newStrides[i] = p_strides[axes[i]]; // manteniamo stride corretto
        }

        p_dims = std::move(newDims);
        p_strides = std::move(newStrides);
    }
    // -------------------
    // Broadcasting
    // -------------------
    static AShape broadcast(const AShape& a, const AShape& b) {
        size_t r1 = a.rank(), r2 = b.rank();
        size_t rmax = std::max(r1, r2);

        std::vector<size_t> dims(rmax, 1);

        // confronto dimensione per dimensione, partendo da destra
        for (int i = int(rmax) - 1; i >= 0; --i) {
            size_t da = (i >= int(rmax - r1)) ? a.p_dims[i - (rmax - r1)] : 1;
            size_t db = (i >= int(rmax - r2)) ? b.p_dims[i - (rmax - r2)] : 1;

            if (da != db && da != 1 && db != 1)
                throw std::invalid_argument("broadcast: incompatible shapes");

            dims[i] = std::max(da, db);
        }

        return AShape(dims);
    }

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
