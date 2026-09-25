#include <string>
#include <vector>

#include "LRU_cache.hpp"
#include "LFU_cache.hpp"
#include "2Q_cache.hpp"
#include "ARC_cache.hpp"
#include "LIRS_cache.hpp"
#include "Test.hpp"
#include "Logger.hpp"
#include "File_request.hpp"
#include "Multi_Level_cache.hpp"


/**
 * @brief Прогоняет входные данные через систему кэшей с детальным логированием.
 * 
 * Выполняет запросы к кэшу в цикле, собирает статистику попаданий (hits) 
 * и выводит результаты через глобальный логгер согласно текущему уровню детализации.
 * 
 * @tparam Key Тип данных ключа.
 * @param[in,out] cache Готовый объект многоуровневого кэша.
 * @param[in] conf Конфигурация каскада (для вывода информации).
 * @param[in] data Тестовые данные (включая вектор запросов).
 */
template <typename Key> 
void data_output(Multi_Level_Cache<Key>& cache, const Config& conf, const InputData<Key>& data)
{
    size_t HITS = 0;
    size_t total_requests = 0;

    // Берем данные из структуры data
    Log::trace(Log::ERROR, "Создана многоуровневая система. Размер: ", data.Cache_size, ", уровни: ", conf.levels, "\n");
    Log::trace(Log::INFO, "--- Исходное состояние кэша ---\n");
    //cache.read_cache();
    Log::trace(Log::DEBUG, "Приходят новые ключи!\n");

    // Итерируемся по вектору из структуры data
    for(const Key& key : data.keys) {
        Log::trace(Log::INFO, "Запрошен ключ: ", key, "\n");
        
        if(cache.request(key).hit) {
            Log::trace(Log::INFO, "-> ХИТ!\n");
            HITS++;
        } else {
            Log::trace(Log::INFO, "-> МИСС!\n");
        }
        
        //cache.read_cache();
        total_requests++;
    }

    Log::trace(Log::INFO, "--- Cостояние кэша после теста ---\n");
    //cache.read_cache();
    Log::trace(Log::ERROR, "Количество запросов: ", total_requests, ", Количество хитов: ", HITS, "\n\n");
}


/**
 * @brief Программный запуск тестирования многоуровневого кэша.
 * 
 * Формирует конфигурацию и данные на лету, минуя чтение из файла.
 * Удобен для написания unit-тестов прямо в коде.
 * 
 * @tparam Key Тип данных ключа.
 * @param capacity Суммарная вместимость кэша.
 * @param algos Вектор с названиями алгоритмов для каждого уровня.
 * @param keys Вектор запросов к кэшу.
 */
template <typename Key> 
void test_multi_level_cache(size_t capacity, const std::vector<std::string>& algos, const std::vector<Key>& keys) 
{
    Config conf;
    conf.algorithms = algos;
    conf.levels = algos.size();

    InputData<Key> data;
    data.Cache_size = capacity;
    data.keys = keys;
    data.num_requests = keys.size();

    Multi_Level_Cache<Key> cache(conf, data);

    data_output(cache, conf, data);
}


/**
 * @brief Файловый запуск тестирования.
 * 
 * Читает настройки и запросы из указанного текстового файла, 
 * собирает кэши и запускает процесс логирования.
 * 
 * @tparam Key Тип данных ключа.
 * @param filename Путь к файлу с конфигурацией и данными.
 */
template <typename Key> 
void file_test(const std::string& filename) 
{
    Config conf;
    InputData<Key> data;

    read_file(filename, conf, data);

    if (data.num_requests == 0) {
        return; 
    }

    Multi_Level_Cache<Key> cache(conf, data);

    data_output(cache, conf, data);
}


/**
 * @brief Бенчмарк для подбора лучшей конфигурации.
 * 
 * В отличие от других функций тестирования, полностью отключает вызовы 
 * логгера. Это позволяет обрабатывать миллионы
 * для поиска алгоритма с максимальным hit rate.
 * 
 * @tparam Key Тип данных ключа.
 * @param capacity Суммарная вместимость кэша.
 * @param algos Вектор с названиями алгоритмов для каждого уровня.
 * @param keys Вектор запросов к кэшу (рекомендуется передавать распределение Ципфа).
 * @return size_t Итоговое количество попаданий (хитов).
 */
template <typename Key>
size_t run_silent_benchmark(size_t capacity, const std::vector<std::string>& algos, const std::vector<Key>& keys) 
{
    Config conf;
    conf.algorithms = algos;
    conf.levels = algos.size();

    InputData<Key> data;
    data.Cache_size = capacity;
    data.keys = keys;
    data.num_requests = keys.size();

    Multi_Level_Cache<Key> cache(conf, data);

    size_t HITS = 0;
    for(const Key& key : data.keys) {
        if(cache.request(key).hit) {
            HITS++;
        }
    }
    return HITS;
}