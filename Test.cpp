#include <vector>

#include "LRU_cache.hpp"
#include "LFU_cache.hpp"
#include "Test.hpp"
#include "Logger.hpp"

// хочется сделать функцию типа: test(кол-во мест в кэше, список значнеий вводимых с клавиатуры)

void test_LRU ()
{
    Log::trace(Log::INFO, "----   Тест для LRU-кэша   ----\n");
    Log::trace(Log::INFO, "----        Тест 1         ----\n");
    TEST<LRU_Cache> (2, {1, 2, 3, 2});
    
    Log::trace(Log::INFO, "----        Тест 2         ----\n");
    TEST<LRU_Cache> (3, {5, 5, 5, 5, 5});

    Log::trace(Log::INFO, "----        Тест 3         ----\n");
    TEST<LRU_Cache> (2, {1, 2, 1, 2, 1, 2});

    Log::trace(Log::INFO, "----        Тест 4         ----\n");
    TEST<LRU_Cache> (3, {1, 2, 3, 4, 5, 6});

    Log::trace(Log::INFO, "----        Тест 5         ----\n");
    TEST<LRU_Cache> (1, {1, 2, 1, 3});
}


void test_LFU ()
{
    Log::trace(Log::INFO, "----   Тест для LFU-кэша   ----\n");
    Log::trace(Log::INFO, "----        Тест 1         ----\n");
    TEST<LFU_Cache> (3, {1, 2, 3});

    Log::trace(Log::INFO, "----        Тест 2         ----\n");
    TEST<LFU_Cache> (3, {1, 2, 3, 1, 1, 2});

    Log::trace(Log::INFO, "----        Тест 3         ----\n");
    TEST<LFU_Cache> (3, {1, 2, 3, 1, 1, 2, 4});

    Log::trace(Log::INFO, "----        Тест 4         ----\n");
    TEST<LFU_Cache> (3, {1, 2, 3, 1, 1, 2, 4, 4, 5});
}