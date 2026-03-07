//
//  main.cpp
//  test
//
//  Created by Giuseppe Coppini on 25/02/26.
//
#include <iostream>
#include "AShape.h"

using namespace Alib;

void printShapeTable(const AShape& s, const std::string& name) {
    std::cout << "Shape " << name << ":\n";
    std::cout << "  Dim   Stride  Offset\n";
    auto dims = s.dims();
    auto strides = s.strides();
    for (size_t i = 0; i < dims.size(); ++i) {
        std::cout << "  " << dims[i] << "     " << strides[i];
        if (i == 0) std::cout << "       " << s.offset();
        std::cout << "\n";
    }
    std::cout << std::endl;
}

int main() {
    AShape a({2,3});
    AShape b({1,3});
    AShape c({2,1});

    std::cout << "a: " << a.numpy_str() << std::endl;
    std::cout << "b: " << b.numpy_str() << std::endl;
    std::cout << "c: " << c.numpy_str() << std::endl;

    // Slice
    AShape a_slice = a.sliceShape({0,1},{2,3},{1,1});
    std::cout << "a_slice: " << a_slice.numpy_str() << std::endl;

    // Transpose
    a.transpose({1,0});
    std::cout << "a_transposed: " << a.numpy_str() << std::endl;

    // Broadcast
    AShape bcast = AShape::broadcast(b,c);
    std::cout << "broadcasted: " << bcast.numpy_str() << std::endl;
    return 0;
}
