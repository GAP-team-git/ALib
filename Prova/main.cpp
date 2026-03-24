//
//  main.cpp
//  test
//
//  Created by Giuseppe Coppini on 25/02/26.
//

#include <iostream>
#include <iomanip>
#include <time.h>
#include "AArray.h"
#include "AExpr.h"
#include "ABinaryExpr.h"
#include "AUnaryExpr.h"
#include "AArrayOps.h"

#include "AShape.h"
#include "AConfig.h"
#include "AAssert.h"
#include <math.h>

using namespace Alib;

int main() {
    
    std::vector<size_t> d={512,512};
    AArray<float> img(d),im2(d);
   // img.randomize(0,1);
    im2.randomize(0,1);
    img.fill(10);
    
    img = img + im2;
    
    img.print(std::cout);
    img.printDescription(std::cout);
    std::cout <<"\nMAX: "<< min(img) << " at ";
    img.printNDIndex(std::cout,argmin_nd(img));
    std::cout <<"\nimg at ";
    img.printNDIndex(std::cout,img.unravel_index(argmin(img)));
    std::cout << " "<<img(argmin_nd(img)) <<std::endl;
    return EXIT_SUCCESS;
}
