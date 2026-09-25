#pragma once

#include <algorithm>
#include <list>
#include <unordered_map>
#include <assert.h>

#include "I_cache.hpp"
#include "Logger.hpp"


/**
 * @brief Алгоритм вытеснения LIRS (Low Inter-reference Recurrence Set).
 * 
 * Оценивает полезность элемента не по времени последнего 
 * обращения, а по дистанции между двумя последними обращениями (IRR). 
 * Данные делятся на LIR-блоки (элементы с маленькой дистанцией повторного обращения, 99% памяти) и HIR-блоки 
 * (остальные элементы, к которым мы имеем ключ, 1% памяти).
 * 
 * 
 * Реализован со строгой асимптотикой O(1) для всех операций за счет 
 * связки хэш-таблицы (std::unordered_map) и двух двусвязных списков (std::list): 
 * стека S (хранит историю обращений для расчета IRR, включая "призраков") 
 * и очереди Q (управляет вытеснением резидентных блоков). 
 */
template <typename Key>
class LIRS_Cache : public ICache<Key> {

    using IT = typename std::list<Key>::iterator;

    enum class QueueType {
        LIR,                    ///< Элементы с маленькой дистанцией повторного обращения, находятся в кэше (в стеке S)
        HIR_RESIDENT,           ///< Элементы, физически находящиеся в кэше (в очереди Q).
        HIR_NO_RESIDENT         ///< "Призраки" (только ключи), вытесненные из памяти, но оставшиеся в стеке S.
    };

    struct ItemInfo {
        QueueType type;
        IT it_S;                
        IT it_Q;                
        bool is_in_S;
    };

private:

    std::unordered_map<Key, ItemInfo> LIRS_map;
    std::list<Key> LIRS_list_S, LIRS_list_Q;
    size_t capacity, LIR_capacity;
    size_t count_LIR;

    /**
     * @brief Очищает дно стека S (операция stack pruning).
     * 
     * Удаляет все HIR-элементы со дна стека S до тех пор, пока на дне 
     * не окажется LIR-элемент. Это гарантирует, что стек не будет бесконечно 
     * расти из-за редко запрашиваемых ключей.
     */
    void prune ();
    
    
    /**
     * @brief Понижает статус LIR-блока до HIR-блока.
     * 
     * Вызывается, когда LIR-элемент опускается на самое дно стека S. 
     * Элемент теряет "горячий" статус, переносится в очередь Q 
     * и после этого запускается операция prune().
     */
    void demote_LIR();
    
    public:
    

    /**
     * @brief Инициализирует LIRS кэш.
     * @param N Общий объем памяти. Автоматически выделяет 99% под LIR и HIR-блоки.
     */
    LIRS_Cache (size_t N);
    
    CacheResult<Key> request(const Key& key) override;
    bool contains(const Key& key) override;
    void erase(const Key& key) override;
    std::vector<Key> get_elements() override; 
    void read_cache() override;
};


template <typename Key>
void LIRS_Cache<Key>::prune()
{
    Log::trace(Log::DEBUG, "Начало работы  функции для частки дна стека S");
    while (!LIRS_list_S.empty()) {
        

        Key key_last = LIRS_list_S.back();
        auto last_it = LIRS_map.find(key_last);
        
        if (last_it->second.type == QueueType::LIR) {
            Log::trace(Log::DEBUG, "В стеке S последний элемент из LIR, значит ничего не удаляем");
            break;
        }
            
        last_it->second.is_in_S = false;

        if (last_it->second.type == QueueType::HIR_NO_RESIDENT) {
            Log::trace(Log::DEBUG, "В стеке S последний элемент из HIR_NO_RESIDENT, значит удаляем его из очереди Q");
            LIRS_map.erase(last_it);
        }
        
        Log::trace(Log::DEBUG, "В стеке S последний элемент не из LIR, значит удаляем его из стека S");
        LIRS_list_S.pop_back();
    }
    Log::trace(Log::DEBUG, "Конец работы функции для частки дна стека S");
}

template <typename Key>
void LIRS_Cache<Key>::demote_LIR()
{               
    Key it_LIR_last_in_S = LIRS_list_S.back();
    auto it_map_last_LIR = LIRS_map.find(it_LIR_last_in_S);
    
    it_map_last_LIR->second.type = QueueType::HIR_RESIDENT;
    it_map_last_LIR->second.is_in_S = false;
    LIRS_list_S.pop_back();
    LIRS_list_Q.push_front(it_map_last_LIR->first);
    it_map_last_LIR->second.it_Q = LIRS_list_Q.begin();
    prune();
}


template <typename Key>
LIRS_Cache<Key>::LIRS_Cache(size_t N)
{
    capacity = N;
    count_LIR = 0;
    LIR_capacity = (N * 99) / 100;
    if(LIR_capacity == 0) {
        LIR_capacity = 1;
    }
    
    if(LIR_capacity == capacity) {
        LIR_capacity = N - 1;
    }
}

template <typename Key>
CacheResult<Key> LIRS_Cache<Key>::request(const Key& key)
{
    auto it = LIRS_map.find(key);
    CacheResult<Key> result = {false, false, Key{}};
    
    // Ключ найден в хэш-табллице
    if (it != LIRS_map.end()) {
        
        // Ключ найден в LIR
        if(it->second.type == QueueType::LIR) {
            
            Log::trace(Log::DEBUG, "ХИТ: Ключ ", key, " найден в LIR, переносим в начало\n");
            
            LIRS_list_S.erase(it->second.it_S);
            LIRS_list_S.push_front(key);
            it->second.it_S = LIRS_list_S.begin();
            prune();
            
            result.hit = true;
            return result;
        }
        
        // Кдюч найден в HIR_RESIDENT
        if(it->second.type == QueueType::HIR_RESIDENT) {
            
            Log::trace(Log::DEBUG, "ХИТ: Ключ ", key, " найден в HIR, переносим в начало\n");
            
            bool in_stack = it->second.is_in_S;
            
            if (in_stack) {
                LIRS_list_S.erase(it->second.it_S);
            }
            LIRS_list_S.push_front(key);
            it->second.it_S = LIRS_list_S.begin();
            it->second.is_in_S = true;
            
            if (in_stack) {
                
                LIRS_list_Q.erase(it->second.it_Q);
                it->second.type = QueueType::LIR;
                
                demote_LIR();
            }
            else {
                LIRS_list_Q.erase(it->second.it_Q);
                LIRS_list_Q.push_front(it->first);
                it->second.it_Q = LIRS_list_Q.begin();
            }
            
            result.hit = true;
            return result;
        }
        
        // Ключ найден в HIR_NO_RESIDENT
        if(it->second.type == QueueType::HIR_NO_RESIDENT) {
            
            Log::trace(Log::DEBUG, "МИСС: Ключ ", key, " найден в призраке HIR\n");
            
            if (count_LIR + LIRS_list_Q.size() >= capacity) {
                Key victim = LIRS_list_Q.back();
                auto victim_it = LIRS_map.find(victim);
                
                LIRS_list_Q.pop_back();
                
                if (victim_it->second.is_in_S) {
                    victim_it->second.type = QueueType::HIR_NO_RESIDENT;
                } else {
                    LIRS_map.erase(victim_it);
                }
                
                result.has_evicted = true;
                result.evicted_key = victim;
            }
            
            bool in_stack = it->second.is_in_S;
            
            if (in_stack) {
                LIRS_list_S.erase(it->second.it_S);
            }
            LIRS_list_S.push_front(key);
            it->second.it_S = LIRS_list_S.begin();
            it->second.is_in_S = true;
            
            if (in_stack) {
                
                it->second.type = QueueType::LIR;
                
                demote_LIR();
                
            }
            else {
                
                LIRS_list_Q.push_front(it->first);
                it->second.it_Q = LIRS_list_Q.begin();
            }
            return result;
        }
    }
    
    //Ключ не найден в хэш-таблице
    Log::trace(Log::DEBUG, "МИСС: Ключ ", key, " не найден и добавлен в HIR\n");
    
    if (count_LIR < LIR_capacity) {
        LIRS_list_S.push_front(key);
        LIRS_map[key] = {QueueType::LIR, LIRS_list_S.begin(), LIRS_list_Q.end(), true};
        count_LIR++;
    }
    else {
        
        if (LIRS_list_Q.size() >= capacity - LIR_capacity) {

            Key last_key_in_Q = LIRS_list_Q.back();
            auto last_it = LIRS_map.find(last_key_in_Q);
            
            LIRS_list_Q.pop_back();
            
            if (last_it->second.is_in_S) {
                
                last_it->second.type = QueueType::HIR_NO_RESIDENT;
            } 
            else {
                LIRS_map.erase(last_it);
            }
            
            result.has_evicted = true;
            result.evicted_key = last_key_in_Q;
        }
        
        LIRS_list_S.push_front(key);
        LIRS_list_Q.push_front(key);
        LIRS_map[key] = {QueueType::HIR_RESIDENT, LIRS_list_S.begin(), LIRS_list_Q.begin(), true};
    }
    
    assert(count_LIR + LIRS_list_Q.size() <= capacity);
    return result;
}

