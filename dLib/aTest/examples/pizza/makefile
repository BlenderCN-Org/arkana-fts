COMPILER = g++
CCFLAGS = -g -ansi
LIBFLAGS = ../../aTest.a

%.o : %.cpp
	$(COMPILER) $(CCFLAGS) -c $<

#objects = main.o trivial.o peetsaTest.o
objects = main.o peetsaTest.o peetsaTestSuites.o dough.o sauce.o ingredients.o # peetsaTestFixtures.cpp

all: aTest peetsa

aTest:
	cd ../..; make all

peetsa: $(objects)
	${COMPILER} -o peetsa $(objects) ${LIBFLAGS}

test:
	./peetsa

main.o: main.cpp ../../aTest.a
# trivial.o: trivial.cpp
peetsaTest.o: peetsaTest.cpp peetsa.h dough.h sauce.h ingredients.h
peetsaTestSuites.o: peetsaTestSuites.cpp peetsa.h dough.h sauce.h ingredients.h
#peetsaTestFixtures.o: peetsaTestFixtures.cpp peetsa.h dough.h sauce.h ingredients.h
dough.o: dough.cpp dough.h
sauce.o: sauce.cpp sauce.h
ingredients.o: ingredients.cpp ingredients.h

clean:
	rm $(objects) peetsa
