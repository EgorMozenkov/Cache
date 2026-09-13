#pragma once

#include <list>
#include <unordered_map>
#include "I_cache.hpp"

using IT = std::list<int>::iterator;

struct ItemInfo {
    int freq;
    IT list_it;
};

class LFU_Cache : public ICache {
private:
    std::unordered_map<int, ItemInfo> LFU_map_key;
    std::unordered_map<int, std::list<int>> LFU_map_freq;
    size_t capacity;
    int min_freq;

public:
    LFU_Cache(size_t N); 

    bool request(int key) override;
    void read_cache() override;
};