#include <iostream>
#include <string>
#include <vector>

#include "LRU_cache.hpp"
#include "LFU_cache.hpp"
#include "2Q_cache.hpp"
#include "ARC_cache.hpp"
#include "LIRS_cache.hpp"
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
        1, 3, 1,   
        2, 4, 2,   
        5, 6, 7, 8 
    });
    Log::trace(Log::INFO, "\n");
}

void test_ARC ()
{
    Log::trace(Log::INFO, "----   Тест для ARC-кэша   ----\n");
    
    Log::trace(Log::INFO, "----        Тест 1        ----\n");
    Log::trace(Log::INFO, "Базовое добавление и повышение до VIP (Хит в MRU)\n");
    TEST<ARC_Cache<int>> (5, std::vector<int>{1, 1});
    Log::trace(Log::INFO, "\n");

    Log::trace(Log::INFO, "----        Тест 2        ----\n");
    Log::trace(Log::INFO, "Магия ARC: Хит в MRU_ghost (Увеличение квоты p)\n");
    TEST<ARC_Cache<int>> (5, std::vector<int>{1, 2, 3, 4, 5, 6, 1});
    Log::trace(Log::INFO, "\n");

    Log::trace(Log::INFO, "----        Тест 3        ----\n");
    Log::trace(Log::INFO, "Закрепление в VIP-зоне (Выживание при сканировании)\n");
    TEST<ARC_Cache<int>> (5, std::vector<int>{1, 1, 2, 3, 4, 5, 6, 1});
    Log::trace(Log::INFO, "\n");

    Log::trace(Log::INFO, "----        Тест 4        ----\n");
    Log::trace(Log::INFO, "Обратная адаптация: Хит в MFU_ghost (Уменьшение квоты p)\n");
    TEST<ARC_Cache<int>> (5, std::vector<int>{1, 1, 2, 2, 3, 4, 5, 6, 7, 8, 9, 1});
    Log::trace(Log::INFO, "\n");

    Log::trace(Log::INFO, "----        Тест 5        ----\n");
    Log::trace(Log::INFO, "Проверка лимитов истории (Очистка старых призраков)\n");
    TEST<ARC_Cache<int>> (5, std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12});
    Log::trace(Log::INFO, "\n");
}

void test_LIRS ()
{
    Log::trace(Log::INFO, "----   Тест для LIRS-кэша   ----\n");
    
    Log::trace(Log::INFO, "----        Тест 1        ----\n");
    Log::trace(Log::INFO, "Заполнение VIP-зоны (LIR) и повторный хит\n");
    TEST<LIRS_Cache<int>> (4, std::vector<int>{1, 2, 3, 2});
    Log::trace(Log::INFO, "\n");

    Log::trace(Log::INFO, "----        Тест 2        ----\n");
    Log::trace(Log::INFO, "Появление прохожих и призраков (Вытеснение из Очереди Q)\n");
    TEST<LIRS_Cache<int>> (4, std::vector<int>{1, 2, 3, 4, 5});
    Log::trace(Log::INFO, "\n");

    Log::trace(Log::INFO, "----        Тест 3        ----\n");
    Log::trace(Log::INFO, "Успешное возвращение прохожего: Хит по HIR_RESIDENT\n");
    TEST<LIRS_Cache<int>> (4, std::vector<int>{1, 2, 3, 4, 4});
    Log::trace(Log::INFO, "\n");

    Log::trace(Log::INFO, "----        Тест 4        ----\n");
    Log::trace(Log::INFO, "Эффект домино: Хит по призраку (HIR_NO_RESIDENT)\n");
    TEST<LIRS_Cache<int>> (4, std::vector<int>{1, 2, 3, 4, 5, 4});
    Log::trace(Log::INFO, "\n");

    Log::trace(Log::INFO, "----        Тест 5        ----\n");
    Log::trace(Log::INFO, "Работа дворника (prune): Очистка мусора при свержении дна\n");
    TEST<LIRS_Cache<int>> (4, std::vector<int>{1, 2, 3, 4, 1, 5, 6, 4});
    Log::trace(Log::INFO, "\n");
}