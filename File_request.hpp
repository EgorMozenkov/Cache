#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "Logger.hpp"

struct Config {
    int levels;
    std::vector<std::string> algorithms;
};

template <typename Key>
struct InputData {
    size_t Cache_size;
    size_t num_requests;
    std::vector<Key> keys;
};

template <typename Key>
void read_file(Config& conf, InputData<Key>& data)
{
    std::ifstream file("test1.txt");

    if (!file.is_open()) {
        std::cerr << "Ошибка: не удалось открыть файл!" << '\n';
        return;
    }

    file >> conf.levels;
    for (size_t i = 0; i < conf.levels; i++) {

        std::string str_file;
        file >> str_file;
        conf.algorithms.push_back(str_file);
    }

    file >> data.Cache_size >> data.num_requests;

    for (size_t i = 0; i < data.num_requests; i++) {
        Key temp_key;                   
        file >> temp_key;               
        data.keys.push_back(temp_key);  
        
        Log::trace(Log::INFO, "Прочитан ключ: ", temp_key, "\n");
    }

    file.close();
}