QMAKE_CXXFLAGS += $$(CXXFLAGS) -fstack-protector-all -D_FORTIFY_SOURCE=2 -fPIE -pie -Wformat -Wformat-security
QMAKE_CFLAGS += $$(CFLAGS) -fstack-protector-all -D_FORTIFY_SOURCE=2 -fPIE -pie -Wformat -Wformat-security
QMAKE_LFLAGS += $$(LDFLAGS) -Wl,-z,relro -Wl,-z,now
QT += widgets
 HEADERS       = ../common/utils.h ../common/pid.h ../common/common.h \
 		firejail_ui.h wizard.h home_widget.h help_widget.h appdb.h
 SOURCES       = main.cpp \
 		wizard.cpp \
 		home_widget.cpp \
 		help_widget.cpp \
 		appdb.cpp \
		../common/utils.cpp \
		../common/pid.cpp
#RESOURCES = firejail-ui.qrc
TARGET=../../build/firejail-ui
