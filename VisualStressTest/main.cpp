// Main.cpp
//  ALib
//
//  Created by Giuseppe Coppini on 06/03/26.
//
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <cassert>
#include "AArray.h"

// ----------------------
// Helpers
// ----------------------
template<typename T>
void fillSequential(Alib::AArray<T>& A, T start=1) {
    T val = start;
    for(size_t i=0; i<A.size(); ++i) (*A.dataPtr())[i] = val++;
}

std::vector<size_t> randomDims(size_t rank, size_t maxDim=6) {
    std::vector<size_t> dims(rank);
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<size_t> dist(1,maxDim);
    for(size_t i=0;i<rank;++i) dims[i]=dist(gen);
    return dims;
}

void randomSlice(const std::vector<size_t>& dims,
                 std::vector<size_t>& start,
                 std::vector<size_t>& stop,
                 std::vector<size_t>& step) {
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<size_t> stepDist(1,2);

    start.resize(dims.size());
    stop.resize(dims.size());
    step.resize(dims.size());

    for(size_t i=0;i<dims.size();++i){
        std::uniform_int_distribution<size_t> dist(0,dims[i]-1);
        start[i]=dist(gen);
        stop[i]=dist(gen)+1;
        if(stop[i]>dims[i]) stop[i]=dims[i];
        step[i]=stepDist(gen);
        if(start[i]>=stop[i]) start[i]=0;
    }
}

// ----------------------
// Mega Stress Test Assertivo
// ----------------------
void megaStressAssertive(size_t iterations=1000) {
    using namespace Alib;
    std::cout << "=== MEGA ASSERTIVE STRESS TEST AArray ===\n";

    for(size_t iter=1; iter<=iterations; ++iter){
        // Random rank 2-4
        size_t rank = 2 + rand()%3;
        auto dims = randomDims(rank,6);
        AArray<int> A(dims);
        fillSequential(A);

        // Random slice
        std::vector<size_t> start, stop, step;
        randomSlice(dims,start,stop,step);
        auto S = A.slice(start,stop,step);

        // Check slice dimensions
        for(size_t d=0; d<rank; ++d) {
            size_t expectedDim = (stop[d]<=start[d]?0:(stop[d]-start[d]+step[d]-1)/step[d]);
            assert(S.shape().dims()[d]==expectedDim);
        }

        // Random transpose
        std::vector<size_t> axes(S.shape().rank());
        for(size_t i=0;i<axes.size();++i) axes[i]=i;
        std::shuffle(axes.begin(), axes.end(), std::mt19937(std::random_device()()));
        auto T = S.transpose(axes);

        // Check transpose preserves total size
        assert(T.size()==S.size());

        // Broadcasting addition with self
        auto R = T + T;
        assert(R.size()==T.size());

        // Spot check first and last elements
        if(R.size()>0) {
            assert(R(0)==T(0)+T(0));
            assert(R(R.size()-1)==T(T.size()-1)+T(T.size()-1));
        }

        if(iter % 100 == 0) std::cout << "Iteration " << iter << " passed.\n";
    }

    std::cout << "\n=== ASSERTIVE TEST COMPLETED SUCCESSFULLY ===\n";
}

int main() {
    megaStressAssertive(1000); // 1000 iterazioni
    return 0;
}
