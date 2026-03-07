//
//  AShape.h
//  ALib
//
//  Created by Giuseppe Coppini on 03/03/26.
//  Final version  with string helper
//

#ifndef ASHAPE_H
#define ASHAPE_H
#include <vector>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <functional>
#include <string>
#include "AObject.h"
#include "AErrors.h"

namespace Alib {

class AShape : public AObject {
public:
    // -------------------
    // Constructors
    // -------------------
    AShape() = default;

    explicit AShape(const std::vector<size_t>& dims)
        : p_dims(dims) { calcStrides(); }

    AShape(const std::vector<size_t>& dims, const std::vector<size_t>& strides, size_t offset = 0)
        : p_dims(dims), p_strides(strides), p_offset(offset)
    {
        if (dims.size() != strides.size())
            throw std::runtime_error("AShape:: dims/stride size mismatch");
    }

    // -------------------
    // Clone / Hash
    // -------------------
    [[nodiscard]] std::unique_ptr<AObject> clone() const override {
        return std::make_unique<AShape>(*this);
    }

    size_t hash() const override {
        size_t h = 0;
        for (const auto& v : p_dims) h ^= std::hash<size_t>{}(v) + 0x9e3779b9 + (h << 6) + (h >> 2);
        for (const auto& v : p_strides) h ^= std::hash<size_t>{}(v) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<size_t>{}(p_offset) + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h;
    }

    // -------------------
    // Accessors
    // -------------------
    size_t rank() const noexcept { return p_dims.size(); }
    const std::vector<size_t>& dims() const noexcept { return p_dims; }
    const std::vector<size_t>& strides() const noexcept { return p_strides; }
    size_t offset() const noexcept { return p_offset; }

    void setDims(const std::vector<size_t>& dims) { p_dims = dims; calcStrides(); }
    void setStrides(const std::vector<size_t>& s) { p_strides = s; }
    void setOffset(size_t o) { p_offset = o; }

    size_t totalSize() const {
        return std::accumulate(p_dims.begin(), p_dims.end(), 1ULL, std::multiplies<size_t>());
    }

    // -------------------
    // Linear index
    // -------------------
    size_t flatIndex(const std::vector<size_t>& idx) const {
        auto tp_rank = rank();
        if (idx.size() != tp_rank)
            throw std::runtime_error("AShape::flatIndex rank mismatch");
        size_t off = p_offset;
        for (size_t i = 0; i < tp_rank; ++i) {
            if (idx[i] >= p_dims[i])
                throw std::out_of_range("AShape::flatIndex index out of bounds");
            off += idx[i] * p_strides[i];
        }
        return off;
    }

    // -------------------
    // Equality
    // -------------------
    friend bool operator==(const AShape& a, const AShape& b) {
        return a.p_dims == b.p_dims &&
               a.p_strides == b.p_strides &&
               a.p_offset == b.p_offset;
    }

    friend bool operator!=(const AShape& a, const AShape& b) {
        return !(a == b);
    }

    // -------------------
    // Slice
    // -------------------
    AShape sliceShape(const std::vector<size_t>& start,
                      const std::vector<size_t>& stop,
                      const std::vector<size_t>& step) const
    {
        auto tp_rank = rank();
        if (start.size() != tp_rank || stop.size() != tp_rank || step.size() != tp_rank)
            throw std::invalid_argument("sliceShape: size mismatch");

        std::vector<size_t> newDims(tp_rank);
        std::vector<size_t> newStrides(tp_rank);
        size_t newOffset = p_offset;

        for (size_t i = 0; i < tp_rank; ++i) {
            if (step[i] == 0) throw std::invalid_argument("sliceShape: step=0");
            newDims[i] = (stop[i] <= start[i] ? 0 : (stop[i] - start[i] + step[i] - 1) / step[i]);
            newStrides[i] = p_strides[i] * step[i];
            newOffset += start[i] * p_strides[i];
        }

        return AShape(newDims, newStrides, newOffset);
    }

