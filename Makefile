CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -pthread -I./include
TARGET  = log_analyzer

SRCS = src/main.c src/file_loader.c src/partitioner.c src/worker.c src/aggregator.c src/output.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET) test.log results.json
