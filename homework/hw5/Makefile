CC = gcc
CFLAGS = -ggdb -O0
LDFLAGS = -lpthread -lcrypto
TARGET = task
SRC = task.c

$(TARGET): $(SRC)
        $(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

.PHONY: clean run

run: $(TARGET)
        ./$(TARGET) 3 12 1000 123

clean:
        rm -f $(TARGET)
