 # look for include files in
 #   each of the modules
 # CFLAGS += $(patsubst %,-I%,$(MODULES))
CFLAGS += -g -p -pg -Wall -DUSE_STD_ALLOC -I../.. -I../../linux/include/ftplib
CXXFLAGS += $(CFLAGS)

LIBS = -lstdc++

EXTERNALS = ../../linux/lib/ftplib/ftplib.cpp \
            ../../dLib/dBrowse/linux.cpp \
            ../../dLib/dBrowse/wildcard.cpp \
            ../../dLib/dProcess/dProcess.cpp

all:
	gcc $(CFLAGS) $(EXTERNALS) $(LIBS) *.cpp -o fts_miniupdater

 # clean
clean:
	rm -f fts_miniupdater
