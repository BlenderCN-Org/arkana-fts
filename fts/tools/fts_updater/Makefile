 # the source files extension
EXT := cpp

 # look for include files in
 #   each of the modules
 # CFLAGS += $(patsubst %,-I%,$(MODULES))
CFLAGS += -DUSE_STD_ALLOC \
	  -I../.. \
	  -I../../linux/include \
	  -I../../linux/include/ftplib \
	  -I../../linux/include/CEGUI \
	  -I/usr/local/include \
#	  -I/usr/local/include/CEGUI

CXXFLAGS += $(CFLAGS)

LDFLAGS += -L../../linux/lib/CEGUI

LIBS := -lSDL \
	-lSDL_image \
	-lGL \
	-lGLU \
	-lCEGUIBase \
	-lCEGUIFalagardWRBase \
	-lCEGUIOpenGLRenderer \
	-lCEGUISILLYImageCodec \
	-lCEGUIXercesParser \
	-lSILLY

EXTERNALS = ../../linux/lib/ftplib/ftplib.cpp \
            ../../dLib/dBrowse/linux.cpp \
            ../../dLib/dBrowse/wildcard.cpp \
            ../../dLib/dProcess/dProcess.cpp

 # link the program
all:
	gcc $(CFLAGS) $(LDFLAGS) -o fts_updater $(EXTERNALS) *.cpp $(LIBS)

 # clean
clean:
	rm -f fts_updater
