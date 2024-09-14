SHELL := /bin/bash
CC = gcc
TARGET = project1
CFLAGS = -Wall -Wextra -g -O0
OBJS = p1.o createSymbolTable.o checker.o freeSymbolTable.o
all: $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) -o $@ $(OBJS)
%.o: src/%.c
	$(CC) $(CFLAGS) -c $<
clean:
	@rm -f $(OBJS) $(TARGET)
run:
	./$(TARGET)