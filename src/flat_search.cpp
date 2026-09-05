#include <vecdb/flat_search.hpp>
#include <vecdb/distance.hpp>
#include <queue>
#include <stdexcept>

namespace vecdb{

std::vector<int> flat_search(const std::vector<float>& query, const std::vector<VectorRecord<float>>& base, std::size_t k){
    if(k<=0 || k>base.size()){
        throw std::invalid_argument("Invalid K");
    }

    std::priority_queue<std::pair<float, int>> candidates;

    for(const auto& record: base){
        // if(query.size() != base.size()){    
        //     throw std::invalid_argument("Vector dimensions not same");
        // }
        float dist = squared_l2_distance(query, record.vector);    // already takes checks dimensions

        if(candidates.size() < k){    // slots empty, take whoever applies
            candidates.push({dist, record.id});
        } else if(dist < candidates.top().first){    // slots full but good candidate: kick out worst one in team and replace
            candidates.pop();
            candidates.push({dist, record.id});
        }
    }

    // this is max heap, but need to report in closest-to-farthest order, hence reverse
    int n = candidates.size();
    std::vector<int> arr(n);
    for(int i = 0; i < n; i++){
        arr[n-i-1] = candidates.top().second;
        candidates.pop();
    }
    return arr;
}

}