CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
LDFLAGS = -lzip
TARGET = zippy
SRCS = zippy.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean