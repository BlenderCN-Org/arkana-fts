COMPILER = g++
CCFLAGS = -g -ansi -I../..
LIBFLAGS = ../../aTest.a

%.o : %.cpp
	$(COMPILER) $(CCFLAGS) -c $<

objects = main.o MyTestClass.o simplest_test.o fixture_test.o suite_test.o fixture_suite_test.o

all: aTest test

aTest:
	cd ../..; make all

test: $(objects)
	${COMPILER} -o test $(objects) ${LIBFLAGS}
	./test

main.o: main.cpp ../../aTest.a
MyTestClass.o: MyTestClass.cpp
simplest_test.o: simplest_test.cpp
fixture_test.o: fixture_test.cpp
fixture_suite_test.o: fixture_suite_test.cpp
suite_test.o: suite_test.cpp

clean:
	cd ../..; make clean
	rm $(objects) test
