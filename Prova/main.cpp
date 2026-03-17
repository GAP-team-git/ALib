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
    
    AShape s({256,32});
    
    if(s.write("/Users/giuseppe/Desktop/tt/shape.raw"))std::cout << "written "<<std::endl;
    
    AShape sr;
    if(sr.read("/Users/giuseppe/Desktop/tt/shape.raw")){
        std::cout << sr <<std::endl;
    }
    
    
    
    AArray<float> img(512,512),im2(512,512);
    img += 1;
    im2 += 3;
    auto patch = img.slice({100,100},{164,164},{1,1});
   
    auto im = img + im2;

    auto p1 = im2.slice({100,100},{164,164},{1,1});
    
    auto p2 = p1+patch+p1+p1;
    
    for(int i = 0;  i < img.shape().dims()[0]; i++ ){
        for(int j = 0;  j < img.shape().dims()[1]; j++ ){
          std::cout << im(i,j) << "  ";
        }
       std::cout << std::endl;
    }
    
    std::cout <<img.shape().dims()[0] << " " << img.shape().dims()[1]<<std::endl;
    
    std::cout <<" ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ " <<std::endl;
    
    for(int i = 0;  i < patch.shape().dims()[0]; i++ ){
        for(int j = 0;  j < patch.shape().dims()[1]; j++ ){
            std::cout << p2(i,j) << "  ";
        }
        std::cout << std::endl;
    }
    std::cout <<p2.shape().dims()[0] << " " << p2.shape().dims()[1]<<std::endl;
    
    
    if(img.write("/Users/giuseppe/Desktop/tt/img.raw"))std::cout << "written "<<std::endl;
    
    AArray<float> im3;
    if(im3.read("/Users/giuseppe/Desktop/tt/img.raw")){
        std::cout << im3.shape() <<std::endl;
    }
    
    return EXIT_SUCCESS;
}
