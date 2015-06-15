all: schnorr
CC = g++
INCLUDE = .
CFLAGS = -O2 -std=c++11 -pthread

schnorr: schnorr.o
	${CC} schnorr.o -o $@ -lcrypto -lgmpxx -lgmp -pthread
schnorr.o: schnorr.cpp
	${CC} $< ${CFLAGS} -c
.PHONY: clean
clean:
	@rm -rf *.o
