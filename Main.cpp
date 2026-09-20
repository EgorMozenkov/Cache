#include <iostream>
#include <string>
#include <vector>
#include <chrono>

#include "Logger.hpp"
#include "Test.hpp" 
#include "Test_multi_level_cache.hpp" 

int main(int argc, char* argv[]) 
{
    // Если запустили просто ./app без аргументов
    if (argc < 2) {
        std::cerr << "Использование:\n";
        std::cerr << "  ./app --file <имя_файла>\n";
        std::cerr << "  ./app --test-lru\n";
        std::cerr << "  ./app --test-multi\n";
        return 1; // Завершаем программу с ошибкой
    }

    std::string mode = argv[1];

    if (mode == "--file") {
        if (argc < 3) {
            std::cerr << "Ошибка: Вы не указали имя файла! Пример: ./app --file test1.txt\n";
            return 1;
        }
        std::string filename = argv[2];
        Log::trace(Log::ERROR, "=== ЗАПУСК ИЗ ФАЙЛА: ", filename, " ===\n");
        
        auto start_time = std::chrono::high_resolution_clock::now();

        file_test<int>(filename);
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        Log::trace(Log::ERROR, "Время работы теста (чтение + симуляция): ", duration.count(), " мс\n");
    }
    else if (mode == "--test-lru") {
        Log::trace(Log::INFO, "=== ЗАПУСК ТЕСТА ОДИНОЧНОГО КЭША (LRU) ===\n");
        TEST<LRU_Cache<int>, int>(4, std::vector<int>{1, 2, 3, 1, 2, 4, 5, 6, 4, 5});
    } 
    else if (mode == "--test-multi") {
        Log::trace(Log::INFO, "=== ЗАПУСК ТЕСТА МНОГОУРОВНЕВОГО КЭША ===\n");
        test_multi_level_cache<int>(4, {"LRU", "2Q"}, {1, 2, 3, 1, 2, 4, 5, 6, 4, 5});
    } 
    else {
        std::cerr << "Ошибка: Неизвестная команда '" << mode << "'\n";
        return 1;
    }

    return 0;
}