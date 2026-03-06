//
//  main.cpp
//  test
//
//  Created by Giuseppe Coppini on 25/02/26.
//
#include <iostream>
#include "AArray.h"

//#define USE_COLOR
#undef USE_COLOR
using namespace Alib;

// Funzione ricorsiva compatta, colorata solo sui valori
template<typename T>
void printNDColored(const AArray<T>& arr, size_t dim=0, std::vector<size_t> idx={}, int indent=0) {
    if(idx.empty()) idx.resize(arr.shape().rank(),0);

#ifdef USE_COLOR
    const std::vector<std::string> colors = {
        "\033[31m", // red
        "\033[32m", // green
        "\033[33m", // yellow
        "\033[34m", // blue
        "\033[35m", // magenta
        "\033[36m", // cyan
        "\033[37m", // white
    };
    const std::string reset = "\033[0m";
#else
    const std::vector<std::string> colors = {
        "",
        "",
        "",
        "",
        "",
        "",
        "",
    };
    const std::string reset = "";
#endif // USE_COLOR
    
    
    size_t n = arr.shape().dims()[dim];

    if(dim == arr.shape().rank() - 1){
        std::cout << std::string(indent,' ') << "[";
        for(size_t i=0;i<n;++i){
            idx[dim] = i;
            std::cout << colors[i % colors.size()] << arr.at(idx) << reset;
            if(i < n-1) std::cout << ", ";
        }
        std::cout << "]";
        return;
    }

    std::cout << std::string(indent,' ') << "[\n";
    for(size_t i=0;i<n;++i){
        idx[dim] = i;
        printNDColored(arr, dim+1, idx, indent+4);
        if(i < n-1) std::cout << ",\n";
        else std::cout << "\n";
    }
    std::cout << std::string(indent,' ') << "]";
}

int main() {
    AArray<int> A({2,3,4}); // 2x3x4
    int val=1;
    for(size_t i=0;i<2;++i)
        for(size_t j=0;j<3;++j)
            for(size_t k=0;k<4;++k)
                A[i][j][k] = val++;

    std::cout << "Array A (2x3x4):\n";
    printNDColored(A);
    std::cout << "\n\n";

    // Slice e reshape
    auto S = A.slice({0,0,0},{2,2,4});
    S.reshape({4,2,2});
    std::cout << "Slice S reshaped (4x2x2):\n";
    printNDColored(S);
    std::cout << "\n\n";

    // Transpose
    auto T = S.transpose({2,1,0});
    std::cout << "Transposed T (2x2x4):\n";
    printNDColored(T);
    std::cout << "\n\n";

    // Broadcasting arithmetic
    AArray<int> B({1,3,4});
    B[0][0][0] = 100; B[0][1][1] = 200; B[0][2][2] = 300;
    auto C = A + B;
    std::cout << "Broadcasted sum C (A+B):\n";
    printNDColored(C);
    std::cout << "\n\n";

    // In-place arithmetic
    C += B;
    std::cout << "C after in-place C += B:\n";
    printNDColored(C);
    std::cout << "\n\n";

    return 0;
}
