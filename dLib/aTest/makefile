COMPILER = g++
CCFLAGS = -g -ansi
LIBFLAGS =

%.o : %.cpp
	$(COMPILER) $(CCFLAGS) -c $<

objects = Test.o TestRegistry.o TestResult.o TestResultNiceOutput.o TestResultCompiler.o TestResultCombiner.o TestResultSQL.o TestSuite.o

all: aTest

aTest: $(objects)
	ar cr aTest.a $(objects)

Test.o: Test.cpp Test.h
TestRegistry.o: TestRegistry.cpp TestRegistry.h
TestResult.o: TestResult.cpp TestResult.h
TestResultCompiler.o: TestResultCompiler.cpp TestResultCompiler.h
TestResultCombiner.o: TestResultCombiner.cpp TestResultCombiner.h
TestResultNiceOutput.o: TestResultNiceOutput.cpp TestResultNiceOutput.h
TestResultSQL.o: TestResultSQL.cpp TestResultSQL.h
TestSuite.o: TestSuite.cpp TestSuite.h

clean:
	rm $(objects) aTest.a
