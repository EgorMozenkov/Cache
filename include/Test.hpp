#pragma once

#include <iostream>
#include <vector>
#include "Logger.hpp"


/**
 * @brief Универсальный испытательный стенд для проверки работы кэша.
 * 
 * Функция прогоняет заданную последовательность ключей через переданный кэш, 
 * подсчитывает количество попаданий (hits) и выводит детальную трассировку 
 * состояния памяти на каждом шаге (в зависимости от уровня логирования).
 * 
 * @tparam CacheType Класс тестируемого кэша (например, LRU_Cache<Key>).
 * @tparam Key Тип данных ключа (целые числа, строки и т.д.).
 * 
 * @param N Максимальная вместимость кэша.
 * @param keys Вектор запросов (последовательность обращений к кэшу).
 */
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

/// @name Интеграционные тесты для целочисленных ключей (int)
/// @{
void test_LRU ();
void test_LFU ();
void test_TwoQ ();
void test_ARC ();
void test_LIRS ();  
/// @}

/// @name Интеграционные тесты для строковых ключей (std::string)
/// @{
void string_test_LRU ();
void string_test_LFU ();
void string_test_TwoQ ();
void string_test_ARC ();
void string_test_LIRS ();
/// @}