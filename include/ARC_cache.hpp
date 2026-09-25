#pragma once

#include <algorithm>
#include <list>
#include <unordered_map>
#include "I_cache.hpp"
#include "Logger.hpp"


/**
 * @brief Алгоритм вытеснения ARC (Adaptive Replacement Cache).
 * 
 * Динамически балансирует между недавностью (MRU) и частотой (MFU) использования, адаптируясь под паттерны нагрузки.
 * Использует "призрачные" очереди (ghost lists) для отслеживания вытесненных ключей. 
 * Самостоятельно настраивает целевой размер MRU (параметр p) на основе промахов.
 */
template <typename Key>
class ARC_Cache : public ICache<Key> {

    using IT = typename std::list<Key>::iterator;

    enum class QueueType {
        MFU,            ///< Часто используемые элементы
        MRU,            ///< Недавно использованные элементы
        MFU_ghost,      ///< История вытесненных из MFU
        MRU_ghost,      ///< История вытесненных из MRU
    };

    struct ItemInfo {
        QueueType type;
        IT list_it;
    };

private:

    std::unordered_map<Key, ItemInfo> ARC_map;
    std::list<Key> MFU_list, MRU_list, MFU_ghost_list, MRU_ghost_list;
    size_t capacity;
    size_t p;                                       ///< Динамический размер очереди MRU


    /**
     * @brief Подпрограмма балансировки.
     * 
     * Выбирает, откуда вытеснять физический элемент: из MRU или MFU, 
     * опираясь на адаптивный параметр 'p' и граничные условия.
     * 
     * @param key Запрашиваемый ключ (необходим для проверки попадания в MFU_ghost).
     * @param[out] result Ссылка на структуру результата, куда записывается жертва.
     */
    void replace (const Key& key, CacheResult<Key>& result);
    
    public:
    
    /**
     * @brief Инициализирует кэш ARC.
     * @param N Общая вместимость кэша (capacity). Призрачные очереди B1 и B2 
     *          также получат суммарный лимит N.
     * @throw std::invalid_argument Если размер кэша меньше или равен 4.
     */
    ARC_Cache (size_t N);
    
    
    CacheResult<Key> request(const Key& key) override;
    bool contains(const Key& key) override;
    void erase(const Key& key) override;
    std::vector<Key> get_elements() override; 
    void read_cache() override;
};

template <typename Key>
void ARC_Cache<Key>::replace(const Key& key, CacheResult<Key>& result)
{
    auto it = ARC_map.find(key);
    bool is_mfu_ghost = false;


    if (it != ARC_map.end() && it->second.type == QueueType::MFU_ghost) {
        is_mfu_ghost = true;
    }

    if (MRU_list.size() > p || (is_mfu_ghost && MRU_list.size() == p && p > 0) ) {

        Log::trace(Log::DEBUG, "Размер MRU больше динамического размера p. Удаляем элемент из MRU и добавляем ключ в MRU_ghost\n");

        Key value_1 = MRU_list.back();
        MRU_list.pop_back();

        MRU_ghost_list.push_front(value_1);
        ARC_map[value_1] = {QueueType::MRU_ghost, MRU_ghost_list.begin()};

        result.has_evicted = true;
        result.evicted_key = value_1;
    }
    else if (MFU_list.size() > 0) {

        Log::trace(Log::DEBUG, "Удаляем элемент из MFU и добавляем ключ в MFU_ghost\n");

        Key value_2 = MFU_list.back();
        MFU_list.pop_back();

        MFU_ghost_list.push_front(value_2);
        ARC_map[value_2] = {QueueType::MFU_ghost, MFU_ghost_list.begin()};
    
        result.has_evicted = true;
        result.evicted_key = value_2;
    }
}


template <typename Key>
ARC_Cache<Key>::ARC_Cache(size_t N)
{
    if (N <= 4) {
        throw std::invalid_argument("Ошибка: Размер кэша ARC должен быть больше 4.");
    }
    
    capacity = N;
    p = 0;
}

