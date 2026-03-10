//
//  ALIBH.h
//  ALib
//
//  Created by Giuseppe Coppini on 27/02/26.
//

#ifndef ALIBH_h
#define ALIBH_h

namespace Alib {

#define AOBJ_MAGIC  0x414F424A // "AOBJ"
#define AOBJ_VERSION 1 

// ------------------------ helper ------------------------
template<typename... Ts>
struct all_integral;

// Base case: empty pack
template<>
struct all_integral<> : std::true_type {};

// Recursive case
template<typename T, typename... Rest>
struct all_integral<T, Rest...>
    : std::integral_constant<bool, std::is_integral<T>::value && all_integral<Rest...>::value>
{};

template<class T>
[[nodiscard]] size_t type_id_of()  noexcept
{
    static int id;
    return reinterpret_cast<size_t>(&id);
}



}


#endif /* ALIBH_h */
