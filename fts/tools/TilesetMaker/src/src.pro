SOURCES += main.cpp \
 tilesetmaker.cpp
TEMPLATE = app
CONFIG += warn_on \
	  thread \
          qt \
          console
TARGET = ../bin/tilesetmaker
FORMS += MainDlg.ui

HEADERS += tilesetmaker.h

RESOURCES += application.qrc
