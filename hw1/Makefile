CC = g++
CFLAGS = -g -Wall
obj = main.o utils.o

main: $(obj)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean
clean:
	rm *.o main