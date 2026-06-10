CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
TARGET = coord
SRC = coord.cpp

all: run

$(TARGET): $(SRC)
	@$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	@./$(TARGET)
	@$(MAKE) --no-print-directory clean

clean:
	@rm -f $(TARGET)
