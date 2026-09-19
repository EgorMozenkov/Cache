#pragma once

#include <algorithm>
#include <list>
#include <unordered_map>
#include "I_cache.hpp"
#include "Logger.hpp"

template <typename Key>
class LIRS_Cache : public ICache<Key> {

    using IT = typename std::list<Key>::iterator;

    enum class QueueType {
        LIR,                    // Список с частоиспользуемыми элементами
        HIR_RESIDENT,           // Список с малоиспользуемыми элементами
        HIR_NO_RESIDENT         // Вытесненные элементы из списков LIR и HIR
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

    // Функция для чистки дна стека S
    void prune ()
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


    void demote_LIR()
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

public:

    LIRS_Cache (size_t N) {
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

    CacheResult<Key> request(const Key& key) override
    {
        auto it = LIRS_map.find(key);
        CacheResult<Key> result = {false, false, Key{}};

        // Ключ найден в хэш-табллице
        if (it != LIRS_map.end()) {

            // Ключ найден в LIR
            if(it->second.type == QueueType::LIR) {

                Log::trace(Log::TRACE, "ХИТ: Ключ ", key, " найден в LIR, переносим в начало\n");
            
                LIRS_list_S.erase(it->second.it_S);
                LIRS_list_S.push_front(key);
                it->second.it_S = LIRS_list_S.begin();
                prune();

                result.hit = true;
                return result;
            }

            // Кдюч найден в HIR_RESIDENT
            if(it->second.type == QueueType::HIR_RESIDENT) {

                Log::trace(Log::TRACE, "ХИТ: Ключ ", key, " найден в HIR, переносим в начало\n");
            
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

                Log::trace(Log::TRACE, "МИСС: Ключ ", key, " найден в призраке HIR\n");

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
        Log::trace(Log::TRACE, "МИСС: Ключ ", key, " не найден и добавлен в HIR\n");

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

        return result;
    }


    void erase(const Key& key) override
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


    void read_cache() override 
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
};