#pragma once

#include <list>
#include <unordered_map>
#include "I_cache.hpp"

using IT = std::list<int>::iterator;

class LRU_Cache : public ICache {
private:
    std::list<int> LRU_list;
    std::unordered_map<int, IT> LRU_map;
    size_t capacity;

public:
    LRU_Cache(size_t N); 

    bool request(int key) override;
    void read_cache() override;
};