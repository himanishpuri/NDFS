CXX = g++
CXXFLAGS = -std=c++20 -Wall
TARGET = ft

SRC = ft.cpp

all: $(TARGET)
	./$(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) /usr/local/lib/libredis++.a /usr/local/lib/libhiredis.a -pthread

clean:
	rm -f $(TARGET)