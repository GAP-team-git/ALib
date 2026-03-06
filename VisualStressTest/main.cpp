// Main.cpp
//  ALib
//
//  Created by Giuseppe Coppini on 06/03/26.
//
#include "AArray.h"
#include <gtest/gtest.h>
#include <random>
#include <iostream>

using namespace Alib;

// Helper: genera un array casuale di dimensioni date
template<typename T>
AArray<T> randomArray(const std::vector<size_t>& dims, T minVal=0, T maxVal=100) {
    AArray<T> arr(dims);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<T> dis(minVal,maxVal);

    for(size_t i=0;i<arr.size();++i) arr.dataPtr()->at(i) = dis(gen);
    return arr;
}

// Helper: copia lineare (per views)
template<typename T>
AArray<T> linearCopy(const AArray<T>& arr) {
    AArray<T> copy(arr.shape());
    for(size_t i=0;i<arr.size();++i) copy.dataPtr()->at(i) = arr.dataPtr()->at(i);
    return copy;
}

// Mega Stress Test
TEST(AArrayStressTest, MegaStressSafe) {
    std::vector<std::vector<size_t>> shapes = {
        {3,4}, {5,2,3}, {4,4,4}, {2,3,5,2}
    };

    for(const auto& dims : shapes){
        // Array base
        auto A = randomArray<int>(dims, 0, 1000);

        // Slice casuale
        std::vector<size_t> start(dims.size(),0);
        std::vector<size_t> stop = dims;
        std::vector<size_t> step(dims.size(),1);
        auto S = A.slice(start, stop, step);

        // Transpose casuale
        std::vector<size_t> axes(dims.size());
        for(size_t i=0;i<dims.size();++i) axes[i]=i;
        std::shuffle(axes.begin(), axes.end(), std::mt19937{std::random_device{}()});
        auto T = S.transpose(axes);

        // Copia lineare per sicurezza
        auto L1 = linearCopy(T);
        auto L2 = linearCopy(T);

        // Operazioni binarie
        auto R = L1 + L2;    // somma
        auto R2 = L1 * L2;   // moltiplicazione

        // Verifiche
        ASSERT_EQ(R.size(), L1.size());
        ASSERT_EQ(R2.size(), L1.size());

        for(size_t i=0;i<L1.size();++i){
            EXPECT_EQ(R(i), L1(i)+L2(i));
            EXPECT_EQ(R2(i), L1(i)*L2(i));
        }
    }
    std::cout << "[Mega Stress Test] Tutti i casi passati!" << std::endl;
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
