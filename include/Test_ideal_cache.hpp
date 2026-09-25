#pragma once

#include <string>
#include <vector>

#include "Ideal_cache.hpp"
#include "Test.hpp"
#include "Logger.hpp"
#include "File_request.hpp"
#include "Multi_Level_cache.hpp"

template <typename Key>
void TEST_Ideal_cache (size_t N, const std::vector<Key>& keys)
{
    Ideal_Cache<Key> cache(N, keys);
    
    size_t HITS = 0;
    size_t total_request = 0;

    Log::trace(Log::INFO, "Создан идеальный кэша размером ", N, "\n");
    Log::trace(Log::INFO, "--- Исходное состояние кэша ---\n");
    cache.read_cache();
    Log::trace(Log::DEBUG, "Приходят новые ключи!\n");
    for(const Key& key : keys) {
        Log::trace(Log::INFO, "Запрошен ключ: ", key, "\n");
        
        if(cache.request(key).hit) {
            Log::trace(Log::INFO, "-> ХИТ!\n");
            HITS++;
        } else {
            Log::trace(Log::INFO, "-> МИСС!\n");
        }
        
        cache.read_cache();
        total_request++;
    }
    Log::trace(Log::INFO, "--- Cостояние кэша после теста ---\n");
    cache.read_cache();
    Log::trace(Log::ERROR, "Количество запросов: ", total_request, ", Количество хитов: ", HITS, "\n");
}