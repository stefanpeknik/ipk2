CXX = g++
CXXFLAGS = -std=c++20

SRCS = ipkcpd.cpp
OBJS = $(SRCS:.cpp=.o)

TARGET = ipkcpd

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

runtcp: all
	./$(TARGET) -h 0.0.0.0 -p 2023 -m tcp

runudp: all
	./$(TARGET) -h 0.0.0.0 -p 2023 -m udp

clean:
	$(RM) $(OBJS) $(TARGET)
