win32:TEMPLATE = vcapp
TARGET = dvrcp

win32:QT += xml
unix:QT += xml

win32:DESTDIR = "../../bin/windows/debug"
unix:DESTDIR = $${OUTPUT_BIN}
unix:MOC_DIR = $${OUTPUT_OBJ}
unix:OBJECTS_DIR = $${OUTPUT_OBJ}
unix:UI_DIR = $${OUTPUT_OBJ}
QMAKE_LFLAGS_WINDOWS += /DEBUG /PDB:$(TargetDir)dvrcp.pdb

INCLUDEPATH += "../../include"

win32:LIBS += -lddraw -ldxguid -lmsimg32

unix:LIBS += -lsdvr_sdk -lsdvr_ui_sdk -lsct -laio -lrt -L$${SDK_LIB} -lXv

unix:POST_TARGETDEPS = $${SDK_LIB}/libsct.a \
                       $${SDK_LIB}/libsdvr_sdk.a

SOURCES = main.cpp \
          mainwindow.cpp \
          videopanel.cpp \
          viewsettings.cpp \
          dvrsystem.cpp \
          dvrsystemtree.cpp \
          dvrboard.cpp \
          boarddialog.cpp \
          camera.cpp \
          cameradialog.cpp \
          connectdialog.cpp \
          decoder.cpp \
          decoderview.cpp \
          decoderdialog.cpp \
          monitordialog.cpp \
          player.cpp \
          recorder.cpp \
          recview.cpp \
          regiondialog.cpp \
          relaysdialog.cpp \
          sensorsdialog.cpp \
          sensorshandler.cpp \
          smodialog.cpp \
          smoyuvplayer.cpp \
          spotmonitor.cpp \
          statviewdialog.cpp \
          systemdialog.cpp \
          alarmhandler.cpp \
          aboutdialog.cpp \
          log.cpp \
          avi_api.cpp \
          sdvr.cpp \
          ioctldialog.cpp

HEADERS = mainwindow.h \
          videopanel.h \
          viewsettings.h \
          dvrsystem.h \
          dvrsystemtree.h \
          dvrboard.h \
          boarddialog.h \
          camera.h \
          cameradialog.h \
          connectdialog.h \
          decoder.h \
		  decoderview.h \
          decoderdialog.h \
          monitordialog.h \
          player.h \
          recorder.h \
          recview.h \
          regiondialog.h \
          relaysdialog.h \
          sensorsdialog.h \
          sensorshandler.h \
          smodialog.h \
          smoyuvplayer.h \
          spotmonitor.h \
          statviewdialog.h \
          systemdialog.h \
          alarmhandler.h \
          aboutdialog.h \
          log.h \
          avi_api.h \
          sdvr.h \
          ioctldialog.h

FORMS   = aboutdialog.ui \
          boarddialog.ui \
          cameradialog.ui \
          camerasetupdialog.ui \
          connectdialog.ui \
          decoderdialog.ui \
          monitordialog.ui \
          regionsdialog.ui \
          relaysdialog.ui \
          sensorsdialog.ui \
          smodialog.ui \
          smoyuvplayer.ui \
          startupdialog.ui \
          statviewdialog.ui \
          systemdialog.ui \
          ioctldialog.ui


RESOURCES = dvrcp.qrc
