#include <vecdb/distance.hpp>

namespace vecdb {

float squared_l2_distance(const std::vector<float> &first, const std::vector<float> &second) {
    if (first.size() != second.size()) {
        throw std::invalid_argument("Vector Dimension not same");
    }
    float distSQ = 0;
    for (int i = 0; i < first.size(); i++) {
        distSQ += (first[i] - second[i]) * (first[i] - second[i]);
    }
    return distSQ;
}

} // namespace vecdb