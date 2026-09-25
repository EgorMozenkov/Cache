#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "Logger.hpp"

/**
 * @brief Конфигурация многоуровневой системы кэшей.
 * 
 * Хранит архитектурные настройки, считанные из файла или переданные 
 * генератором. Определяет глубину системы и алгоритм для каждого уровня.
 */
struct Config {
    size_t levels;                                  ///< Количество уровней кэша (например, 2 для L1->L2).
    std::vector<std::string> algorithms;            ///< Названия алгоритмов по уровням (например, {"LFU", "ARC"}).
};


/**
 * @brief Набор тестовых данных для прогона тестов.
 * 
 * Содержит общую доступную память и саму последовательность обращений, 
 * которая будет подана на вход первому уровню кэша.
 * 
 * @tparam Key Тип данных ключа (соответствует типу кэша).
 */
template <typename Key>
struct InputData {
    size_t Cache_size;                      ///< Суммарная вместимость всех уровней кэша.
    size_t num_requests;                    ///< Общее количество запросов (длина трассы).
    std::vector<Key> keys;                  ///< Вектор ключей, имитирующий поток обращений.
};


/**
 * @brief Парсит текстовый файл с конфигурацией и входными данными.
 * 
 * Функция считывает архитектуру системы и запросы.
 * Если файл не найден или недоступен, выводит ошибку в поток cerr и прерывает чтение.
 * 
 * @tparam Key Тип считываемых ключей (тип должен поддерживать оператор >>).
 * @param[in] filename Путь к текстовому файлу с данными.
 * @param[out] conf Ссылка на структуру конфигурации для записи считанных настроек.
 * @param[out] data Ссылка на структуру данных для записи потока запросов.
 */
template <typename Key>
void read_file(const std::string& filename, Config& conf, InputData<Key>& data)
{
    std::ifstream file(filename);

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
        
        Log::trace(Log::DEBUG, "Прочитан ключ: ", temp_key, "\n");
    }

    file.close();
}