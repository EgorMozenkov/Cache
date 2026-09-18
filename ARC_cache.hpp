#pragma once

#include <algorithm>
#include <list>
#include <unordered_map>
#include "I_cache.hpp"
#include "Logger.hpp"

template <typename Key>
class ARC_Cache : public ICache<Key> {

    using IT = typename std::list<Key>::iterator;

    enum class QueueType {
        MFU,            // часть с частоиспользуемыми элементами
        MRU,            // малоиспольуемые элементы
        MFU_ghost,
        MRU_ghost,
    };

    struct ItemInfo {
        QueueType type;
        IT list_it;
    };

private:

    std::unordered_map<Key, ItemInfo> ARC_map;
    std::list<Key> MFU_list, MRU_list, MFU_ghost_list, MRU_ghost_list;
    size_t capacity;
    size_t p;                                       // динамический размер MRU


    void replace (const Key& key)
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
        }
        else if (MFU_list.size() > 0) {

            Log::trace(Log::DEBUG, "Удаляем элемент из MFU и добавляем ключ в MFU_ghost\n");

            Key value_2 = MFU_list.back();
            MFU_list.pop_back();

            MFU_ghost_list.push_front(value_2);
            ARC_map[value_2] = {QueueType::MFU_ghost, MFU_ghost_list.begin()};
        }
    }

public:


    ARC_Cache (size_t N) 
    {
        if (N <= 4) {
            throw std::invalid_argument("Ошибка: Размер кэша ARC должен быть больше 4.");
        }

        capacity = N;
        p = 0;
    }


    bool request(const Key& key) override 
    {
        auto it = ARC_map.find(key);

        // Ключ найден в хэш таблицe
        if (it != ARC_map.end()) {

            // Нашли в MRU
            if (it->second.type == QueueType::MRU) {

                Log::trace(Log::TRACE, "ХИТ: Ключ ", key, " в MRU, переносим его в начало MFU\n");

                MRU_list.erase (it->second.list_it);
                MFU_list.push_front(key);
                
                it->second.type = QueueType::MFU;
                it->second.list_it = MFU_list.begin();

                return true;
            }

            // Нашли в MFU
            if (it->second.type == QueueType::MFU) {

                Log::trace(Log::TRACE, "ХИТ: Ключ ", key, " в MFU, переносим его в начало MFU\n");

                MFU_list.splice(MFU_list.begin(), MFU_list, it->second.list_it);

                return true;
            }

            // Нашли в MFU_ghost
            if (it->second.type == QueueType::MFU_ghost) {

                Log::trace(Log::TRACE, "МИСС: Ключ ", key, " ключ найден в MFU_ghost\n");

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
                replace(key);

                MFU_ghost_list.erase (it->second.list_it);
                MFU_list.push_front(key);
                
                it->second.type = QueueType::MFU;
                it->second.list_it = MFU_list.begin();

                return false;
            }

            //Нашли в MRU_ghost
            if (it->second.type == QueueType::MRU_ghost) {
                
                Log::trace(Log::TRACE, "МИСС: Ключ ", key, " ключ найден в MRU_ghost\n");

                size_t delta = 0;
                if (MRU_ghost_list.size() >= MFU_ghost_list.size()) {
                    delta = 1;
                }
                else {
                    delta = MFU_ghost_list.size() / MRU_ghost_list.size(); 
                }


                p = std::min(capacity, p + delta);
                replace(key);

                MRU_ghost_list.erase (it->second.list_it);
                MFU_list.push_front(key);
                ARC_map[key] = {QueueType::MFU, MFU_list.begin()};

                return false;
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
                // MRU_ghost его нет
                ARC_map.erase(MRU_list.back());
                MRU_list.pop_back();
            }
        } 
        else if (MRU_list.size() + MFU_list.size() + MRU_ghost_list.size() + MFU_ghost_list.size() == 2 * capacity) {
            ARC_map.erase(MFU_ghost_list.back());
            MFU_ghost_list.pop_back();
        }
        

        if (MRU_list.size() + MFU_list.size() >= capacity) {
            replace(key);
        }

        Log::trace(Log::TRACE, "МИСС: Ключ ", key, " не найден и добавлен в MRU\n");
        MRU_list.push_front(key);
        ARC_map[key] = {QueueType::MRU, MRU_list.begin()};

        return false;
    }


    void read_cache() override 
    {
        Log::trace(Log::INFO, "Динамический размер p: ", p, "\n");

        Log::trace(Log::INFO, "Начало кэша |  ");
        Log::trace(Log::INFO, "Начало MFU |  ");
        for(Key value : MFU_list) {
            Log::trace(Log::INFO, value, " |  ");
        }
        Log::trace(Log::INFO, "Начало MRU |  ");
        for(Key value : MRU_list) {
            Log::trace(Log::INFO, value, " |  ");
        }
        Log::trace(Log::INFO, "Конец кэша |\n");

        // Запись призрачной части
        Log::trace(Log::DEBUG, "Запись призрачной части\n");
        Log::trace(Log::DEBUG, "MFU_ghost |  ");
        for(Key value : MFU_ghost_list) {
            Log::trace(Log::DEBUG, value, " |  ");
        }
        Log::trace(Log::DEBUG, "\n");

        Log::trace(Log::DEBUG, "MRU_ghost |  ");
        for(Key value : MRU_ghost_list) {
            Log::trace(Log::DEBUG, value, " |  ");
        }
        Log::trace(Log::DEBUG, "\n");
    }
};