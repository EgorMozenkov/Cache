#pragma once

#include <iostream>
#include <vector>

#include "Logger.hpp"

template <typename CacheType, typename Key>

void TEST (size_t N, const std::vector<Key>& keys)
{
    CacheType cache(N);
    
    size_t HITS = 0;
    size_t total_request = 0;

    Log::trace(Log::INFO, "Создан тип кэша размером ", N, "\n");
    Log::trace(Log::INFO, "--- Исходное состояние кэша ---\n");
    cache.read_cache();
    Log::trace(Log::DEBUG, "Приходят новые ключи!\n");
    for (const Key& key : keys) {

        if (cache.request(key).hit) {
            HITS++;
        }
        cache.read_cache();
        total_request++;
    }
    Log::trace(Log::INFO, "--- Cостояние кэша после теста ---\n");
    cache.read_cache();
    Log::trace(Log::TRACE, "Количество запросов: ", total_request, ", Количество хитов: ", HITS, "\n");
}

void test_LRU ();
void test_LFU ();
void test_TwoQ ();
void test_ARC ();
void test_LIRS ();  

void string_test_LRU ();
void string_test_LFU ();
void string_test_TwoQ ();
void string_test_ARC ();
void string_test_LIRS ();