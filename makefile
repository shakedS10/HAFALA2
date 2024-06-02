CC = g++
CFLAGS = -Wall -Wextra -std=c++11

all: mync ttt fsdgds


ttt: ttt.cpp
	$(CC) $(CFLAGS) -fprofile-arcs -ftest-coverage -o ttt ttt.cpp

mync: mync.cpp
	$(CC) $(CFLAGS) -fprofile-arcs -ftest-coverage -o mync mync.cpp

fsdgds: fsdgds.cpp
	$(CC) $(CFLAGS) -fprofile-arcs -ftest-coverage -o fsdgds fsdgds.cpp


clean:
	rm -f *.gcda *.gcno *.gcov mync ttt fsdgds