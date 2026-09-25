# Multi-Level Cache

Проект реализует многоуровневую систему кэширования и сравнивает эффективность различных алгоритмов вытеснения.

## Реализованные алгоритмы
* **LRU** (Least Recently Used)
* **LFU** (Least Frequently Used)
* **2Q** (Two Queues)
* **ARC** (Adaptive Replacement Cache)
* **LIRS** (Low Inter-reference Recurrence Set)

## Системные требования
* Компилятор с поддержкой стандарта **C++17** (GCC / Clang)
* Среда выполнения: **Linux** (Ubuntu / WSL2)
* **Python 3.x** (для работы скрипта генерации тестовых данных)