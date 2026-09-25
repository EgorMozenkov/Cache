#pragma once

#include <list>
#include <unordered_map>
#include "I_cache.hpp"
#include "Logger.hpp"


/**
 * @brief Алгоритм вытеснения 2Q (Two Queues).
 * 
 * Решает проблему уязвимости классического LRU к полному сканированию данных. 
 * Разделяет память на три независимые очереди: IN (для новых элементов), 
 * OUT (теневой кэш для истории вытесненных) и VIP (для часто используемых). 
 * Элемент признается "горячим" и попадает в VIP только в том случае, если 
 * повторное обращение к нему произошло, пока его ключ находился в очереди OUT.
 */
template <typename Key>
class TwoQ_Cache : public ICache<Key> {

    using IT = typename std::list<Key>::iterator;

    enum class QueueType {
        IN,
        OUT,
        VIP,
    };

    struct ItemInfo {
        QueueType type;
        IT list_it;
    };

private:

    std::unordered_map<Key, ItemInfo> TwoQ_map;
    std::list<Key> IN_list, OUT_list, VIP_list;
    size_t size_IN, size_OUT, size_VIP;
    size_t capacity;

public:
    
    /**
     * @brief Инициализирует 2Q кэш.
     * @param N Общий объем кэша. Память автоматически делится в пропорциях: 
     *          25% под очередь IN, 50% под OUT и остаток под VIP.
     * @throw std::invalid_argument Если размер кэша меньше или равен 3.
     */
    TwoQ_Cache (size_t N); 

    
    CacheResult<Key> request(const Key& key) override;
    bool contains(const Key& key) override;
    void erase(const Key& key) override;
    std::vector<Key> get_elements() override;
    void read_cache() override;
};


template <typename Key>
TwoQ_Cache<Key>::TwoQ_Cache(size_t N)
{
    if (N <= 3) {
        throw std::invalid_argument("Ошибка: Размер кэша 2Q должен быть больше 3.");
    }
    size_IN = N / 4;
    size_VIP = N - size_IN;
    size_OUT = N / 2;
    capacity = N;
}

template <typename Key>
CacheResult<Key> TwoQ_Cache<Key>::request(const Key& key)
{
    auto it = TwoQ_map.find(key);
    CacheResult<Key> result = {false, false, Key{}};       
    
    // Элемента нет ни в одной из очередей
    if (it == TwoQ_map.end()) {
        
        // Проверяем переполнение IN
        if (IN_list.size() >= size_IN) {
            Key value_1 = IN_list.back();
            IN_list.pop_back();
            
            // Проверяем переполнение OUT
            if (OUT_list.size() >= size_OUT) {
                Key value_2 = OUT_list.back();
                Log::trace(Log::DEBUG, "Вытеснение: Удаляем старейший ключ из OUT: ", value_2, "\n");
                OUT_list.pop_back();
                TwoQ_map.erase(value_2);
                
                result.has_evicted = true;
                result.evicted_key = value_2;
            }
            
            // Переносим элемент из IN в OUT
            OUT_list.push_front(value_1);
            TwoQ_map[value_1] = {QueueType::OUT, OUT_list.begin()};
        }
        
        // Добавляем новый ключ в IN
        IN_list.push_front(key);
        TwoQ_map[key] = {QueueType::IN, IN_list.begin()};
        
        Log::trace(Log::DEBUG, "МИСС: Ключ ", key, " добавлен в IN\n");
        return result;
    }
    
    // Ключ найден
    auto& node = it->second;
    
    if (node.type == QueueType::VIP) {
        VIP_list.splice(VIP_list.begin(), VIP_list, node.list_it);
        Log::trace(Log::DEBUG, "ХИТ: Ключ ", key, " в VIP, переносим в начало\n");
        
        result.hit = true;
        return result;
    }
    else if (node.type == QueueType::IN) {
        Log::trace(Log::DEBUG, "ХИТ: Ключ ", key, " в IN, оставляем на месте\n");
        
        result.hit = true;
        return result;
    }
    else if (node.type == QueueType::OUT) {
        
        // Проверяем переполнение VIP
        if (VIP_list.size() >= size_VIP) {
            Key evicted_vip = VIP_list.back();
            Log::trace(Log::DEBUG, "Вытеснение: VIP переполнен. Удаляем: ", evicted_vip, "\n");
            TwoQ_map.erase(evicted_vip);
            VIP_list.pop_back();
            
            result.has_evicted = true;
            result.evicted_key = evicted_vip;
        }
        
        OUT_list.erase(node.list_it);
        
        VIP_list.push_front(key);
        
        node.type = QueueType::VIP;
        node.list_it = VIP_list.begin();
        
        Log::trace(Log::DEBUG, "МИСС: Ключ ", key, " повышен из OUT в VIP\n");
        return result;
    }
    
    return result;
}

template <typename Key>
bool TwoQ_Cache<Key>::contains(const Key& key)
{
    auto it = TwoQ_map.find(key);
    
    if (it == TwoQ_map.end()) {
        return false;
    }
    
    if (it->second.type == QueueType::IN || it->second.type == QueueType::VIP) {
        return true;
    }
    
    return false;
}

template <typename Key>
void TwoQ_Cache<Key>::erase(const Key& key)
{
    auto it_map = TwoQ_map.find(key);
    
    if (it_map != TwoQ_map.end()) {
        
        if(it_map->second.type == QueueType::IN) {
            IN_list.erase(it_map->second.list_it);
        }
        else if(it_map->second.type == QueueType::VIP) {
            VIP_list.erase(it_map->second.list_it);
        }
        else if(it_map->second.type == QueueType::OUT) {
            OUT_list.erase(it_map->second.list_it);
        }
        
        TwoQ_map.erase(it_map);
    }
}

template <typename Key>
std::vector<Key> TwoQ_Cache<Key>::get_elements()
{
    std::vector<Key> result;
    for(const Key& value : VIP_list) {
        result.push_back(value);
    }
    for(const Key& value : IN_list) {
        result.push_back(value);
    }
    return result;
}


template <typename Key>
void TwoQ_Cache<Key>::read_cache()
{
    Log::trace(Log::INFO, "Начало кэша |  ");
    for(const Key& value : VIP_list) {
        Log::trace(Log::INFO, value, " |  ");
    }
    for(const Key& value : IN_list) {
        Log::trace(Log::INFO, value, " |  ");
    }
    Log::trace(Log::INFO, "Конец кэша |\n");
}