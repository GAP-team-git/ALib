//
//  main.cpp
//  Tests
//
//  Created by Giuseppe Coppini on 05/03/26.
//

// test_AArray.cpp
#include <gtest/gtest.h>
#include "AArray.h"
#include "AShape.h"
#include "AErrors.h"
#include <vector>
#include <numeric>

using namespace Alib;
void printND(const AArray<int>& A);
// ---------------------------
// Helper function: fill array with sequential values
template<typename T>
void fillSequential(AArray<T>& arr) {
    T val = 1;
    for(auto& x : *arr.dataPtr()) x = val++;
}

// ---------------------------
// Unit tests
TEST(AArrayTest, ConstructorAndShape) {
    AArray<int> A({2,3,4});
    EXPECT_EQ(A.shape().rank(), 3);
    EXPECT_EQ(A.shape().dims(), (std::vector<size_t>{2,3,4}));
    EXPECT_EQ(A.size(), 24);

    AArray<int> B(5,2);
    EXPECT_EQ(B.shape().dims(), (std::vector<size_t>{5,2}));
    EXPECT_EQ(B.size(), 10);
}

TEST(AArrayTest, Reshape) {
    AArray<int> A({2,3,4});
    fillSequential(A);

    A.reshape({4,3,2});
    EXPECT_EQ(A.shape().dims(), (std::vector<size_t>{4,3,2}));
    EXPECT_EQ(A.size(), 24);

    EXPECT_THROW(A.reshape({5,5}), std::invalid_argument);
}

TEST(AArrayTest, Slice) {
    AArray<int> A({3,4});
    fillSequential(A); // values 1..12

    auto S = A.slice({0,1},{3,4},{1,2}); // pick every 2nd column
    
    EXPECT_EQ(S.shape().dims(), (std::vector<size_t>{3,2}));
    EXPECT_EQ(S(0,0), 2);
    EXPECT_EQ(S(2,1), 12);
}

TEST(AArrayTest, Transpose) {
    AArray<int> A({2,3});
    fillSequential(A);

    auto T = A.transpose(); // default reverse axes
    EXPECT_EQ(T.shape().dims(), (std::vector<size_t>{3,2}));
    EXPECT_EQ(T(0,0), 1);
    EXPECT_EQ(T(2,1), 6);

    auto T2 = A.transpose({1,0}); // explicit axes
    EXPECT_EQ(T2.shape().dims(), (std::vector<size_t>{3,2}));
    EXPECT_EQ(T2(2,1), 6);
}

TEST(AArrayTest, OperatorCallAndProxy) {
    AArray<int> A({2,2,3});
    fillSequential(A);

    EXPECT_EQ(A(0,0,0), 1);
    EXPECT_EQ(A(1,1,2), 12);

    AArrayProxy<int> p = A[1][1];
    EXPECT_EQ(p[2], 12);

    p[0] = 42;
    EXPECT_EQ(A(1,1,0), 42);
}

TEST(AArrayTest, BroadcastingArithmetic) {
    AArray<int> A({2,3});
    fillSequential(A); // 1..6

    AArray<int> B({1,3});
    for(int i=0;i<3;++i) (*B.dataPtr())[i] = 10+i; // 10,11,12

    auto C = A + B; // broadcasting
    EXPECT_EQ(C.shape().dims(), (std::vector<size_t>{2,3}));
    EXPECT_EQ(C(0,0), 1+10);
    EXPECT_EQ(C(1,2), 6+12);

    A += B;
    EXPECT_EQ(A(0,1), 2+11);
}

TEST(AArrayTest, EdgeCases) {
    AArray<int> A({1,0,3});
    EXPECT_EQ(A.size(), 0);

    AArray<int> B({2});
    B(0) = 5;
    EXPECT_EQ(B(0),5);

    EXPECT_THROW(B(2), std::out_of_range);
}

// Optional: printND visualization for debugging
void printND(const AArray<int>& A) {
    const auto& dims = A.shape().dims();
    if(dims.size()==2){
        for(size_t i=0;i<dims[0];++i){
            for(size_t j=0;j<dims[1];++j) std::cout << A(i,j) << " ";
            std::cout << std::endl;
        }
    }
}

// ---------------------------
// Main
int main(int argc, char** argv){
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
