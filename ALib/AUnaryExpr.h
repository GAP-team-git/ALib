//
//  AUnaryExpr.h
//  ALib
//
//  Created by Giuseppe Coppini on 23/03/26.
//

#ifndef AUNARYEXPR_H
#define AUNARYEXPR_H

#include "AExpr.h"
#include <functional>

namespace Alib {

template<typename T, typename LHS, typename Op>
class AUnaryExpr : public AExpr<T, AUnaryExpr<T,LHS,Op>> {
public:
    AUnaryExpr(const LHS& l, Op op)
    : lhs(l), op(op) {}
    
    T operator[](size_t i) const { return op(lhs[i]); }
    const AShape& shape() const { return lhs.shape(); }
    size_t size() const { return lhs.size(); }
    
private:
    const LHS& lhs;
    Op op;
};

} // !namespace Alib
#endif // !AUNARYEXPR_H
