////
////  ATensorIterator.hpp
////  ALib
////
////  Created by Giuseppe Coppini on 08/03/26.
////
//
//#ifndef ATensorIterator_h
//#define ATensorIterator_h
//#include "AArray.h"
//#include "Abroadcast.h"
//
//
//
//
//namespace Alib{
//
//
//template<typename T>
//class ATensorIterator
//{
//public:
//    
//    ATensorIterator(
//                   const AArray<T>& A,
//                   const AArray<T>& B,
//                   AArray<T>& C,
//                   const BroadcastInfo& bc)
//    :
//    pp_Adata(A.raw()),
//    pp_Bdata(B.raw()),
//    pp_Cdata(C.raw()),
//    pp_dims(bc.dims),
//    pp_strideA(bc.strideA),
//    pp_strideB(bc.strideB),
//    pp_strideC(C.getShape().strides())
//    {
//        pp_index.resize(pp_dims.size(),0);
//        
//        pp_offA = A.getShape().offset();
//        pp_offB = B.getShape().offset();
//        pp_offC = C.getShape().offset();
//    }
//    
//    bool next()
//    {
//        for(int i=pp_dims.size()-1;i>=0;i--)
//        {
//            pp_index[i]++;
//            
//            pp_offA += pp_strideA[i];
//            pp_offB += pp_strideB[i];
//            pp_offC += pp_strideC[i];
//            
//            if(pp_index[i] < pp_dims[i]) return true;
//            
//            pp_offA -= pp_dims[i]*pp_strideA[i];
//            pp_offB -= pp_dims[i]*pp_strideB[i];
//            pp_offC -= pp_dims[i]*pp_strideC[i];
//            
//            pp_index[i]=0;
//        }
//        
//        return false;
//    }
//    
//    T& a(){ return const_cast<T&>(pp_Adata[pp_offA]); }
//    T& b(){ return const_cast<T&>(pp_Bdata[pp_offB]); }
//    T& c(){ return pp_Cdata[pp_offC]; }
//    
//private:
//    
//    const T* pp_Adata;
//    const T* pp_Bdata;
//    T* pp_Cdata;
//    
//    std::vector<size_t> pp_dims;
//    std::vector<size_t> pp_strideA;
//    std::vector<size_t> pp_strideB;
//    std::vector<size_t> pp_strideC;
//    
//    std::vector<size_t> pp_index;
//    
//    size_t pp_offA;
//    size_t pp_offB;
//    size_t pp_offC;
//};
//
//} // ALib end
//#endif /* ATensorIterator_h */
