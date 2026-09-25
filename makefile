CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3 -Iinclude

TARGET = cache_test

SRCS = src/Main.cpp src/Logger.cpp tests/Test.cpp tests/Test_string.cpp

OBJS = $(patsubst src/%.cpp, build/%.o, $(patsubst tests/%.cpp, build/%.o, $(SRCS)))


all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "Сборка успешно завершена! Исполняемый файл: $(TARGET)"

build/%.o: src/%.cpp | build
	$(CXX) $(CXXFLAGS) -c $< -o $@

build/%.o: tests/%.cpp | build
	$(CXX) $(CXXFLAGS) -c $< -o $@

build:
	mkdir -p build

clean:
	rm -rf build $(TARGET)
	@echo "Очистка завершена."