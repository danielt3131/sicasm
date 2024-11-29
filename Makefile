SHELL := /bin/bash
CC = gcc
TARGET = project2
CFLAGS = -Wall -Wextra -g -O0 -fsanitize=address
LDFLAGS = -fsanitize=address
OBJS = main.o createSymbolTable.o checker.o freeSymbolTable.o objcodeCreate.o stringSplit.o createSicObjectFile.o createXeObjectFile.o freeObjectFile.o fileBuffer.o recordTable.o
all: $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)
%.o: src/%.c
	$(CC) $(CFLAGS) -c $<
clean:
	@rm -f $(OBJS) $(TARGET)
run:
	./$(TARGET)
