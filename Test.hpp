#pragma once

#include <iostream>
#include <vector>

#include "Logger.hpp"

template <typename CacheType>

void TEST (size_t N, const std::vector<int>& keys)
{
    CacheType cache(N);
    
    Log::trace(Log::INFO, "--- Исходное состояние кэша ---\n");
    cache.read_cache();
    Log::trace(Log::DEBUG, "Приходят новые ключи!\n");
    for (int key : keys) {
        cache.request(key);
        cache.read_cache();
    }
    Log::trace(Log::INFO, "--- Cостояние кэша после теста ---\n");
    cache.read_cache();
}

void test_LRU ();