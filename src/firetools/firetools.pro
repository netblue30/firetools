QMAKE_CXXFLAGS += $$(CXXFLAGS) -fstack-protector-all -D_FORTIFY_SOURCE=2 -fPIE -pie -Wformat -Wformat-security
QMAKE_CFLAGS += $$(CFLAGS) -fstack-protector-all -D_FORTIFY_SOURCE=2 -fPIE -pie -Wformat -Wformat-security
QMAKE_LFLAGS += $$(LDFLAGS) -Wl,-z,relro -Wl,-z,now
QT += widgets
# HEADERS       = mainwindow.h ../common/utils.h ../common/pid.h ../common/common.h applications.h \
#		  pid_thread.h db.h dbstorage.h dbpid.h stats_dialog.h graph.h firetools.h edit_dialog.h
 HEADERS       = mainwindow.h ../common/utils.h ../common/common.h applications.h \
		  firetools.h edit_dialog.h
 SOURCES       = mainwindow.cpp \
                 main.cpp \
 #                stats_dialog.cpp \
                 edit_dialog.cpp \
 #                pid_thread.cpp \
#                 db.cpp \
#                 dbpid.cpp \
#                 graph.cpp \
                  ../common/utils.cpp \
                  ../common/pid.cpp \
                  applications.cpp
RESOURCES = firetools.qrc
TARGET=../../build/firetools
