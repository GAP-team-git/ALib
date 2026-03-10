//
//  main.cpp
//  test
//
//  Created by Giuseppe Coppini on 25/02/26.
//
#include <iostream>
#include "AArray.h"
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
    
    AArray<float> A(3,3);
    AArray<float> B(3,3);
    for(int i = 0;  i < 3; i++ ){
        for(int j = 0;  j < 3; j++ ){
            A[i][j] = 1;
            B[i][j] = i+j;
        }
   }
    for(int i = 0;  i < 3; i++ ){
        for(int j = 0;  j < 3; j++ )
            std::cout <<  A[i][j] << " ";
        std::cout << std::endl;
    }
    std::cout << std::endl;
    for(int i = 0;  i < 3; i++ ){
        for(int j = 0;  j < 3; j++ )
            std::cout <<  B[i][j] << " ";
        std::cout << std::endl;
    }
    auto C = A + 5;
    std::cout << std::endl;
    for(int i = 0;  i < 3; i++ ){
        for(int j = 0;  j < 3; j++ )
            std::cout <<  C[i][j] << " ";
        std::cout << std::endl;
    }
    std::cout << std::endl;
    std::cout << A.sum() << std::endl;
    std::cout << B.mean() << std::endl;
    std::cout << B.max() << std::endl;
    std::cout << B.min() << std::endl;
    
    if(A==B) std::cout << "A == B" <<std::endl;
    
    return 0;
}
