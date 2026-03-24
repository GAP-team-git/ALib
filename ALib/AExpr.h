//
//  AExpr.h
//  ALib
//
//  Created by Giuseppe Coppini on 23/03/26.
//

#ifndef AEXPR_H
#define AEXPR_H

#include "AShape.h"

namespace Alib {

template<typename T, typename Derived>
class AExpr {
public:
    const Derived& derived() const { return *static_cast<const Derived*>(this); }
    Derived& derived() { return *static_cast<Derived*>(this); }

    const AShape& shape() const { return derived().shape(); }
    size_t size() const { return derived().size(); }
};

} // !namespace Alib

#endif // !AEXPR_H