template <typename Key>
CacheResult<Key> ARC_Cache<Key>::request(const Key& key)
{
    auto it = ARC_map.find(key);
    CacheResult<Key> result = {false, false, Key{}};
    
    // Ключ найден в хэш таблицe
    if (it != ARC_map.end()) {
        
        // Нашли в MRU
        if (it->second.type == QueueType::MRU) {
            
            Log::trace(Log::DEBUG, "ХИТ: Ключ ", key, " в MRU, переносим его в начало MFU\n");
            
            MRU_list.erase (it->second.list_it);
            MFU_list.push_front(key);
            
            it->second.type = QueueType::MFU;
            it->second.list_it = MFU_list.begin();
            
            result.hit = true;
            return result;
        }
        
        // Нашли в MFU
        if (it->second.type == QueueType::MFU) {
            
            Log::trace(Log::DEBUG, "ХИТ: Ключ ", key, " в MFU, переносим его в начало MFU\n");
            
            MFU_list.splice(MFU_list.begin(), MFU_list, it->second.list_it);
            
            result.hit = true;
            return result;
        }
        
        // Нашли в MFU_ghost
        if (it->second.type == QueueType::MFU_ghost) {
            
            Log::trace(Log::DEBUG, "МИСС: Ключ ", key, " ключ найден в MFU_ghost\n");
            
            size_t delta = 0;
            if (MFU_ghost_list.size() >= MRU_ghost_list.size()) {
                delta = 1;
            }
            else {
                delta = MRU_ghost_list.size() / MFU_ghost_list.size(); 
            }
            
            if (p > delta) {
                p = std::min(capacity, p - delta);
            }
            else {
                p = 0;
            }
            replace(key, result);
            
            MFU_ghost_list.erase (it->second.list_it);
            MFU_list.push_front(key);
            
            it->second.type = QueueType::MFU;
            it->second.list_it = MFU_list.begin();
            
            return result;
        }
        
        //Нашли в MRU_ghost
        if (it->second.type == QueueType::MRU_ghost) {
            
            Log::trace(Log::DEBUG, "МИСС: Ключ ", key, " ключ найден в MRU_ghost\n");
            
            size_t delta = 0;
            if (MRU_ghost_list.size() >= MFU_ghost_list.size()) {
                delta = 1;
            }
            else {
                delta = MFU_ghost_list.size() / MRU_ghost_list.size(); 
            }
            
            
            p = std::min(capacity, p + delta);
            replace(key, result);
            
            MRU_ghost_list.erase (it->second.list_it);
            MFU_list.push_front(key);
            ARC_map[key] = {QueueType::MFU, MFU_list.begin()};
            
            return result;
        }
        
    }
    
    // Ключа нет в хэш-таблице
    
    if (MRU_list.size() + MRU_ghost_list.size() == capacity) {
        if (MRU_list.size() < capacity) {
            // Удаляем самого старого из MRU_ghost
            ARC_map.erase(MRU_ghost_list.back());
            MRU_ghost_list.pop_back();
            
        } 
        else {
            // в MRU_ghost его нет
            
            result.has_evicted = true;
            result.evicted_key = MRU_list.back();
            
            ARC_map.erase(MRU_list.back());
            MRU_list.pop_back();
        }
    } 
    else if (MRU_list.size() + MFU_list.size() + MRU_ghost_list.size() + MFU_ghost_list.size() == 2 * capacity) {
        ARC_map.erase(MFU_ghost_list.back());
        MFU_ghost_list.pop_back();
    }
    
    
    if (MRU_list.size() + MFU_list.size() >= capacity) {
        replace(key, result);
    }
    
    Log::trace(Log::TRACE, "МИСС: Ключ ", key, " не найден и добавлен в MRU\n");
    MRU_list.push_front(key);
    ARC_map[key] = {QueueType::MRU, MRU_list.begin()};
    
    return result;
}

template <typename Key>
bool ARC_Cache<Key>::contains(const Key& key)
{
    auto it = ARC_map.find(key);
    
    if (it == ARC_map.end()) {
        return false;
    }

    if (it->second.type == QueueType::MFU || it->second.type == QueueType::MRU) {
        return true;
    }
    
    return false;
}

template <typename Key>
void ARC_Cache<Key>::erase(const Key& key)
{
    auto it_map = ARC_map.find(key);
    
    if (it_map != ARC_map.end()) {
        
        if(it_map->second.type == QueueType::MFU) {
            MFU_list.erase(it_map->second.list_it);
        }
        else if(it_map->second.type == QueueType::MRU) {
            MRU_list.erase(it_map->second.list_it);
        }
        else if(it_map->second.type == QueueType::MFU_ghost) {
            MFU_ghost_list.erase(it_map->second.list_it);
        }
        else if(it_map->second.type == QueueType::MRU_ghost) {
            MRU_ghost_list.erase(it_map->second.list_it);
        }
        
        ARC_map.erase(it_map);
    }
}

template <typename Key>
std::vector<Key> ARC_Cache<Key>::get_elements()
{
    std::vector<Key> result;
    for(const Key& value : MFU_list) {
        result.push_back(value);
    }
    for(const Key& value : MRU_list) {
        result.push_back(value);
    }
    return result;
}

template <typename Key>
void ARC_Cache<Key>::read_cache()
{
    Log::trace(Log::INFO, "Динамический размер p: ", p, "\n");
    
    Log::trace(Log::INFO, "Начало кэша |  ");
    Log::trace(Log::INFO, "Начало MFU |  ");
    for(const Key& value : MFU_list) {
        Log::trace(Log::INFO, value, " |  ");
    }
    Log::trace(Log::INFO, "Начало MRU |  ");
    for(const Key& value : MRU_list) {
        Log::trace(Log::INFO, value, " |  ");
    }
    Log::trace(Log::INFO, "Конец кэша |\n");
    
    // Запись призрачной части
    Log::trace(Log::DEBUG, "Запись призрачной части\n");
    Log::trace(Log::DEBUG, "MFU_ghost |  ");
    for(const Key& value : MFU_ghost_list) {
        Log::trace(Log::DEBUG, value, " |  ");
    }
    Log::trace(Log::DEBUG, "\n");
    
    Log::trace(Log::DEBUG, "MRU_ghost |  ");
    for(const Key& value : MRU_ghost_list) {
        Log::trace(Log::DEBUG, value, " |  ");
    }
    Log::trace(Log::DEBUG, "\n");
}