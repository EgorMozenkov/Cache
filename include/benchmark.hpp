#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>


/**
 * @brief Структура для хранения результатов одиночного прогона бенчмарка.
 * 
 * Связывает конкретную архитектуру одной многоуровневой системы с ее итоговой эффективностью.
 */
struct BenchResult {
    std::vector<std::string> algos;             ///< Цепочка алгоритмов (например, {"LFU", "ARC"}).
    size_t hits;                                ///< Количество попаданий (hit count) для данной цепочки.
};


/**
 * @brief Автоматический поиск оптимальной конфигурации кэша.
 * 
 * Выполняет полный перебор всех доступных одиночных алгоритмов 
 * и их парных комбинаций (двухуровневых систем). Прогоняет входную трассу 
 * через каждую архитектуру в режиме без логирования для максимальной 
 * производительности. В конце выводит Топ-3 самых эффективных связок.
 * 
 * @tparam Key Тип данных ключа.
 * @param[in] data Вектор запросов (трасса обращений, желательно сгенерированная по закону Ципфа).
 * @param[in] capacity Общий лимит физической памяти, выделяемый на каскад.
 */
template <typename Key>
void find_best_configuration(const std::vector<Key>& data, size_t capacity) 
{
    std::vector<std::string> available = {"LRU", "LFU", "2Q", "ARC", "LIRS"};
    std::vector<BenchResult> results;

    std::cout << "Запуск масштабного тестирования конфигураций...\n";

    // 1. Тестируем все одиночные кэши
    for (const auto& algo : available) {
        size_t hits = run_silent_benchmark<Key>(capacity, {algo}, data);
        results.push_back({{algo}, hits});
        std::cout << "[" << algo << "] -> Хиты: " << hits << "\n";
    }

    // 2. Тестируем все двойные системы
    for (const auto& algo1 : available) {
        for (const auto& algo2 : available) {
            
            if (algo1 == algo2) continue; 
            
            size_t hits = run_silent_benchmark<Key>(capacity, {algo1, algo2}, data);
            results.push_back({{algo1, algo2}, hits});
            std::cout << "[" << algo1 << " -> " << algo2 << "] -> Хиты: " << hits << "\n";
        }
    }

    std::sort(results.begin(), results.end(), [](const BenchResult& a, const BenchResult& b) {
        return a.hits > b.hits; 
    });

    std::cout << "\n=== ТОП-3 КОНФИГУРАЦИИ ===\n";
    for (size_t i = 0; i < 3 && i < results.size(); ++i) {
        std::cout << i + 1 << ". [";
        for (size_t j = 0; j < results[i].algos.size(); ++j) {
            std::cout << results[i].algos[j] << (j < results[i].algos.size() - 1 ? " -> " : "");
        }
        std::cout << "] : " << results[i].hits << " хитов\n";
    }
}