SHELL := /bin/bash
CC = gcc
TARGET = project2
CFLAGS = -Wall -Wextra -ggdb -O0
OBJS = main.o createSymbolTable.o checker.o freeSymbolTable.o objcodeCreate.o stringSplit.o createSicObjectFile.o createXeObjectFile.o freeObjectFile.o fileBuffer.o recordTable.o
all: $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) -o $@ $(OBJS)
%.o: src/%.c
	$(CC) $(CFLAGS) -c $<
clean:
	@rm -f $(OBJS) $(TARGET)
run:
	./$(TARGET)
