#include <iostream>
#include <list>
#include <unordered_map>

#include "Logger.hpp"
#include "I_cache.hpp"
#include "LFU_cache.hpp"

LFU_Cache::LFU_Cache (size_t N) 
{
    capacity = N;
    min_freq = 0;
}

bool LFU_Cache::request (int key)
{
    if (capacity == 0) return false;

    Log::trace(Log::DEBUG, "Запрошен ключ: %d\n", key);
    auto it = LFU_map_key.find(key);


    if (it != LFU_map_key.end()) {

        Log::trace(Log::TRACE, "ХИТ: Ключ %d найден в кэше, переносим в начало\n", key);
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

        return true;
    }

    // Проверяем переполнение
    if (LFU_map_key.size() == capacity) {

        int value_delete = LFU_map_freq[min_freq].back();
        Log::trace(Log::DEBUG, "Вытеснение: Кэш переполнен. Удаляем самый не популярный ключ: %d\n", value_delete);

        LFU_map_freq[min_freq].pop_back();
        LFU_map_key.erase(value_delete);
    }

    // Ставим в кэш новое значение
    min_freq = 1;
    LFU_map_freq[min_freq].push_front(key);
    LFU_map_key[key].freq = min_freq;
    LFU_map_key[key].list_it = LFU_map_freq[min_freq].begin();

    Log::trace(Log::TRACE, "МИСС: Ключ %d добавлен в кэш\n", key);

    return false;
}

void LFU_Cache::read_cache ()
{
    Log::trace(Log::INFO, "Начало кэша |  ");
    for(const auto& item : LFU_map_freq) {

        if (item.second.empty()) {
            continue;
        }

        for (int key : item.second) {
            Log::trace(Log::INFO, "%d  |  ", key);
        }
        
    }
    Log::trace(Log::INFO, "Конец кэша |\n");
    Log::trace(Log::INFO, "Текущая min_freq: %d\n", min_freq);
}