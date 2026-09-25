#pragma once

#include <list>
#include <unordered_map>
#include "I_cache.hpp"
#include "Logger.hpp"


/**
 * @brief Алгоритм вытеснения LFU (Least Frequently Used).
 * 
 * Выполнен на частоте обращений к элементам. При переполнении памяти 
 * вытесняет ключ, к которому обращались реже всего. 
 * Реализован со строгой асимптотикой O(1) для всех операций за счет 
 * использования двух хэш-таблиц: одна связывает ключ с его узлом, 
 * вторая группирует ключи по частоте их использования.
 */
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

    /**
     * @brief Инициализирует LFU кэш заданного размера.
     * @param N Максимальная вместимость кэша.
     */
    LFU_Cache(size_t N);


    CacheResult<Key> request(const Key& key) override;    
    bool contains(const Key& key) override;
    void erase(const Key& key) override;
    std::vector<Key> get_elements() override;
    void read_cache() override;
};


template <typename Key>
LFU_Cache<Key>::LFU_Cache(size_t N)
{
    capacity = N;
    min_freq = 0;
}

template <typename Key>
CacheResult<Key> LFU_Cache<Key>::request(const Key& key)
{
    
    if (capacity == 0) return {false, false, Key{}};
    
    CacheResult<Key> result = {false, false, Key{}};
    
    Log::trace(Log::DEBUG, "Запрошен ключ: ", key, "\n");
    auto it = LFU_map_key.find(key);
    
    
    if (it != LFU_map_key.end()) {
        
        Log::trace(Log::DEBUG, "ХИТ: Ключ ", key, " найден в кэше, переносим в начало\n");
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
    
    Log::trace(Log::DEBUG, "МИСС: Ключ ", key, " добавлен в кэш\n");
    
    result.hit = false;
    return result;
}

template <typename Key>
bool LFU_Cache<Key>::contains(const Key& key)
{
    return (LFU_map_key.find(key) != LFU_map_key.end());    
}

template <typename Key>
void LFU_Cache<Key>::erase(const Key& key)
{
    auto it_map = LFU_map_key.find(key);
    
    if (it_map != LFU_map_key.end()) {
        
        int freq = it_map->second.freq;
        auto list_it = it_map->second.list_it;
        
        LFU_map_freq[freq].erase(list_it);
        LFU_map_key.erase(it_map);
    }
}

template <typename Key>
std::vector<Key> LFU_Cache<Key>::get_elements()
{
    std::vector<Key> result;
    for(const auto& item : LFU_map_freq) {
        for (const Key& key : item.second) {
            result.push_back(key);
        }
        
    }
    return result;    
}

template <typename Key>
void LFU_Cache<Key>::read_cache()
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