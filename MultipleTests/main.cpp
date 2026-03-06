//
//  main.cpp
//  MultipleTests
//
//  Created by Giuseppe Coppini on 06/03/26.
//

#include <gtest/gtest.h>
#include <random>
#include <numeric>
#include <algorithm>
#include <iostream>
#include "AArray.h"

using namespace Alib;

static size_t flatIndexRef(
    const std::vector<size_t>& idx,
    const std::vector<size_t>& dims)
{
    size_t stride = 1;
    size_t flat = 0;

    for(int i=int(dims.size())-1;i>=0;i--){
        flat += idx[i]*stride;
        stride*=dims[i];
    }

    return flat;
}

static std::vector<size_t> randomDims(
    size_t rank,
    std::mt19937& rng)
{
    std::uniform_int_distribution<int> dimDist(1,5);

    std::vector<size_t> d(rank);

    for(size_t i=0;i<rank;i++)
        d[i]=dimDist(rng);

    return d;
}

static std::vector<size_t> randomIndex(
    const std::vector<size_t>& dims,
    std::mt19937& rng)
{
    std::vector<size_t> idx(dims.size());

    for(size_t i=0;i<dims.size();i++){
        std::uniform_int_distribution<size_t> dist(0,dims[i]-1);
        idx[i]=dist(rng);
    }

    return idx;
}

TEST(AArrayMegaStressTest, FullRandomValidation)
{
    std::mt19937 rng(12345);

    std::uniform_int_distribution<int> rankDist(1,4);
    std::uniform_int_distribution<int> valDist(0,100);

    const size_t NTEST = 20000;

    for(size_t test=0; test<NTEST; ++test)
    {
        size_t rank = rankDist(rng);

        auto dims = randomDims(rank,rng);

        AArray<int> A(dims);
        AArray<int> B(dims);

        size_t total = A.size();

        for(size_t i=0;i<total;i++)
        {
            (*A.dataPtr())[i]=valDist(rng);
            (*B.dataPtr())[i]=valDist(rng);
        }

        // =========================
        // ND indexing validation
        // =========================

        for(size_t k=0;k<20;k++)
        {
            auto idx = randomIndex(dims,rng);

            size_t flat = flatIndexRef(idx,dims);

            EXPECT_EQ(A(idx), (*A.dataPtr())[flat]);
        }

        // =========================
        // Arithmetic ops
        // =========================

        auto C = A + B;

        for(size_t i=0;i<total;i++)
            EXPECT_EQ((*C.dataPtr())[i],
                      (*A.dataPtr())[i] + (*B.dataPtr())[i]);

        auto D = A * B;

        for(size_t i=0;i<total;i++)
            EXPECT_EQ((*D.dataPtr())[i],
                      (*A.dataPtr())[i] * (*B.dataPtr())[i]);

        // =========================
        // Transpose test
        // =========================

        if(rank>=2)
        {
            std::vector<size_t> axes(rank);
            std::iota(axes.begin(),axes.end(),0);

            std::shuffle(axes.begin(),axes.end(),rng);

            auto T = A.transpose(axes);

            auto td = T.shape().dims();

            for(size_t k=0;k<20;k++)
            {
                auto idx = randomIndex(td,rng);

                std::vector<size_t> orig(rank);

                for(size_t i=0;i<rank;i++)
                    orig[axes[i]]=idx[i];

                EXPECT_EQ(T(idx), A(orig));
            }
        }

        // =========================
        // Slice test
        // =========================

        if(rank>=2)
        {
            std::vector<size_t> start(rank,0);
            std::vector<size_t> stop=dims;
            std::vector<size_t> step(rank,1);

            for(size_t i=0;i<rank;i++)
                step[i] = (rng()%2)+1;

            auto S = A.slice(start,stop,step);

            auto sd = S.shape().dims();

            for(size_t k=0;k<20;k++)
            {
                auto idx = randomIndex(sd,rng);

                std::vector<size_t> orig(rank);

                for(size_t i=0;i<rank;i++)
                    orig[i] = start[i] + idx[i]*step[i];

                EXPECT_EQ(S(idx), A(orig));
            }
        }

        // =========================
        // Broadcasting test
        // =========================

        if(rank>=2)
        {
            std::vector<size_t> dimsB=dims;

            dimsB[rank-1]=1;

            AArray<int> E(dimsB);

            for(size_t i=0;i<E.size();i++)
                (*E.dataPtr())[i]=valDist(rng);

            auto R = A + E;

            for(size_t k=0;k<20;k++)
            {
                auto idx = randomIndex(dims,rng);

                std::vector<size_t> idxE=idx;

                idxE[rank-1]=0;

                EXPECT_EQ(R(idx),
                          A(idx)+E(idxE));
            }
        }

        // =========================
        // Reshape test
        // =========================

        if(total>1)
        {
            std::vector<size_t> newDims={total};

            auto copyA = A;

            copyA.reshape(newDims);

            for(size_t i=0;i<total;i++)
                EXPECT_EQ(copyA(i), (*A.dataPtr())[i]);
        }

        if(test%500==0)
            std::cout<<"stress iteration "<<test<<"\n";
    }
}

// ---------------------------
// Main
int main(int argc, char** argv){
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
