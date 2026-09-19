#pragma once

#include <list>
#include <unordered_map>
#include "I_cache.hpp"
#include "Logger.hpp"

template <typename Key>
class LFU_Cache : public ICache<Key> {

    using IT = typename std::list<Key>::iterator;

    struct ItemInfo {
        int freq;
        IT list_it;
    };

private:
    std::unordered_map<Key, ItemInfo> LFU_map_key;
    std::unordered_map<int, std::list<Key>> LFU_map_freq;
    size_t capacity;
    int min_freq;

public:

    LFU_Cache(size_t N)
    {
        capacity = N;
        min_freq = 0;
    }

    CacheResult<Key> request(const Key& key) override
    {
    if (capacity == 0) return {false, false, Key{}};

    CacheResult<Key> result = {false, false, Key{}};

    Log::trace(Log::DEBUG, "Запрошен ключ: ", key, "\n");
    auto it = LFU_map_key.find(key);


    if (it != LFU_map_key.end()) {

        Log::trace(Log::TRACE, "ХИТ: Ключ ", key, " найден в кэше, переносим в начало\n");
        int freq_before = LFU_map_key[key].freq;
        LFU_map_key[key].freq++;

        // Перемещаем элемент в список с freq + 1
        LFU_map_freq[freq_before + 1].splice
            (LFU_map_freq[freq_before + 1].begin(), 
            LFU_map_freq[freq_before], 
            LFU_map_key[key].list_it);

        // Проверяем не остался ли старый список пустым
        if (LFU_map_freq[freq_before].empty() && min_freq == freq_before) {
            min_freq++;
        }

        result.hit = true;
        result.has_evicted = false;
        result.evicted_key = Key{};
        return result;
    }

    // Проверяем переполнение
    if (LFU_map_key.size() == capacity) {

        Key value_delete = LFU_map_freq[min_freq].back();
        Log::trace(Log::DEBUG, "Вытеснение: Кэш переполнен. Удаляем самый не популярный ключ: ", value_delete, "\n");

        LFU_map_freq[min_freq].pop_back();
        LFU_map_key.erase(value_delete);

        result.has_evicted = true;
        result.evicted_key = value_delete;
    }

    // Ставим в кэш новое значение
    min_freq = 1;
    LFU_map_freq[min_freq].push_front(key);
    LFU_map_key[key].freq = min_freq;
    LFU_map_key[key].list_it = LFU_map_freq[min_freq].begin();

    Log::trace(Log::TRACE, "МИСС: Ключ ", key, " добавлен в кэш\n");

    result.hit = false;
    return result;
    }

    void erase(const Key& key) override
    {
        auto it_map = LFU_map_key.find(key);
    
        if (it_map != LFU_map_key.end()) {

            int freq = it_map->second.freq;
            auto list_it = it_map->second.list_it;
            
            LFU_map_freq[freq].erase(list_it);
            LFU_map_key.erase(it_map);
        }
    }

    void read_cache() override
    {
        Log::trace(Log::INFO, "Начало кэша |  ");
        for(const auto& item : LFU_map_freq) {

            if (item.second.empty()) {
                continue;
            }

            for (const Key& key : item.second) {
                Log::trace(Log::INFO, key, " |  ");
            }
            
        }
        Log::trace(Log::INFO, "Конец кэша |\n");
        Log::trace(Log::INFO, "Текущая min_freq: ", min_freq, "\n");
    }
};