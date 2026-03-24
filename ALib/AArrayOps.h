//
//  AArrayOps.h
//  ALib
//
//  Created by Giuseppe Coppini on 23/03/26.
//
#ifndef AARRAYOPS_H
#define AARRAYOPS_H

#include "AArray.h"
#include "ABinaryExpr.h"

namespace Alib {

// Generic binary op
template<typename T, typename LHS, typename RHS, typename Func>
auto binaryOp(const AExpr<T,LHS>& lhs, const AExpr<T,RHS>& rhs, Func f) {
    return ABinaryExpr<T,LHS,RHS,Func>(lhs.derived(), rhs.derived(), f);
}

// operator+
template<typename T, typename LHS, typename RHS>
auto operator+(const AExpr<T,LHS>& lhs, const AExpr<T,RHS>& rhs) {
    return binaryOp(lhs,rhs,std::plus<T>());
}

// operator-
template<typename T, typename LHS, typename RHS>
auto operator-(const AExpr<T,LHS>& lhs, const AExpr<T,RHS>& rhs) {
    return binaryOp(lhs,rhs,std::minus<T>());
}

// operator*
template<typename T, typename LHS, typename RHS>
auto operator*(const AExpr<T,LHS>& lhs, const AExpr<T,RHS>& rhs) {
    return binaryOp(lhs,rhs,std::multiplies<T>());
}

// operator/
template<typename T, typename LHS, typename RHS>
auto operator/(const AExpr<T,LHS>& lhs, const AExpr<T,RHS>& rhs) {
    return binaryOp(lhs,rhs,std::divides<T>());
}



} // !namespace Alib
#endif // !AARRAYOPS_H
