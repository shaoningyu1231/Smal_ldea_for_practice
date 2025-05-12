# Compiler
CXX = g++
CXXFLAGS = -Wall -g -pthread

# Source files
SRCS = ThreadPool.cpp

# Target files
OBJS = $(SRCS:.cpp=.o)

# Execute files
TARGET = threadpool_demo

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET)