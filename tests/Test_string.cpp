#include <iostream>
#include <string>

#include "LRU_cache.hpp"
#include "LFU_cache.hpp"
#include "2Q_cache.hpp"
#include "ARC_cache.hpp"
#include "LIRS_cache.hpp"
#include "Test.hpp"
#include "Test_engine.hpp"
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

void string_test_TwoQ ()
{
    std::vector<std::string> requests = {
        "Radio Circuits", "Physics Lab", "Computer Technologies", "Radio Circuits", 
        "Digital Production Practicum", "Physics Lab", "Computer Technologies", "C++"
    };

    Log::trace(Log::INFO, "----   Тест для 2Q-кэша   ----\n");
    TEST<TwoQ_Cache<std::string>> (4, requests);
    Log::trace(Log::INFO, "\n");
}

void string_test_ARC ()
{
    std::vector<std::string> requests = {
        "Computer Technologies", "Digital Production Practicum", "Radio Circuits", "C++", "Physics Lab",
        "Computer Technologies", "Physical Education", "Digital Production Practicum"
    };

    Log::trace(Log::INFO, "----   Тест для ARC-кэша   ----\n");
    TEST<ARC_Cache<std::string>> (5, requests);
    Log::trace(Log::INFO, "\n");
}

void string_test_LIRS ()
{
    std::vector<std::string> requests = {
        "Physics Lab", "Radio Circuits", "Digital Production Practicum", 
        "Computer Technologies", "Physical Education", "Computer Technologies"
    };

    Log::trace(Log::INFO, "----   Тест для LIRS-кэша   ----\n");
    TEST<LIRS_Cache<std::string>> (4, requests);
    Log::trace(Log::INFO, "\n");
}
