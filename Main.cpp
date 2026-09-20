#include "Test.hpp"
#include "Test_multi_level_cache.hpp"


int main()
{
    test_multi_level_cache<int>(
        4,                              
        {"LRU", "LFU"},                  // Алгоритмы (уровни)
        {1, 2, 3, 1, 2, 4, 5, 6, 4, 5}  // Ключи
    );

    Log::trace(Log::INFO, "----   Тест для LRU-кэша   ----\n");
    Log::trace(Log::INFO, "----        Тест 1         ----\n");
    TEST<LRU_Cache<int>> (4, std::vector<int>{1, 2, 3, 1, 2, 4, 5, 6, 4, 5});
    Log::trace(Log::INFO, "\n");









    //string_test_TwoQ();
    //string_test_ARC();
    //string_test_LIRS();
    //test_LIRS();
    //test_ARC ();
    //test_TwoQ ();
    //test_LRU ();
    //test_LFU ();
    //string_test_LRU ();
    //string_test_LFU ();
    return 0;
}