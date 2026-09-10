#include <iostream>
#include <list>
#include <unordered_map>
#include <string>

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
    int capacity;

    public:

    LRU_Cache (int N) {
        capacity = N;
    }

    bool request (int key) override {
        auto it = LRU_map.find(key);

        if (it != LRU_map.end()) {
            LRU_list.splice (LRU_list.begin(), LRU_list, it->second);

            return 1;
        }

        if (LRU_list.size() == capacity) {
            int last_value = LRU_list.back();
            LRU_map.erase(last_value);
            LRU_list.pop_back();
        }
        LRU_list.push_front(key);
        LRU_map[key] = LRU_list.begin();

        return 0;
    }
};