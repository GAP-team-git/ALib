//
//  AtensorIterator.h
//  ALib
//
//  Created by Giuseppe Coppini on 14/03/26.
//

#ifndef AtensorIterator_h
#define AtensorIterator_h

#include <vector>
#include <array>
#include <cassert>
#include "AShape.h"
#include "AArray.h"

namespace Alib {

template<typename T> class AArrayBase;

template<typename T>
class ATensorIterator
{
public:
    static constexpr size_t MAX_ARRAYS = 5;
    static constexpr size_t MAX_DIMS   = 8;

    // --- nuovo costruttore principale ---
    ATensorIterator(const std::vector<AArrayBase<T>*>& lhs_arrays,
                    const std::vector<const AArrayBase<T>*>& rhs_arrays)
    {
        init(lhs_arrays, rhs_arrays);
    }

    // --- costruttore legacy: un solo vettore ---
    ATensorIterator(const std::vector<AArrayBase<T>*>& arrays)
    {
        // Tutti gli array vengono trattati come non-const (modificabili)
        init(arrays, {});
    }
    
    template<typename F>
    void forEach(F&& fn)
    {
        if(totalSize() == 0) return;

        // fast path contiguo
        if(isContiguous())
        {
            std::array<T*, MAX_ARRAYS> ptrs;
            for(size_t a = 0; a < n_arrays; ++a) ptrs[a] = base_ptrs[a];

            for(size_t i = 0; i < totalSize(); ++i)
            {
                fn(ptrs, i);
                for(size_t a = 0; a < n_arrays; ++a)
                    ptrs[a] += strides[a][rank - 1];
            }
            return;
        }

        // caso generico
        std::array<T*, MAX_ARRAYS> ptrs;
        std::array<size_t, MAX_DIMS> idx{};
        for(size_t a = 0; a < n_arrays; ++a) ptrs[a] = base_ptrs[a];

        size_t outer = totalOuter();
        size_t inner = dims[rank - 1];

        for(size_t o = 0; o < outer; ++o)
        {
            for(size_t i = 0; i < inner; ++i)
            {
                fn(ptrs, i);
                for(size_t a = 0; a < n_arrays; ++a)
                    ptrs[a] += strides[a][rank - 1];
            }

            // carry / odometer
            for(int d = int(rank) - 2; d >= 0; --d)
            {
                idx[d]++;
                for(size_t a = 0; a < n_arrays; ++a)
                    ptrs[a] += strides[a][d];

                if(idx[d] < dims[d]) break;

                idx[d] = 0;
                for(size_t a = 0; a < n_arrays; ++a)
                    ptrs[a] -= strides[a][d] * dims[d];
            }
        }
    }

private:
    size_t n_arrays{};
    size_t rank{};
    AShape shape;
    std::vector<size_t> dims;

    T* base_ptrs[MAX_ARRAYS]{};
    bool is_const[MAX_ARRAYS]{};
    AArrayBase<T>* lhs_ptrs[MAX_ARRAYS]{};
    const AArrayBase<T>* rhs_ptrs[MAX_ARRAYS]{};
    const AArrayBase<T>* const_ptrs[MAX_ARRAYS]{};

    ptrdiff_t strides[MAX_ARRAYS][MAX_DIMS]{};

    
    void init(const std::vector<AArrayBase<T>*>& lhs_arrays,
              const std::vector<const AArrayBase<T>*>& rhs_arrays)
    {
        n_arrays = lhs_arrays.size() + rhs_arrays.size();
        assert(n_arrays <= MAX_ARRAYS);

        // non-const array copy
        size_t idx = 0;
        for(auto* arr : lhs_arrays) { lhs_ptrs[idx] = arr; is_const[idx++] = false; }

        // cont array copy
        for(auto* arr : rhs_arrays) { const_ptrs[idx] = arr; is_const[idx++] = true; }

        // shape broadcast
        assert(n_arrays > 0);
        shape = lhs_arrays.empty() ? rhs_arrays[0]->shape() : lhs_arrays[0]->shape();
        for(size_t i = 1; i < lhs_arrays.size(); ++i) shape = AShape::broadcast(shape, lhs_arrays[i]->shape());
        for(size_t i = 0; i < rhs_arrays.size(); ++i) shape = AShape::broadcast(shape, rhs_arrays[i]->shape());

        rank = shape.rank();
        dims = shape.dims();

        // base_ptrs and strides initialization
        for(size_t a = 0; a < n_arrays; ++a)
        {
            if(!is_const[a])
                base_ptrs[a] = lhs_ptrs[a]->raw() + lhs_ptrs[a]->shape().offset();
            else
                base_ptrs[a] = const_cast<T*>(const_ptrs[a]->raw()) + const_ptrs[a]->shape().offset();

            const auto& arr_shape = is_const[a] ? const_ptrs[a]->shape() : lhs_ptrs[a]->shape();
            const auto& adims = arr_shape.dims();
            const auto& astrides = arr_shape.strides();
            size_t offset = rank - adims.size();
            for(size_t d = 0; d < rank; ++d)
                strides[a][d] = (d < offset || adims[d - offset] == 1) ? 0 : astrides[d - offset];
        }

        collapseDims();
    }

    // contigue dimension collapsing
    void collapseDims()
    {
        size_t new_rank = 0;
        for(size_t d = 0; d < rank; ++d)
        {
            if(d == 0) { copyDim(new_rank++, d); continue; }

            bool canCollapse = true;
            for(size_t a = 0; a < n_arrays; ++a)
            {
                if(strides[a][d] != strides[a][d - 1] * static_cast<ptrdiff_t>(dims[d - 1]))
                {
                    canCollapse = false;
                    break;
                }
            }

            if(canCollapse) dims[new_rank - 1] *= dims[d];
            else copyDim(new_rank++, d);
        }
        rank = new_rank;
    }

    void copyDim(size_t dst, size_t src)
    {
        dims[dst] = dims[src];
        for(size_t a = 0; a < n_arrays; ++a)
            strides[a][dst] = strides[a][src];
    }

    size_t totalSize() const
    {
        size_t t = 1;
        for(auto d : dims) t *= d;
        return t;
    }

    size_t totalOuter() const
    {
        size_t t = 1;
        for(size_t i = 0; i < rank - 1; ++i) t *= dims[i];
        return t;
    }

    bool isContiguous() const
    {
        for(size_t a = 0; a < n_arrays; ++a)
        {
            ptrdiff_t expected = 1;
            for(int d = int(rank) - 1; d >= 0; --d)
            {
                if(strides[a][d] != expected) return false;
                expected *= dims[d];
            }
        }
        return true;
    }
};

} // namespace Alib

#endif /* AtensorIterator_h */
