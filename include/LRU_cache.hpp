#pragma once

#include <list>
#include <unordered_map>
#include "I_cache.hpp"
#include "Logger.hpp"


/**
 * @brief Алгоритм вытеснения LRU (Least Recently Used).
 * 
 * Поддерживает ключи в порядке их последнего использования. При переполнении 
 * памяти вытесняет элемент, к которому обращались давнее всего. 
 * Гарантирует асимптотику O(1) для всех операций благодаря связке 
 * двусвязного списка (std::list) и хэш-таблицы (std::unordered_map).
 */
template <typename Key>
class LRU_Cache : public ICache<Key> {

    using IT = typename std::list<Key>::iterator;

private:

    std::list<Key> LRU_list;
    std::unordered_map<Key, IT> LRU_map;
    size_t capacity;

public:

    /**
     * @brief Инициализирует LRU кэш заданного размера.
     * @param N Максимальное количество элементов, которое может храниться в кэше.
     */
    LRU_Cache(size_t N);

    CacheResult<Key> request(const Key& key) override;
    bool contains(const Key& key) override;
    void erase(const Key& key) override;
    std::vector<Key> get_elements() override;
    void read_cache() override;
};


template <typename Key>
LRU_Cache<Key>::LRU_Cache(size_t N)
{
    capacity = N;
}

template <typename Key>
CacheResult<Key> LRU_Cache<Key>::request(const Key& key)
{
    if (capacity == 0) return {false, false, Key{}};
        CacheResult<Key> result = {false, false, Key{}};

        Log::trace(Log::DEBUG, "Запрошен ключ: ", key, "\n");
        auto it = LRU_map.find(key);

        if (it != LRU_map.end()) {

            Log::trace(Log::DEBUG, "ХИТ: Ключ ", key, " найден в кэше, переносим в начало\n");
            LRU_list.splice (LRU_list.begin(), LRU_list, it->second);

            result.hit = true;
            result.has_evicted = false;
            result.evicted_key = Key{};

            return result;
        }

        if (LRU_list.size() == capacity) {

            Key last_value = LRU_list.back();

            Log::trace(Log::DEBUG, "Вытеснение: Кэш переполнен. Удаляем старейший ключ: ", last_value, "\n");

            LRU_map.erase(last_value);
            LRU_list.pop_back();

            result.has_evicted = true;
            result.evicted_key = last_value;
        }

        Log::trace(Log::DEBUG, "МИСС: Ключ ", key, " добавлен в кэш\n");

        LRU_list.push_front(key);
        LRU_map[key] = LRU_list.begin();

        result.hit = false;

        return result;
}

template <typename Key>
bool LRU_Cache<Key>::contains(const Key& key)
{
    return (LRU_map.find(key) != LRU_map.end());
}

template <typename Key>
void LRU_Cache<Key>::erase(const Key& key)
{
    auto it_map = LRU_map.find(key);

    if (it_map != LRU_map.end()) {
        LRU_list.erase(it_map->second);
        LRU_map.erase(it_map);
    }
}

template <typename Key>
std::vector<Key> LRU_Cache<Key>::get_elements()
{
    std::vector<Key> result;
    for (const Key& key : LRU_list) {
        result.push_back(key);
    }
    return result;
}

template <typename Key>
void LRU_Cache<Key>::read_cache()
{
    Log::trace(Log::INFO, "Начало кэша |  ");
    for(const Key& value : LRU_list) {
        Log::trace(Log::INFO, value, " |  ");
    }
    Log::trace(Log::INFO, "Конец кэша |\n");
}