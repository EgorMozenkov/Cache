#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include "Ideal_cache.hpp"
#include "Logger.hpp"

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
 * Выполняет полный перебор всех доступных систем состоящх до 5 уровней. 
 * Прогоняет входные запросы через каждую архитектуру в режиме без логирования для максимальной 
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

    Log::trace(Log::INFO, "Запуск масштабного тестирования конфигураций...\n");

    for (size_t depth = 1; depth <= 5; ++depth) {
        build_chain(depth, {}, available, data, results, capacity);
    }

    std::sort(results.begin(), results.end(), [](const BenchResult& a, const BenchResult& b) {
        return a.hits > b.hits; 
    });


    // Расчёт идеального кэша
    Log::trace(Log::INFO, "Рассчет предела Белади (Идеальный кэш)\n");
    Ideal_Cache<Key> ideal(capacity, data);
    size_t ideal_hits = 0;
    for (const Key& key : data) {
        if (ideal.request(key).hit) {
            ideal_hits++;
        }
    }
    Log::trace(Log::INFO, "Количество хитов у идеального кэша: ", ideal_hits, "\n");

    Log::trace(Log::INFO, "\n=== ТОП-3 КОНФИГУРАЦИИ ===\n");
    for (size_t i = 0; i < 3 && i < results.size(); ++i) {
        Log::trace(Log::INFO, i + 1, ". [");
        for (size_t j = 0; j < results[i].algos.size(); ++j) {
            Log::trace(Log::INFO, results[i].algos[j], (j < results[i].algos.size() - 1 ? " -> " : ""));
        }
        Log::trace(Log::INFO, "] : ", results[i].hits, " хитов\n");
    }
}


/**
 * @brief Рекурсивный генератор конфигураций многоуровневого кэша.
 * 
 * Формирует все возможные комбинации многоуровневых систем 
 * заданной длины, используя доступные алгоритмы. Гарантирует уникальность 
 * алгоритмов в рамках одной системы (один и тот же алгоритм не может 
 * встречаться дважды). При достижении целевой глубины автоматически запускает 
 * бенчмарк и сохраняет результат.
 * 
 * @tparam Key Тип данных ключа.
 * @param[in] target_depth Целевая глубина каскада (количество уровней).
 * @param[in] current_chain Частично собранная цепочка алгоритмов на текущем шаге рекурсии.
 * @param[in] available_algos Список всех доступных алгоритмов для перебора.
 * @param[in] data Вектор обращений для проведения теста.
 * @param[out] results Ссылка на вектор, в который записываются результаты (цепочка + хиты).
 * @param[in] capacity Суммарная физическая вместимость каскада кэшей.
 */
template <typename Key>
void build_chain(
    size_t target_depth,                            
    std::vector<std::string> current_chain,         
    const std::vector<std::string>& available_algos, 
    const std::vector<Key>& data,
    std::vector<BenchResult>& results,
    size_t capacity) 
{
    if (current_chain.size() == target_depth) {
        // Запускаем бенчмарк для собранной цепочки
        size_t hits = run_silent_benchmark(capacity, current_chain, data);
        
        // Добавляем результат в общий массив
        results.push_back({current_chain, hits});
        std::cout << "[";
        for (size_t i = 0; i < current_chain.size(); ++i) {
            Log::trace(Log::INFO, current_chain[i], (i < current_chain.size() - 1 ? " -> " : ""));
        }
        Log::trace(Log::INFO, "] -> Хиты: ", hits, "\n");
        return; 
    }

    // Рекурсивный шаг
    for (const std::string& algo : available_algos) {
        
        if (std::find(current_chain.begin(), current_chain.end(), algo) != current_chain.end()) {
            continue;
        }

        std::vector<std::string> next_chain = current_chain;
        next_chain.push_back(algo);

        build_chain(target_depth, next_chain, available_algos, data, results, capacity);
    }
}