#include <iostream>
#include <list>
#include <unordered_map>
#include <string>

#include "Logger.hpp"

typedef std::list<int>::iterator IT;

class ICache {
    public:

    virtual ~ICache () = default;

    virtual bool request (int key) = 0;
};

class LRU_Cache : public ICache {
    private:

    std::list <int> LRU_list;
    std::unordered_map <int, IT> LRU_map;
    size_t capacity;

    public:

    LRU_Cache (size_t N) {
        capacity = N;
    }

    bool request (int key) override {
        Log::trace(Log::DEBUG, "Запрошен ключ: %d\n", key);
        auto it = LRU_map.find(key);

        if (it != LRU_map.end()) {

            Log::trace(Log::INFO, "ХИТ: Ключ %d найден в кэше, переносим в начало\n", key);
            LRU_list.splice (LRU_list.begin(), LRU_list, it->second);

            return 1;
        }

        if (LRU_list.size() == capacity) {

            int last_value = LRU_list.back();

            Log::trace(Log::TRACE, "Вытеснение: Кэш переполнен. Удаляем старейший ключ: %d\n", last_value);

            LRU_map.erase(last_value);
            LRU_list.pop_back();
        }

        Log::trace(Log::INFO, "МИСС: Ключ %d добавлен в кэш\n", key);

        LRU_list.push_front(key);
        LRU_map[key] = LRU_list.begin();

        return 0;
    }
};

int main()
{
    ICache* cache_1 = new LRU_Cache (2);

    std::cout << cache_1->request(1) << "\n";
    std::cout << cache_1->request(2) << "\n";
    std::cout << cache_1->request(2) << "\n";

    delete cache_1;
    return 0;
}