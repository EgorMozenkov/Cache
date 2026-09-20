#pragma once

#include <string>
#include <memory>
#include <vector>

#include "Logger.hpp"
#include "I_cache.hpp"
#include "File_request.hpp"

#include "LRU_cache.hpp"
#include "LFU_cache.hpp"
#include "2Q_cache.hpp"
#include "ARC_cache.hpp"
#include "LIRS_cache.hpp"


template <typename Key>
class Multi_Level_Cache : public ICache<Key> {

    private:

        std::vector<std::unique_ptr<ICache<Key>>> levels;
        size_t capacity;

    public:

        Multi_Level_Cache(Config& conf, InputData<Key>& Data)
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


        CacheResult<Key> request(const Key& key) override
        {
            CacheResult<Key> result = {false, false, Key{}};

            int hit_level = -1;

            for(size_t i = 0; i < levels.size(); i++) {
                if(levels[i]->contains(key)) {
                    hit_level = i;
                    break;
                }
            }


            if (hit_level > 0) {
                levels[hit_level]->erase(key);
            }


            result = levels[0]->request(key);


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


            if (hit_level != -1) {
                result.hit = true;
            }

            return result;
        }


        bool contains(const Key& key) override 
        {
            for(size_t i = 0; i < levels.size(); i++) {
                if(levels[i]->contains(key)) {
                    return true;
                }
            }

            return false;
        }


        void erase(const Key& key) override
        {
            for(size_t i = 0; i < levels.size(); i++) {
                levels[i]->erase(key);
            }
        }


        std::vector<Key> get_elements() override
        {
            std::vector<Key> all_elements;
            
            for(size_t i = 0; i < levels.size(); i++) {
                std::vector<Key> level_elements = levels[i]->get_elements();
                
                all_elements.insert(all_elements.end(), level_elements.begin(), level_elements.end());
            }
            
            return all_elements;
        }


        void read_cache() override
        {
            Log::trace(Log::INFO, "Начало кэша |  ");
            
            std::vector<Key> all_elements = get_elements();
            
            for(const Key& val : all_elements) {
                Log::trace(Log::INFO, val, " |  ");
            }
            
            Log::trace(Log::INFO, "Конец кэша |\n");
        }


};