UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
ASMLIB = libaelf64.a
else
ASMLIB = libacof64.lib
endif

CXX=g++
CFLAGS=-Wall -std=c++11 -O3
	
all: countSA locateSA

countSA: test/countSA.cpp libsa.a libs/$(ASMLIB)
	$(CXX) $(CFLAGS) test/countSA.cpp libsa.a libs/$(ASMLIB) -o test/countSA
	
locateSA: test/locateSA.cpp libsa.a libs/$(ASMLIB)
	$(CXX) $(CFLAGS) test/locateSA.cpp libsa.a libs/$(ASMLIB) -o test/locateSA

libsa.a: sa.h sa.cpp shared/common.h shared/common.cpp shared/patterns.h shared/patterns.cpp shared/timer.h shared/timer.cpp shared/sais.h shared/sais.c shared/xxhash.h shared/xxhash.c shared/hash.h shared/hash.cpp
	$(CXX) $(CFLAGS) -c sa.cpp shared/common.cpp shared/patterns.cpp shared/timer.cpp shared/sais.c shared/xxhash.c shared/hash.cpp
	ar rcs libsa.a sa.o common.o patterns.o timer.o sais.o xxhash.o hash.o
	make cleanObjects

cleanObjects:
	rm -f *o

clean:
	rm -f *o test/countSA test/locateSA libsa.a