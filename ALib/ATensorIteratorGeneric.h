////
////   ATensorIteratorGeneric.h
////  ALib
////
////  Created by Giuseppe Coppini on 10/03/26.
////
//// ATensorIteratorGeneric.h
//#ifndef ATENSORITERATORGENERIC_H
//#define ATENSORITERATORGENERIC_H
//
//#include <vector>
//#include <algorithm>
//#include <numeric>
//#include <stdexcept>
//#include <immintrin.h> // SIMD
//#include "AShape.h"
//#include "AArray.h"
//
//namespace Alib {
//
//template<typename T>
//class ATensorIteratorGeneric
//{
//public:
//    // costruttore: prende N array
//    ATensorIteratorGeneric(std::vector<AArray<T>*> arrays)
//        : pp_arrays(arrays)
//    {
//        if(arrays.empty()) throw std::runtime_error("No arrays provided");
//
//        // broadcasting shape
//        p_shape = arrays[0]->shape();
//        for(size_t i=1;i<arrays.size();++i)
//            p_shape = AShape::broadcast(p_shape, arrays[i]->shape());
//
//        pp_rank = p_shape.rank();
//        p_dims = p_shape.dims();
//
//        size_t n = arrays.size();
//        pp_data.resize(n);
//        p_offsets.resize(n);
//        p_strides.resize(n);
//
//        for(size_t i=0;i<n;i++)
//        {
//            pp_data[i] = arrays[i]->raw();
//            p_offsets[i] = arrays[i]->offset();
//            p_strides[i] = arrays[i]->strides();
//        }
//
//        // ottimizzazioni
//        reorder_dims();
//        coalesce_dims();
//        detect_contiguous();
//
//        p_index.assign(pp_rank,0);
//        p_done = false;
//    }
//
//    // -------------------
//    // Element-wise kernel con lambda
//    // -------------------
//    template<typename Func>
//    void apply(Func f)
//    {
//        if(pp_fast_contig)
//        {
//            // fast SIMD path per float
//            if constexpr(std::is_same_v<T,float> && pp_data.size()>=2)
//            {
//                size_t n = totalSize();
//                simd_apply(f, n);
//                return;
//            }
//
//            // altrimenti scalare veloce
//            size_t n = totalSize();
//            for(size_t i=0;i<n;i++)
//            {
//                std::vector<T*> ptrs(pp_data.size());
//                for(size_t j=0;j<pp_data.size();j++)
//                    ptrs[j] = pp_data[j]+p_offsets[j];
//                f(ptrs, i);
//            }
//            return;
//        }
//
//        // slow path: general multi-dimensional
//        do
//        {
//            std::vector<T*> ptrs(pp_data.size());
//            for(size_t j=0;j<pp_data.size();j++)
//                ptrs[j] = pp_data[j] + p_offsets[j] + flatIndex(p_index, p_strides[j]);
//            f(ptrs, 0);
//        } while(next());
//    }
//
//    size_t totalSize() const { return std::accumulate(p_dims.begin(), p_dims.end(), 1ULL, std::multiplies<size_t>()); }
//
//private:
//    std::vector<size_t> p_dims;
//    size_t pp_rank = 0;
//    std::vector<std::vector<size_t>> p_strides;
//    std::vector<T*> pp_data;
//    std::vector<size_t> p_offsets;
//    std::vector<AArray<T>*> pp_arrays;
//
//    std::vector<size_t> p_index;
//    bool pp_fast_contig = false;
//    bool p_done = false;
//
//    AShape p_shape;
//
//    // -------------------
//    // helper
//    // -------------------
//    size_t flatIndex(const std::vector<size_t>& idx, const std::vector<size_t>& stride) const
//    {
//        size_t off = 0;
//        for(size_t i=0;i<idx.size();++i)
//            off += idx[i]*stride[i];
//        return off;
//    }
//
//    void reorder_dims()
//    {
//        if(pp_rank <=1) return;
//
//        std::vector<int> perm(pp_rank);
//        for(int i=0;i<pp_rank;i++) perm[i]=i;
//
//        std::sort(perm.begin(), perm.end(),
//            [&](int a,int b)
//            {
//                size_t sa=SIZE_MAX, sb=SIZE_MAX;
//                for(auto& s: p_strides)
//                {
//                    sa = std::min(sa, s[a]);
//                    sb = std::min(sb, s[b]);
//                }
//                return sa < sb;
//            });
//
//        apply_permutation(perm);
//    }
//
//    void apply_permutation(const std::vector<int>& perm)
//    {
//        auto reorder = [&](std::vector<size_t>& v)
//        {
//            std::vector<size_t> tmp(v.size());
//            for(size_t i=0;i<v.size();i++)
//                tmp[i] = v[perm[i]];
//            v = std::move(tmp);
//        };
//
//        reorder(p_dims);
//        for(auto& s : p_strides) reorder(s);
//    }
//
//    void coalesce_dims()
//    {
//        if(pp_rank <=1) return;
//        int write = pp_rank-1;
//        for(int read=pp_rank-2; read>=0; read--)
//        {
//            bool can_merge = true;
//            auto check = [&](const std::vector<size_t>& stride)
//            {
//                size_t s0=stride[read], s1=stride[write];
//                if(s0==0||s1==0) return true;
//                return s0==s1*p_dims[write];
//            };
//
//            for(auto& s : p_strides) if(!check(s)) { can_merge=false; break; }
//
//            if(can_merge) p_dims[write]*=p_dims[read];
//            else
//            {
//                write--;
//                p_dims[write]=p_dims[read];
//                for(size_t i=0;i<p_strides.size();i++)
//                    p_strides[i][write]=p_strides[i][read];
//            }
//        }
//
//        int new_rank = pp_rank - write;
//        p_dims.erase(p_dims.begin(), p_dims.begin()+write);
//        for(auto& s: p_strides) s.erase(s.begin(), s.begin()+write);
//        pp_rank=new_rank;
//    }
//
//    void detect_contiguous()
//    {
//        pp_fast_contig = true;
//        for(size_t d=0;d<pp_rank;d++)
//            for(auto& s: p_strides)
//                if(s[d]!=1){pp_fast_contig=false; break;}
//    }
//
//    bool next()
//    {
//        if(p_done) return false;
//        if(pp_fast_contig){p_done=true; return true;}
//
//        for(int i=int(pp_rank)-1;i>=0;i--)
//        {
//            if(++p_index[i] < p_dims[i]) return true;
//            p_index[i]=0;
//        }
//        p_done=true;
//        return false;
//    }
//
//    // -------------------
//    // SIMD inner-loop esempio float
//    // -------------------
//    template<typename Func>
//    void simd_apply(Func f, size_t n)
//    {
//        size_t i=0;
//#ifdef __AVX__
//        for(; i+8<=n; i+=8)
//        {
//            __m256 va = _mm256_loadu_ps(pp_data[0]+p_offsets[0]+i);
//            __m256 vb = _mm256_loadu_ps(pp_data[1]+p_offsets[1]+i);
//            __m256 vc = _mm256_add_ps(va,vb);
//            _mm256_storeu_ps(pp_data[2]+p_offsets[2]+i,vc);
//        }
//#endif
//        for(; i<n;i++)
//            pp_data[2][p_offsets[2]+i]=pp_data[0][p_offsets[0]+i]+pp_data[1][p_offsets[1]+i];
//    }
//
//};
//
//} // namespace Alib
//
//#endif
