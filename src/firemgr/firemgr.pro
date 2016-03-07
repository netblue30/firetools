QMAKE_CXXFLAGS += $$(CXXFLAGS) -fstack-protector-all -D_FORTIFY_SOURCE=2 -fPIE -pie -Wformat -Wformat-security
QMAKE_CFLAGS += $$(CFLAGS) -fstack-protector-all -D_FORTIFY_SOURCE=2 -fPIE -pie -Wformat -Wformat-security
QMAKE_LFLAGS += $$(LDFLAGS) -Wl,-z,relro -Wl,-z,now
QT += widgets
 HEADERS       = firemgr.h mainwindow.h topwidget.h fs.h
 SOURCES       = mainwindow.cpp topwidget.cpp main.cpp \
		  ../common/utils.cpp fs.cpp
	
                 
RESOURCES = firemgr.qrc
TARGET=../../build/firemgr
