# Makefile for Phonebook Management System

CC = gcc
CFLAGS = -Wall -Wextra -O2 -fexec-charset=GBK
TARGET = phonebook
SRC = phonebook.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	./$(TARGET).exe

clean:
	del $(TARGET).exe
	@echo 清理完成

cleanall: clean
	del phonebook.dat phonebook.log 2>nul
	@echo 全部清理完成

help:
	@echo Available commands:
	@echo   make        - Compile program
	@echo   make run    - Compile and run
	@echo   make clean  - Remove compiled files

.PHONY: all run clean cleanall help