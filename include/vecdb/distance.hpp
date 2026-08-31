#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

namespace vecdb {

/// Squared L2 (Euclidean) distance between two equal-length vectors.
///
/// The square root of the true Euclidean distance is deliberately omitted:
/// sqrt is monotonically increasing, so it never changes the ranking of
/// neighbours, and skipping it saves one call per comparison.
///
/// Accumulating in float is exact for SIFT data, whose components are integers
/// in [0, 255]: the largest possible sum is 128 * 255^2 ~= 8.3e6, well under
/// the 2^24 limit where float stops representing consecutive integers.
inline float squared_l2(const std::vector<float> &a, const std::vector<float> &b) {
    if (a.size() != b.size()) {
        throw std::invalid_argument("squared_l2: dimension mismatch (" + std::to_string(a.size()) +
                                    " vs " + std::to_string(b.size()) + ")");
    }

    float sum = 0.0f;
    for (std::size_t i = 0; i < a.size(); ++i) {
        const float diff = a[i] - b[i];
        sum += diff * diff;
    }
    return sum;
}

}  // namespace vecdb
