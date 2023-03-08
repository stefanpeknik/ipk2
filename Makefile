CXX = g++
CXXFLAGS = -Wall -Wextra -pedantic -std=c++20

SRCS = ipkcpd.cpp
OBJS = $(SRCS:.cpp=.o)

TARGET = ipkcpd

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS) $(TARGET)
