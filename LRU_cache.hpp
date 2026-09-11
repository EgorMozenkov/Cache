#pragma once

#include <list>
#include <unordered_map>
#include "I_cache.hpp"

typedef std::list<int>::iterator IT;

class LRU_Cache : public ICache {
private:
    std::list<int> LRU_list;
    std::unordered_map<int, std::list<int>::iterator> LRU_map;
    size_t capacity;

public:
    LRU_Cache(size_t N); 

    bool request(int key) override;
    void read_cache() override;
};