//
//  Abroadcast.h
//  ALib
//
//  Created by Giuseppe Coppini on 08/03/26.
//

#ifndef Abroadcast_h
#define Abroadcast_h

#include "AShape.h"

namespace Alib {


struct BroadcastInfo
{
    std::vector<size_t> dims;
    std::vector<size_t> strideA;
    std::vector<size_t> strideB;
};

inline BroadcastInfo broadcast(const AShape& A,const AShape& B)
{
    auto dimsA = A.dims();
    auto dimsB = B.dims();
    
    auto stridesA = A.strides();
    auto stridesB = B.strides();
    
    size_t rank = std::max(dimsA.size(),dimsB.size());
    
    BroadcastInfo out;
    
    out.dims.resize(rank);
    out.strideA.resize(rank);
    out.strideB.resize(rank);
    
    for(size_t i=0;i<rank;i++)
    {
        int64_t ia = dimsA.size()-1-i;
        int64_t ib = dimsB.size()-1-i;
        
        size_t da = ia>=0 ? dimsA[ia] : 1;
        size_t db = ib>=0 ? dimsB[ib] : 1;
        
        size_t sa = ia>=0 ? stridesA[ia] : 0;
        size_t sb = ib>=0 ? stridesB[ib] : 0;
        
        size_t idx = rank-1-i;
        
        if(da==db)
        {
            out.dims[idx]=da;
            out.strideA[idx]=sa;
            out.strideB[idx]=sb;
        }
        else if(da==1)
        {
            out.dims[idx]=db;
            out.strideA[idx]=0;
            out.strideB[idx]=sb;
        }
        else if(db==1)
        {
            out.dims[idx]=da;
            out.strideA[idx]=sa;
            out.strideB[idx]=0;
        }
        else
            ALIB_SHAPE_ASSERT(1,AErrorInfoEx(
                                             AErrors::invalidOperation,
                                             AErrorCategory::array,
                                             AErrorSeverity::critical,
                                             "broadcast(): broadcast error",
                                             "Check shape parameters"
                                             ));
    }
    
    return out;
}
} // end Alib

#endif // !Abroadcast_h
