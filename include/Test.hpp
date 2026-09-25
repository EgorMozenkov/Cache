#pragma once

#include <iostream>
#include <vector>
#include "Logger.hpp"

/// @name Интеграционные тесты для целочисленных ключей (int)
/// @{
void test_LRU ();
void test_LFU ();
void test_TwoQ ();
void test_ARC ();
void test_LIRS ();  

void test_ideal_cache ();
/// @}

/// @name Интеграционные тесты для строковых ключей (std::string)
/// @{
void string_test_LRU ();
void string_test_LFU ();
void string_test_TwoQ ();
void string_test_ARC ();
void string_test_LIRS ();
/// @}