CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
TARGET_COORD = coordinate_conversion
SRC_COORD = coord.cpp
TARGET_DIMENSION = pixel_map
SRC_DIMENSION = pixel_map.cpp

$(TARGET_COORD): $(SRC_COORD)
	@$(CXX) $(CXXFLAGS) $(SRC_COORD) -o $(TARGET_COORD)

$(TARGET_DIMENSION): $(SRC_DIMENSION)
	@$(CXX) $(CXXFLAGS) $(SRC_DIMENSION) -o $(TARGET_DIMENSION)

coord: $(TARGET_COORD)
	@./$(TARGET_COORD)
	@$(MAKE) --no-print-directory clean

map: $(TARGET_DIMENSION)
	@./$(TARGET_DIMENSION)
	@$(MAKE) --no-print-directory clean

clean:
	@rm -f $(TARGET_COORD) $(TARGET_DIMENSION)
