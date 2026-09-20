#pragma once

#include <vector>

template <typename Key>
struct CacheResult {
    bool hit;               
    bool has_evicted;       
    Key evicted_key;        
};


template <typename Key>
class ICache {
    public:

    virtual ~ICache () = default;

    virtual CacheResult<Key> request (const Key& key) = 0;
    virtual bool contains(const Key& key) = 0;
    virtual void erase(const Key& key) = 0;
    virtual std::vector<Key> get_elements() = 0;
    virtual void read_cache () = 0;
};