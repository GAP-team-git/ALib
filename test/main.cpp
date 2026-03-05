//
//  main.cpp
//  test
//
//  Created by Giuseppe Coppini on 25/02/26.
//
#include <iostream>
#include <vector>
#include <iomanip>
#include "AArray.h"

using namespace Alib;

// Funzione ricorsiva per stampare ND array con indentazione
template<typename T>
void printND(const AArray<T>& arr, size_t dim=0, std::vector<size_t> idx={}) {
    if(idx.empty()) idx.resize(arr.shape().rank(), 0);

    if(dim == arr.shape().rank()) {
        // indice completo, stampo il valore
        std::cout << arr.at(idx) << " ";
        return;
    }

    for(size_t i = 0; i < arr.shape().dims()[dim]; ++i) {
        idx[dim] = i;
        printND(arr, dim+1, idx);
        if(dim == arr.shape().rank()-1) std::cout << "\n";
    }
}



// Funzione helper per separatore fra test
void printSeparator(const std::string& msg) {
    std::cout << "\n================ " << msg << " ================\n";
}

int main() {
    // Creazione array 3x2x2
    AArray<int> A({3,2,2});
    int val = 1;
    for(size_t i=0;i<A.size();++i) (*A.dataPtr())[i] = val++;

    printSeparator("Original A");
    printND(A);

    // Slice [1:3,0:2,0:2]
    AArray<int> slice = A.slice({1,0,0},{3,2,2});
    printSeparator("Slice A[1:3,0:2,0:2]");
    printND(slice);

    // Reshape 2x3x2
    AArray<int> B = A;
    B.reshape({2,3,2});
    printSeparator("Reshaped B (2x3x2)");
    printND(B);

    // Transpose (reverse axes)
    AArray<int> C = A.transpose();
    printSeparator("Transposed C (axes reversed)");
    printND(C);

    // Broadcasting inplace
    AArray<int> X({2,3});
    AArray<int> Y({1,3});
    for(size_t i=0;i<X.size();++i) (*X.dataPtr())[i] = int(i+1);
    for(size_t i=0;i<Y.size();++i) (*Y.dataPtr())[i] = int(10*(i+1));

    printSeparator("X before iadd(Y)");
    printND(X);

    X += Y;
    printSeparator("X after iadd(Y) (broadcasted)");
    printND(X);

    // Binary operator broadcasting
    AArray<int> Z = X + Y;
    printSeparator("Z = X + Y");
    printND(Z);

    return 0;
}
