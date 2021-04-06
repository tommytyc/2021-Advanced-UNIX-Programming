CC = g++
CFLAGS = -g
obj = hw1.o utils.o

.cpp.o:
	$(CC) $(CFLAGS) -c $<

hw1: $(obj)
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm *.o hw1