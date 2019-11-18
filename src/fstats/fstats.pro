QMAKE_CXXFLAGS += $$(CXXFLAGS) -fstack-protector-all -D_FORTIFY_SOURCE=2 -fPIE -pie -Wformat -Wformat-security
QMAKE_CFLAGS += $$(CFLAGS) -fstack-protector-all -D_FORTIFY_SOURCE=2 -fPIE -pie -Wformat -Wformat-security
QMAKE_LFLAGS += $$(LDFLAGS) -lrt -Wl,-z,relro -Wl,-z,now
QT += widgets
 HEADERS       = ../common/utils.h ../common/pid.h ../common/common.h \
 		  pid_thread.h db.h dbstorage.h dbpid.h stats_dialog.h graph.h fstats.h
 SOURCES       = main.cpp \
                 stats_dialog.cpp \
                pid_thread.cpp \
                db.cpp \
                dbpid.cpp \
                 graph.cpp \
                  ../common/utils.cpp \
                  ../common/pid.cpp \
                  config.cpp
RESOURCES = fstats.qrc
TARGET=../../build/fstats
