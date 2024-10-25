SHELL := /bin/bash
CC = gcc
TARGET = project2
CFLAGS = -Wall -Wextra -g -O0 -fstack-protector-all
OBJS = main.o createSymbolTable.o checker.o freeSymbolTable.o objcodeCreate.o stringSplit.o createObjectFile.o freeObjectFile.o
all: $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) -o $@ $(OBJS)
%.o: src/%.c
	$(CC) $(CFLAGS) -c $<
clean:
	@rm -f $(OBJS) $(TARGET)
run:
	./$(TARGET)