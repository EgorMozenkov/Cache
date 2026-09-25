#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <algorithm>
#include "Logger.hpp"

/**
 * @brief Анализатор идеального кэширования (Алгоритм Белади).
 * 
 * Не является частью многоуровневой системы системы (не наследуется от ICache). 
 * Используется исключительно как теоретический предел для результатов с реальными кэшами.
 * "Заглядывает в будущее", зная все запросы наперед, и всегда 
 * вытесняет тот элемент, который понадобится позже всех.
 */
template <typename Key>
class Ideal_Cache {
private:

    size_t capacity;
    std::unordered_set<Key> cache_memory;                                ///< Физическая память (что сейчас лежит в кэше)
    std::unordered_map<Key, std::queue<size_t>> future_requests;        ///< Для каждого ключа храним очередь индексов (шагов), когда он будет запрошен.

public:
    /**
     * @brief Конструктор. Сразу строит карту будущих запросов.
     */
    Ideal_Cache(size_t N, const std::vector<Key>& data)
    {
        capacity = N;
        for (size_t i = 0; i < data.size(); ++i) {
            future_requests[data[i]].push(i);
        }
    } 

    /**
     * @brief Пошаговый запрос.
     */
    CacheResult<Key> request(const Key& current_key) 
    {
        CacheResult<Key> result;
        result.hit = false;

        if (capacity == 0) return result;

        // Удаляем текущий момент времени из очереди (если он там есть)
        if (!future_requests[current_key].empty()) {
            future_requests[current_key].pop();
        }

        // Если ключ уже в кэше -> ХИТ
        if (cache_memory.find(current_key) != cache_memory.end()) {
            result.hit = true;
            return result;
        }

        // МИСС. Проверяем, есть ли свободное место.
        if (cache_memory.size() < capacity) {
            cache_memory.insert(current_key);
            return result;
        }

        // Память переполнена. Ищем жертву по алгоритму Белади.
        Key victim = *cache_memory.begin();
        size_t max_future_index = 0;

        for (const Key& cached_key : cache_memory) {
            if (future_requests[cached_key].empty()) {
                victim = cached_key;
                break; 
            }

            size_t next_use = future_requests[cached_key].front();
            if (next_use > max_future_index) {
                max_future_index = next_use;
                victim = cached_key;
            }
        }

        cache_memory.erase(victim);
        cache_memory.insert(current_key);

        return result;
    }

    void read_cache()
    {
        Log::trace(Log::INFO, "Начало кэша |  ");
        for(const Key& value : cache_memory) {
            Log::trace(Log::INFO, value, " |  ");
        }
        Log::trace(Log::INFO, "Конец кэша |\n");
    }
};
