//
//  ABinaryExpr.h
//  ALib
//
//  Created by Giuseppe Coppini on 23/03/26.
//
#ifndef ABINARYEXPR_H
#define ABINARYEXPR_H

#include "AExpr.h"
#include "AShape.h"
#include <functional>

namespace Alib {

template<typename T, typename LHS, typename RHS, typename Op>
class ABinaryExpr : public AExpr<T, ABinaryExpr<T,LHS,RHS,Op>> {
public:
    ABinaryExpr(const LHS& l, const RHS& r, Op op)
        : lhs(l), rhs(r), op(op) {}

    T operator[](size_t i) const { return op(lhs[i], rhs[i]); }
    const AShape& shape() const { return lhs.shape(); }
    size_t size() const { return lhs.size(); }

private:
    const LHS& lhs;
    const RHS& rhs;
    Op op;
};


} // namespace Alib
#endif // !ABINARYEXPR_H
