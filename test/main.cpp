//
//  main.cpp
//  test
//
//  Created by Giuseppe Coppini on 25/02/26.
//


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


#include "AArray.h"
#include "AShape.h"
#include <iostream>
#include <cassert>
#include <numeric> // per iota

using namespace Alib;

/// <#Description#>
int main() {
    std::cout << "=== AArray / AShape Extended Test ===\n";

    // =======================
    // 1️⃣ Creazione array ND
    // =======================
    AArray<int> A({2, 1, 3});       // shape (2,1,3)
    AArray<int> B({1, 2, 3});       // shape (1,2,3)
    AArray<int> C({2,2,3});         // shape (2,2,3)
    
    // Riempio con valori semplici
    int val = 1;
    for(auto& v : *A.dataPtr()) v = val++;
    val = 10;
    for(auto& v : *B.dataPtr()) v = val++;
    val = 100;
    for(auto& v : *C.dataPtr()) v = val++;

    // =======================
    // 2️⃣ Accesso proxy ND
    // =======================
    std::cout << "A[1][0][2] = " << A[1][0][2] << " (should be 6)\n";
    A[1][0][2] = 42;
    std::cout << "A[1][0][2] after set = " << A[1][0][2] << "\n";

    // =======================
    // 3️⃣ Slice con step
    // =======================
    AArray<int> S = A.slice({0,0,0},{2,1,3},{1,1,2}); // step in ultima dim
    std::cout << "Slice shape: ";
    for(auto d : S.shape().dims()) std::cout << d << " ";
    std::cout << " | size = " << S.size() << "\n";

    // =======================
    // 4️⃣ Broadcasting automatico
    // =======================
    AArray<int> D = A + B; // broadcast shapes (2,1,3) + (1,2,3) -> (2,2,3)
    std::cout << "Broadcast result shape: ";
    for(auto d : D.shape().dims()) std::cout << d << " ";
    std::cout << "\nValues:\n";
    for(auto v : *D.dataPtr()) std::cout << v << " ";
    std::cout << "\n";

    // =======================
    // 5️⃣ Operazioni miste ND
    // =======================
    AArray<int> E = A + B * C; // broadcasting automatico
    std::cout << "Mixed op (A + B*C) values:\n";
    for(auto v : *E.dataPtr()) std::cout << v << " ";
    std::cout << "\n";

    // =======================
    // 6️⃣ Reshape
    // =======================
    AArray<int> R(3,2) ;//= A.reshape({3,2});
    std::cout << "Reshape A -> (3,2), size: " << R.size() << "\n";

    // =======================
    // 7️⃣ Transpose
    // =======================
    AArray<int> T = C.transpose({2,1,0});
    std::cout << "Transpose C axes (2,1,0), shape: ";
    for(auto d : T.shape().dims()) std::cout << d << " ";
    std::cout << "\n";

    // =======================
    // 8️⃣ Operazioni in-place
    // =======================
    A += B;
    std::cout << "A += B (broadcasted) values:\n";
    for(auto v : *A.dataPtr()) std::cout << v << " ";
    std::cout << "\n";

    C *= B;
    std::cout << "C *= B (broadcasted) values:\n";
    for(auto v : *C.dataPtr()) std::cout << v << " ";
    std::cout << "\n";

    // =======================
    // 9️⃣ Clone e hash
    // =======================
    auto Cl = A.clone();
    std::cout << "Clone OID: " << Cl->Oid() << " | Original OID: " << A.Oid() << "\n";
    std::cout << "Hash A: " << A.hash() << " | Hash clone: " << Cl->hash() << "\n";

    // =======================
    // 10️⃣ Assert automatici
    // =======================
    assert(A.shape().totalSize() == D.shape().totalSize());
    assert(S.shape().rank() == 3); // shape slice (2,1,2) -> 3 elements
    assert(T.shape().dims()[0] == 3); // first axis after transpose
    assert(E.size() == 12); // broadcast shape (2,2,3) -> 12 elements

    std::cout << "All extended tests passed ✅\n";
    std:: cout << "HASH " << A.hash() << " " << B.hash() << std::endl;
    return EXIT_SUCCESS;
}

