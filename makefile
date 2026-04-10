# Paths
DJGPP_PREFIX = i586-pc-msdosdjgpp-
CXX = $(DJGPP_PREFIX)g++
STRIP = $(DJGPP_PREFIX)strip

# 486DX Optimized Flags
# -fno-exceptions and -fno-rtti are essential for performance/size on 486
CXXFLAGS = -O2 -march=i486 -Wall -fno-exceptions -fno-rtti
LDFLAGS = 

TARGET = game.exe
SOURCES = src/hello.cpp src/graphics.cpp src/small_font.cpp src/keyboard.cpp
OBJECTS = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	$(STRIP) $@
	@echo "Build successful: $(TARGET)"

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *.o $(OBJECTS) $(TARGET) GAME.EXE
