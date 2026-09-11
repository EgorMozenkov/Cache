#include <iostream>
#include <list>
#include <unordered_map>

#include "Logger.hpp"
#include "I_cache.hpp"
#include "LRU_cache.hpp"

LRU_Cache::LRU_Cache(size_t N) {
    capacity = N;
}

bool LRU_Cache::request (int key) 
{
    Log::trace(Log::TRACE, "Запрошен ключ: %d\n", key);
    auto it = LRU_map.find(key);

    if (it != LRU_map.end()) {

        Log::trace(Log::TRACE, "ХИТ: Ключ %d найден в кэше, переносим в начало\n", key);
        LRU_list.splice (LRU_list.begin(), LRU_list, it->second);

        return 1;
    }

    if (LRU_list.size() == capacity) {

        int last_value = LRU_list.back();

        Log::trace(Log::TRACE, "Вытеснение: Кэш переполнен. Удаляем старейший ключ: %d\n", last_value);

        LRU_map.erase(last_value);
        LRU_list.pop_back();
    }

    Log::trace(Log::TRACE, "МИСС: Ключ %d добавлен в кэш\n", key);

    LRU_list.push_front(key);
    LRU_map[key] = LRU_list.begin();

    return 0;
}

void LRU_Cache::read_cache ()
{
    Log::trace(Log::INFO, "Начало кэша |  ");
    for(int value : LRU_list) {
        Log::trace(Log::INFO, "%d  |  ", value);
    }
    Log::trace(Log::INFO, "Конец кэша |\n");
}
