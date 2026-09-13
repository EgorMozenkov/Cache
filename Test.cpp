#include <iostream>
#include <string>
#include <vector>

#include "LRU_cache.hpp"
#include "LFU_cache.hpp"
#include "2Q_cache.hpp"
#include "Test.hpp"
#include "Logger.hpp"


void string_test_LRU ()
{
    std::vector<std::string> requests = {
        "deepslate", "spruce_log", "calcite", "deepslate", 
        "dark_oak", "calcite", "nether_brick", "spruce_log"
    };

    Log::trace(Log::INFO, "----   Тест для LRU-кэша   ----\n");
    TEST<LRU_Cache<std::string>> (3, requests);
    Log::trace(Log::INFO, "\n");
}

void string_test_LFU ()
{
    std::vector<std::string> requests = {
        "deepslate", "spruce_log", "calcite", "deepslate", 
        "dark_oak", "calcite", "nether_brick", "spruce_log"
    };

    Log::trace(Log::INFO, "----   Тест со строками для LFU-кэша   ----\n");
    TEST<LFU_Cache<std::string>> (3, requests);
    Log::trace(Log::INFO, "\n");
}

void test_LRU ()
{
    Log::trace(Log::INFO, "----   Тест для LRU-кэша   ----\n");
    Log::trace(Log::INFO, "----        Тест 1         ----\n");
    TEST<LRU_Cache<int>> (2, std::vector<int>{1, 2, 3, 2});
    Log::trace(Log::INFO, "\n");

    Log::trace(Log::INFO, "----        Тест 2         ----\n");
    TEST<LRU_Cache<int>> (3, std::vector<int>{5, 5, 5, 5, 5});
    Log::trace(Log::INFO, "\n");

    Log::trace(Log::INFO, "----        Тест 3         ----\n");
    TEST<LRU_Cache<int>> (2, std::vector<int>{1, 2, 1, 2, 1, 2});
    Log::trace(Log::INFO, "\n");

    Log::trace(Log::INFO, "----        Тест 4         ----\n");
    TEST<LRU_Cache<int>> (3, std::vector<int>{1, 2, 3, 4, 5, 6});
    Log::trace(Log::INFO, "\n");

    Log::trace(Log::INFO, "----        Тест 5         ----\n");
    TEST<LRU_Cache<int>> (1, std::vector<int>{1, 2, 1, 3});
    Log::trace(Log::INFO, "\n");
}


void test_LFU ()
{
    Log::trace(Log::INFO, "----   Тест для LFU-кэша   ----\n");
    Log::trace(Log::INFO, "----        Тест 1         ----\n");
    TEST<LFU_Cache<int>> (3, std::vector<int>{1, 2, 3});
    Log::trace(Log::INFO, "\n");

    Log::trace(Log::INFO, "----        Тест 2         ----\n");
    TEST<LFU_Cache<int>> (3, std::vector<int>{1, 2, 3, 1, 1, 2});
    Log::trace(Log::INFO, "\n");

    Log::trace(Log::INFO, "----        Тест 3         ----\n");
    TEST<LFU_Cache<int>> (3, std::vector<int>{1, 2, 3, 1, 1, 2, 4});
    Log::trace(Log::INFO, "\n");

    Log::trace(Log::INFO, "----        Тест 4         ----\n");
    TEST<LFU_Cache<int>> (3, std::vector<int>{1, 2, 3, 1, 1, 2, 4, 4, 5});
    Log::trace(Log::INFO, "\n");
}

void test_TwoQ ()
{
    Log::trace(Log::INFO, "----   Тест для 2Q-кэша   ----\n");
    
    Log::trace(Log::INFO, "----        Тест 1        ----\n");
    Log::trace(Log::INFO, "Базовое добавление и хит в IN\n");
    TEST<TwoQ_Cache<int>> (4, std::vector<int>{1, 1});
    Log::trace(Log::INFO, "\n");

    Log::trace(Log::INFO, "----        Тест 2        ----\n");
    Log::trace(Log::INFO, "Вытеснение в OUT и повышение до VIP\n");
    TEST<TwoQ_Cache<int>> (4, std::vector<int>{1, 2, 1});
    Log::trace(Log::INFO, "\n");

    Log::trace(Log::INFO, "----        Тест 3        ----\n");
    Log::trace(Log::INFO, "Полное вытеснение из OUT (забывание)\n");
    TEST<TwoQ_Cache<int>> (4, std::vector<int>{1, 2, 3, 4, 1});
    Log::trace(Log::INFO, "\n");

    Log::trace(Log::INFO, "----        Тест 4        ----\n");
    Log::trace(Log::INFO, "Устойчивость к сканированию (Cache Pollution)\n");
    TEST<TwoQ_Cache<int>> (4, std::vector<int>{
        1, 3, 1,   // 1 уходит в VIP
        2, 4, 2,   // 2 уходит в VIP
        5, 6, 7, 8 // Мусорный трафик. VIP не должен пострадать!
    });
    Log::trace(Log::INFO, "\n");
}