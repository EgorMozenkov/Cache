#include <iostream>
#include <string>
#include <vector>

#include "LRU_cache.hpp"
#include "LFU_cache.hpp"
#include "2Q_cache.hpp"
#include "ARC_cache.hpp"
#include "LIRS_cache.hpp"
#include "Test.hpp"
#include "Logger.hpp"
#include "File_request.hpp"
#include "Multi_Level_cache.hpp"

template <typename Key> 
void test_multi_level_cache(size_t capacity, const std::vector<std::string>& algos, const std::vector<Key>& keys) 
{
    Config conf;
    conf.algorithms = algos;
    conf.levels = algos.size();

    InputData<Key> data;
    data.Cache_size = capacity;
    data.keys = keys;
    data.num_requests = keys.size();

    Multi_Level_Cache<Key> cache(conf, data);

    size_t HITS = 0;
    size_t total_requests = 0;

    Log::trace(Log::INFO, "Создана многоуровневая система. Размер: ", capacity, ", уровни: ", conf.levels, "\n");
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
        total_requests++;
    }

    Log::trace(Log::INFO, "--- Cостояние кэша после теста ---\n");
    cache.read_cache();
    Log::trace(Log::ERROR, "Количество запросов: ", total_requests, ", Количество хитов: ", HITS, "\n");
}