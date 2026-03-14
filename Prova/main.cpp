//
//  main.cpp
//  test
//
//  Created by Giuseppe Coppini on 25/02/26.
//

#include <iostream>
#include "AArray.h"
#include "AShape.h"
#include "AConfig.h"
#include "AAssert.h"

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
    
    AArray<float> A(3);
    
    AArray<float> B(2,3);
    
    std::cout << std::endl;
    for(int i = 0;  i < 2; i++ ){
        for(int j = 0;  j < 3; j++ ){
            B[i][j] = i+j;
            std::cout <<  B[i][j] << " ";
        }
        std::cout << std::endl;
    }
    
    auto a = B.transpose();
    
    std::cout << a.shape()<< "\n "<<a<< std::endl;
    
    
    for(int i = 0;  i < 3; i++ ){
        try {
            A[i] = i+1;
        } catch (Alib::AException e) {
            printError(e.info());
        }
            
    }
    
    for(int i = 0;  i < 3; i++ ){
        
            std::cout <<  A[i] << " ";
        
    }
    std::cout << std::endl;
    return 0;
    for(int i = 0;  i < 3; i++ ){
        for(int j = 0;  j < 2; j++ )
            std::cout <<  A[i][j] << " ";
        std::cout << std::endl;
    }
    
    auto C = A.slice({0,0}, {2,1});
 //   auto C = A.slice({0,0}, {2,1}) + B;
    std::cout << C.size() << std::endl;
//    C.reshape({6});
//    for(int i = 0;  i < C.size(); i++ ){
//            std::cout <<  C[i] << " ";
//        std::cout << std::endl;
//    }
//    
    
    return EXIT_SUCCESS;
}