    // -------------------
    // Transpose
    // -------------------
    void transpose(const std::vector<size_t>& axes) {
        auto tp_rank = rank();
        if (axes.size() != tp_rank)
            throw std::invalid_argument("transpose: axes size mismatch");

        std::vector<size_t> newDims(tp_rank);
        std::vector<size_t> newStrides(tp_rank);
        for (size_t i = 0; i < tp_rank; ++i) {
            newDims[i] = p_dims[axes[i]];
            newStrides[i] = p_strides[axes[i]];
        }

        p_dims = std::move(newDims);
        p_strides = std::move(newStrides);
    }

    // -------------------
    // Broadcasting with strides
    // -------------------
    static AShape broadcastWithStrides(const AShape& a, const AShape& b) {
        size_t r1 = a.rank(), r2 = b.rank();
        size_t rmax = std::max(r1, r2);

        std::vector<size_t> dims(rmax, 1);
        std::vector<size_t> strides(rmax, 0);

        for (int i = int(rmax) - 1; i >= 0; --i) {
            size_t da = (i >= int(rmax - r1)) ? a.p_dims[i - (rmax - r1)] : 1;
            size_t db = (i >= int(rmax - r2)) ? b.p_dims[i - (rmax - r2)] : 1;
            size_t sa = (i >= int(rmax - r1)) ? a.p_strides[i - (rmax - r1)] : 0;
            size_t sb = (i >= int(rmax - r2)) ? b.p_strides[i - (rmax - r2)] : 0;

            if (da != db && da != 1 && db != 1)
                throw std::invalid_argument("broadcastWithStrides: incompatible shapes");

            dims[i] = std::max(da, db);

            // stride=0 per dimension broadcasted
            if (da == dims[i])
                strides[i] = sa;
            else if (da == 1)
                strides[i] = 0;
            else if (db == dims[i])
                strides[i] = sb;
            else
                strides[i] = 0;
        }

        return AShape(dims, strides);
    }

    // -------------------
    // String representation (polymorphic, via AObject)
    // -------------------
    [[nodiscard]] std::string str() const override {
        std::string s = "AShape(dims=[";
        for (size_t i = 0; i < p_dims.size(); ++i) {
            s += std::to_string(p_dims[i]);
            if (i + 1 < p_dims.size()) s += ",";
        }
        s += "], strides=[";
        for (size_t i = 0; i < p_strides.size(); ++i) {
            s += std::to_string(p_strides[i]);
            if (i + 1 < p_strides.size()) s += ",";
        }
        s += "], offset=" + std::to_string(p_offset) + ")";
        return s;
    }
    // -------------------
    // NumPy-like string representation
    // -------------------
    [[nodiscard]] std::string numpy_str() const {
        std::string s = "dims=(";
        for (size_t i = 0; i < p_dims.size(); ++i) {
            s += std::to_string(p_dims[i]);
            if (i + 1 < p_dims.size()) s += ",";
        }
        s += "), strides=(";
        for (size_t i = 0; i < p_strides.size(); ++i) {
            s += std::to_string(p_strides[i]);
            if (i + 1 < p_strides.size()) s += ",";
        }
        s += "), offset=" + std::to_string(p_offset);
        return s;
    }
    // -------------------
    // Alias per broadcasting
    // -------------------
    static AShape broadcast(const AShape& a, const AShape& b) {
        return AShape::broadcastWithStrides(a, b);
    }

private:
    std::vector<size_t> p_dims;
    std::vector<size_t> p_strides;
    size_t p_offset = 0;

    void calcStrides() {
        auto tp_rank = rank();
        p_strides.resize(tp_rank);
        if (tp_rank == 0) return;
        p_strides[tp_rank - 1] = 1;
        for (int i = int(tp_rank) - 2; i >= 0; --i)
            p_strides[i] = p_strides[i + 1] * p_dims[i + 1];
    }

    [[nodiscard("AShape m_compare")]] virtual bool m_compare(const AShape& other) const noexcept {
        return (p_dims == other.p_dims && p_strides == other.p_strides && p_offset == other.p_offset);
    }
};

} // namespace Alib
#endif
