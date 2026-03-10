////
////  AShapeIterator.hpp
////  ALib
////
////  Created by Giuseppe Coppini on 08/03/26.
////
//
//#ifndef AShapeIterator_h
//#define AShapeIterator_h
//#include "AShape.h"
//
//namespace Alib{
//
//class ShapeIterator{
//public:
//        
//        ShapeIterator(const AShape& s)
//        : pp_shape(s),
//        pp_index(s.rank(),0),
//        pp_flat(s.offset())
//        {}
//        
//        size_t flat_index() const { return pp_flat; }
//        
//        bool next()
//        {
//            auto& dims = pp_shape.dims();
//            auto& strides = pp_shape.strides();
//            
//            for(int i = dims.size()-1; i >= 0; --i)
//            {
//                pp_index[i]++;
//                
//                pp_flat += strides[i];
//                
//                if(pp_index[i] < dims[i])
//                    return true;
//                
//                pp_flat -= dims[i]*strides[i];
//                pp_index[i] = 0;
//            }
//            
//            return false;
//        }
//        
//    private:
//        
//        const AShape& pp_shape;
//        std::vector<size_t> pp_index;
//        size_t pp_flat;
//    };
//} // ALib end
//#endif //  AShapeIterator_h  end
