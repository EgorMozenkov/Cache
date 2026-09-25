#pragma once

#include <string>
#include <memory>
#include <vector>
#include <stdexcept>

#include "Logger.hpp"
#include "I_cache.hpp"
#include "File_request.hpp"

#include "LRU_cache.hpp"
#include "LFU_cache.hpp"
#include "2Q_cache.hpp"
#include "ARC_cache.hpp"
#include "LIRS_cache.hpp"

/**
 * @brief Реализация многоуровневой системы кэшей.
 * 
 * Создает иерархию уровней (L0, L1, L2...) на основе входящего файла.
 * Управляет данными между кэшами: при попадании на нижнем уровне элемент "поднимается" 
 * на самый быстрый уровень (L0), а вытесненные из L0 элементы 
 * "проваливаются" на уровни ниже, пока не покинут память окончательно.
 */
template <typename Key>
class Multi_Level_Cache : public ICache<Key> {
private:
    std::vector<std::unique_ptr<ICache<Key>>> levels;       ///< Массив уровней иерархии памяти.
    size_t capacity;                                        ///< Суммарная вместимость системы.

public:
    /**
     * @brief Динамически собирает кэши.
     * 
     * Равномерно распределяет общую доступную память (Cache_size) между всеми 
     * заявленными уровнями. Последний уровень забирает возможный остаток от деления.
     * 
     * @param conf Настройки архитектуры (количество уровней и строковые имена алгоритмов).
     * @param Data Входные данные, содержащие суммарный лимит физической памяти.
     * @throw std::invalid_argument Если запрошен неизвестный алгоритм кэширования.
     */
    Multi_Level_Cache(Config& conf, InputData<Key>& Data);

    CacheResult<Key> request(const Key& key) override;
    bool contains(const Key& key) override;
    void erase(const Key& key) override;
    std::vector<Key> get_elements() override;
    void read_cache() override;
};


template <typename Key>
Multi_Level_Cache<Key>::Multi_Level_Cache(Config& conf, InputData<Key>& Data)
{
    capacity = Data.Cache_size;

    size_t base_capasity_cache = capacity / conf.levels;
    size_t capacity_last_level = capacity / conf.levels + capacity % conf.levels;

    for(size_t i = 0; i < conf.levels; i++) {
        size_t current_size;
        
        if(i == conf.levels - 1) {
            current_size = capacity_last_level;
        }
        else {
            current_size = base_capasity_cache;
        }

        if(conf.algorithms[i] == "LRU") {
            levels.push_back(std::make_unique<LRU_Cache<Key>>(current_size));
        }
        else if(conf.algorithms[i] == "LFU") {
            levels.push_back(std::make_unique<LFU_Cache<Key>>(current_size));
        }
        else if(conf.algorithms[i] == "2Q") {
            levels.push_back(std::make_unique<TwoQ_Cache<Key>>(current_size));
        }
        else if(conf.algorithms[i] == "ARC") {
            levels.push_back(std::make_unique<ARC_Cache<Key>>(current_size));
        }
        else if(conf.algorithms[i] == "LIRS") {
            levels.push_back(std::make_unique<LIRS_Cache<Key>>(current_size));
        }
        else {
            throw std::invalid_argument("Неизвестный алгоритм: " + conf.algorithms[i]);
        }
    }
}

template <typename Key>
CacheResult<Key> Multi_Level_Cache<Key>::request(const Key& key)
{
    CacheResult<Key> result = {false, false, Key{}};
    int hit_level = -1;

    // Ищем ключ по всем уровням (от L0 до L_N)
    for(size_t i = 0; i < levels.size(); i++) {
        if(levels[i]->contains(key)) {
            hit_level = i;
            break;
        }
    }

    // Если ключ найден на нижнем уровне, убираем его оттуда для поднятия наверх
    if (hit_level > 0) {
        levels[hit_level]->erase(key);
    }

    // Запрашиваем ключ у первого уровня (L0)
    result = levels[0]->request(key);

    // Если L0 переполнен, проталкиваем вытесненный элемент вниз
    if(result.has_evicted) {
        Key value = result.evicted_key;

        for(size_t i = 1; i < levels.size(); i++) {
            CacheResult<Key> result_value = levels[i]->request(value);
            if(result_value.has_evicted) {
                value = result_value.evicted_key; 
            }
            else {
                break; 
            }
        }
    }

    // Корректируем итоговый статус хита для внешней статистики
    if (hit_level != -1) {
        result.hit = true;
    }

    return result;
}

template <typename Key>
bool Multi_Level_Cache<Key>::contains(const Key& key)
{
    for(size_t i = 0; i < levels.size(); i++) {
        if(levels[i]->contains(key)) return true;
    }
    return false;
}

template <typename Key>
void Multi_Level_Cache<Key>::erase(const Key& key)
{
    for(size_t i = 0; i < levels.size(); i++) {
        levels[i]->erase(key);
    }
}

template <typename Key>
std::vector<Key> Multi_Level_Cache<Key>::get_elements()
{
    std::vector<Key> all_elements;
    for(size_t i = 0; i < levels.size(); i++) {
        std::vector<Key> level_elements = levels[i]->get_elements();
        all_elements.insert(all_elements.end(), level_elements.begin(), level_elements.end());
    }
    return all_elements;
}

template <typename Key>
void Multi_Level_Cache<Key>::read_cache()
{
    Log::trace(Log::INFO, "Начало кэша |  ");
    std::vector<Key> all_elements = get_elements();
    for(const Key& val : all_elements) {
        Log::trace(Log::INFO, val, " |  ");
    }
    Log::trace(Log::INFO, "Конец кэша |\n");
}