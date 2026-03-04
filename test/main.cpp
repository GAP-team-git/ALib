//
//  main.cpp
//  test
//
//  Created by Giuseppe Coppini on 25/02/26.
//

#include <iostream>
#include "Alib.h"
#include <assert.h>

//namespace Alib {
//
//void test_broadcast() {
//    AArray<float> a(4,1,2);
//    AArray<float> b(1,3,2);
//    
//   // auto c = a + b;
//    
//    for(auto &a: c.shape().dims())
//    std::cout << a<< " ";
//    std::cout << std::endl;
//    
//    assert(c.shape() == AShape(4,3, 2) );
//}
//}

#include <iostream>
#include "AArray.h"

using namespace Alib;

int main() {
    try {
        // -----------------------
        // Crea un array 2x3
        // -----------------------
        AArray<int> arr({2,3});
        int counter = 1;
        for(auto& v : *arr.dataPtr()) v = counter++;

        std::cout << "Original array (2x3): ";
        for(auto v : *arr.dataPtr()) std::cout << v << " ";
        std::cout << "\n";

        // -----------------------
        // Slice: seleziona righe/colonne
        // -----------------------
        std::vector<size_t> start = {0,0};
        std::vector<size_t> stop  = {2,3};
        std::vector<size_t> step  = {1,2};
        auto sliceArr = arr.slice(start, stop, step);
        std::cout << "Slice array shape: ";
        for(auto d : sliceArr.shape().dims()) std::cout << d << " ";
        std::cout << "\n";

        // -----------------------
        // Reshape
        // -----------------------
        arr.reshape({3,2});
        std::cout << "Reshaped array (3x2): ";
        for(auto v : *arr.dataPtr()) std::cout << v << " ";
        std::cout << "\n";

        // -----------------------
        // Transpose
        // -----------------------
        arr.transpose({1,0});
        std::cout << "Transposed array shape: ";
        for(auto d : arr.shape().dims()) std::cout << d << " ";
        std::cout << "\n";

        // -----------------------
        // Broadcasting
        // -----------------------
        AArray<int> bcast({1,2});
        bcast.broadcastTo({3,2}); // broadcast 1x2 -> 3x2
        std::cout << "Broadcasted array shape: ";
        for(auto d : bcast.shape().dims()) std::cout << d << " ";
        std::cout << "\n";

        // -----------------------
        // Operatori aritmetici
        // -----------------------
        AArray<int> arr2({3,2});
        int k=10;
        for(auto& v : *arr2.dataPtr()) v=k++;
        arr += arr2;
        std::cout << "After addition: ";
        for(auto v : *arr.dataPtr()) std::cout << v << " ";
        std::cout << "\n";

        // -----------------------
        // Clone
        // -----------------------
        auto cloneArr = arr.clone();
        std::cout << "Cloned array hash: " << cloneArr->hash() << "\n";

        // -----------------------
        // Hash & Comparison
        // -----------------------
        AArray<int> arrCopy({3,2});
        *arrCopy.dataPtr() = *arr.dataPtr();
        std::cout << "arr == arrCopy? " << ((*arrCopy.dataPtr() == *arr.dataPtr()) ? "Yes" : "No") << "\n";

        // -----------------------
        // Access via proxy (multi-index)
        // -----------------------
        AArray<int> mat({2,3});
        counter=1;
        for(auto& v : *mat.dataPtr()) v=counter++;
        std::cout << "Matrix access via proxy:\n";
        for(size_t i=0;i<2;++i){
            for(size_t j=0;j<3;++j){
                std::cout << mat[i][j] << " ";
            }
            std::cout << "\n";
        }

    } catch(const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return -1;
    }

    return 0;
}
