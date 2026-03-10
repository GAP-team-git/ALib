//
//  Aelementwise_ops.h
//  ALib
//
//  Created by Giuseppe Coppini on 08/03/26.
//

#ifndef Aelementwise_ops_h
#define Aelementwise_ops_h


#include "Abroadcast.h"

namespace  Alib {

template<typename T,typename Op>
void binary_op(
               const AArray<T>& A,
               const AArray<T>& B,
               AArray<T>& C,
               Op op)
{
    auto bc = broadcast(A.getShape(),B.getShape());
    
    ATensorIteratorGeneric<T> it(A,B,C,bc);
    
    do
    {
        it.c() = op(it.a(),it.b());
    }
    while(it.next());
}

} // Alib end
#endif /* Aelementwise_ops_h */
