WARNINGS = -Wpedantic -pedantic-errors
WARNINGS += -Werror
WARNINGS += -Wall
WARNINGS += -Wextra
WARNINGS += -Wold-style-definition
WARNINGS += -Wcast-align
WARNINGS += -Wformat=2
WARNINGS += -Wlogical-op
WARNINGS += -Wmissing-declarations
WARNINGS += -Wmissing-include-dirs
WARNINGS += -Wmissing-prototypes
WARNINGS += -Wnested-externs
WARNINGS += -Wpointer-arith
WARNINGS += -Wredundant-decls
WARNINGS += -Wsequence-point
WARNINGS += -Wshadow
WARNINGS += -Wstrict-prototypes
WARNINGS += -Wswitch
WARNINGS += -Wundef
WARNINGS += -Wunreachable-code
WARNINGS += -Wunused-but-set-parameter
WARNINGS += -Wwrite-strings
WARNINGS += -Wdisabled-optimization
WARNINGS += -Wunsafe-loop-optimizations
WARNINGS += -Wfree-nonheap-object


CC = gcc
CFLAGS = -lraylib -lglfw -lGL -lopenal -lm -pthread -ldl -llua5.4 -std=c99
SOURCES = ./src/*.c


all: tuxmino debug clean

tuxmino:
	#$(CC) -g $(SOURCES) $(CFLAGS) $(WARNINGS) -o tuxmino
	$(CC) -g $(SOURCES) $(CFLAGS) -o tuxmino

run:
	./tuxmino

debug:
	gdb -x gdbinit tuxmino

clean:
	rm tuxmino
