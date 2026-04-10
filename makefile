# Paths
DJGPP_PREFIX = i586-pc-msdosdjgpp-
#CXX = $(DJGPP_PREFIX)g++
CC = $(DJGPP_PREFIX)gcc
STRIP = $(DJGPP_PREFIX)strip

# 486DX Optimized Flags
# -fno-exceptions and -fno-rtti are essential for performance/size on 486
CFLAGS = -O3 -fomit-frame-pointer -finline-functions -march=i386 -Wall  
LDFLAGS = -lemu

TARGET = game.exe
SOURCES = src/hello.c src/graphics.c src/small_font.c src/keyboard.c src/road.c
OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	$(STRIP) $@
	@echo "Build successful: $(TARGET)"

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(OBJECTS) $(TARGET) GAME.EXE
