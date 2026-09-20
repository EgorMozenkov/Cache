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
void data_output(Multi_Level_Cache<Key>& cache, const Config& conf, const InputData<Key>& data)
{
    size_t HITS = 0;
    size_t total_requests = 0;

    // Берем данные из структуры data
    Log::trace(Log::ERROR, "Создана многоуровневая система. Размер: ", data.Cache_size, ", уровни: ", conf.levels, "\n");
    Log::trace(Log::INFO, "--- Исходное состояние кэша ---\n");
    //cache.read_cache();
    Log::trace(Log::DEBUG, "Приходят новые ключи!\n");

    // Итерируемся по вектору из структуры data
    for(const Key& key : data.keys) {
        Log::trace(Log::INFO, "Запрошен ключ: ", key, "\n");
        
        if(cache.request(key).hit) {
            Log::trace(Log::INFO, "-> ХИТ!\n");
            HITS++;
        } else {
            Log::trace(Log::INFO, "-> МИСС!\n");
        }
        
        //cache.read_cache();
        total_requests++;
    }

    Log::trace(Log::INFO, "--- Cостояние кэша после теста ---\n");
    //cache.read_cache();
    Log::trace(Log::ERROR, "Количество запросов: ", total_requests, ", Количество хитов: ", HITS, "\n\n");
}

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

    data_output(cache, conf, data);
}

template <typename Key> 
void file_test(const std::string& filename) 
{
    Config conf;
    InputData<Key> data;

    read_file(filename, conf, data);

    if (data.num_requests == 0) {
        return; 
    }

    Multi_Level_Cache<Key> cache(conf, data);

    data_output(cache, conf, data);
}