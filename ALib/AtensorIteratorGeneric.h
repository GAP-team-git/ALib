//
//  AtensorIteratorGeneric.h
//  ALib
//
//  Created by Giuseppe Coppini on 14/03/26.
//

#ifndef AtensorIteratorGeneric_h
#define AtensorIteratorGeneric_h

namespace Alib{
// -------------------
// Generic Tensor Iterator
// -------------------
//piccolo miglioramento: evitare allocazione per ptrs.
//
//std::array<T*,MAX_ARRAYS>
//
//oppure buffer riutilizzato.

template<typename T>
class AArray;

#define MAX_ARRAYS 5
template<typename T>
class ATensorIteratorGeneric {
public:
    ATensorIteratorGeneric(const std::vector<AArrayBase<T>*> arrays){
        
        ALIB_SHAPE_ASSERT(!arrays.empty(), AErrorInfoEx(
                                                        AErrors::notInitialized,
                                                        AErrorCategory::array,
                                                        AErrorSeverity::critical,
                                                        "ATensorIteratorGeneric: void parameter array",
                                                        "Check constructor data"
                                                        ));
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
    std::vector<AArrayBase<T>*> pp_arrays;
    AShape pp_shape;
    size_t pp_rank;
};
} // end Alib
#endif /* AtensorIteratorGeneric_h */
