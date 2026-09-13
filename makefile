CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
TARGET = cache_test

all:
	g++ -Wall -Wextra -std=c++17 Main.cpp Logger.cpp Test.cpp LRU_cache.cpp LFU_cache.cpp -o cache_test
clean:
	rm -f *.o $(TARGET)