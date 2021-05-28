CC = g++

all: logger.so logger

logger: logger.so hw2.cpp
	$(CC) hw2.cpp -o logger

logger.so: logger.cpp
	$(CC) -o logger.so -shared -fPIC logger.cpp -ldl

.PHONY: clean
clean:
	rm logger.so logger