template <typename Key>
bool LIRS_Cache<Key>::contains(const Key& key)
{
    auto it = LIRS_map.find(key);
    
    if (it == LIRS_map.end()) {
        return false;
    }
    
    if (it->second.type == QueueType::LIR || it->second.type == QueueType::HIR_RESIDENT) {
        return true;
    }
    
    return false;
}

template <typename Key>
void LIRS_Cache<Key>::erase(const Key& key)
{
    auto it_map = LIRS_map.find(key);
    
    if (it_map != LIRS_map.end()) {
        
        if(it_map->second.type == QueueType::LIR) {
            LIRS_list_S.erase(it_map->second.it_S);
            count_LIR--;
        }
        else if(it_map->second.type == QueueType::HIR_RESIDENT) {
            LIRS_list_Q.erase(it_map->second.it_Q);
            if (it_map->second.is_in_S) {
                LIRS_list_S.erase(it_map->second.it_S);
            }
        }
        else if(it_map->second.type == QueueType::HIR_NO_RESIDENT) {
            LIRS_list_S.erase(it_map->second.it_S);
        }
        
        LIRS_map.erase(it_map);
    }
}

template <typename Key>
std::vector<Key> LIRS_Cache<Key>::get_elements()
{
    std::vector<Key> result;
    for (const Key& value : LIRS_list_S) {
        if (LIRS_map[value].type == QueueType::LIR) { 
            result.push_back(value);
        }
    }
    
    for (const Key& value : LIRS_list_Q) {
        result.push_back(value);
    }
    return result;
}

template <typename Key>
void LIRS_Cache<Key>::read_cache()
{
    Log::trace(Log::INFO, "Физическое содержимое кэша:\n");
    
    Log::trace(Log::INFO, "[VIP / LIR]:  |  ");
    for (Key value : LIRS_list_S) {
        auto it = LIRS_map.find(value);
        if (it != LIRS_map.end() && it->second.type == QueueType::LIR) {
            Log::trace(Log::INFO, value, "  |  ");
        }
    }
    
    Log::trace(Log::INFO, "[Обычные / HIR]:  |  ");
    for (Key value : LIRS_list_Q) {
        Log::trace(Log::INFO, value, "  |  ");
    }
    Log::trace(Log::INFO, "\n");
    
    Log::trace(Log::DEBUG, "[Призраки (Только история)]:  |  ");
    for (Key value : LIRS_list_S) {
        auto it = LIRS_map.find(value);
        if (it != LIRS_map.end() && it->second.type == QueueType::HIR_NO_RESIDENT) {
            Log::trace(Log::DEBUG, value, "  |  ");
        }
    }
    Log::trace(Log::DEBUG, "\n");
}