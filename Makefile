ifeq ($(OS), Windows_NT)
	DIRSEP := \\
	RM     := del /Q
else
	DIRSEP := /
	RM     := rm -f
endif
#$(error Unknown host OS)

BINDIR = bin
SRCDIR = src
HEADERSDIR = headers
TARGET = $(BINDIR)$(DIRSEP)hangman_cli
LIBS =
CC = gcc
CFLAGS = -Wall


.PHONY: default
default: $(TARGET)

.PHONY: all
all: default

.PHONY: debug
debug: DEBUG = -g
debug: all

.PHONY: clean
clean:
	$(RM) $(BINDIR)$(DIRSEP)*


SOURCES = $(wildcard $(SRCDIR)$(DIRSEP)*.c)
OBJECTS = $(patsubst $(SRCDIR)$(DIRSEP)%.c, $(BINDIR)$(DIRSEP)%.o, $(SOURCES))
HEADERS = $(wildcard $(HEADERSDIR)$(DIRSEP)*.h)

$(BINDIR)$(DIRSEP)%.o: $(SRCDIR)$(DIRSEP)%.c $(HEADERS)
	$(CC) $(CFLAGS) $(DEBUG) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(DEBUG) $(OBJECTS) -o $@
