CXX = g++
CXXFLAGS = -std=c++20 -Wall
LINKFLAGS = /usr/local/lib/libsodium.a /usr/local/lib/libredis++.a /usr/local/lib/libhiredis.a
TARGET = main

SRC = main.cpp

all: $(TARGET)
	./$(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LINKFLAGS) -pthread

clean:
	rm -f $(TARGET)
