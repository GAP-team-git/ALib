//
//  Aoperators.h
//  ALib
//
//  Created by Giuseppe Coppini on 08/03/26.
//
//
//#ifndef Aoperators_h
//#define Aoperators_h
//
//
//#include "AArray.h"
//
//namespace Alib {
//
//template<typename Op>
//AArray<T> binaryOp(const AArray<T>& other, Op op) const {
//    AArray<T> result(AShape::broadcast(shape(), other.shape()));
//    std::vector<AArray<T>*> arrays = {const_cast<AArray<T>*>(this),
//                                      const_cast<AArray<T>*>(&other),
//                                      &result};
//    ATensorIteratorGeneric<T> it(arrays);
//    it.forEach([&op](std::vector<T*> ptrs, size_t){
//        *ptrs[2] = op(*ptrs[0], *ptrs[1]);
//    });
//    return result;
//}
//
//AArray<T> operator+(const AArray<T>& other) const { return binaryOp(other,std::plus<T>()); }
//AArray<T> operator-(const AArray<T>& other) const { return binaryOp(other,std::minus<T>()); }
//AArray<T> operator*(const AArray<T>& other) const { return binaryOp(other,std::multiplies<T>()); }
//AArray<T> operator/(const AArray<T>& other) const { return binaryOp(other,std::divides<T>()); }
//
//} // Alib end
//#endif /* Aoperators_h */
