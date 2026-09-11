CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
TARGET = cache_test

all:
	$(CXX) $(CXXFLAGS) Main.cpp Logger.cpp Test.cpp LRU_cache.cpp -o $(TARGET)

clean:
	rm -f *.o $(TARGET)