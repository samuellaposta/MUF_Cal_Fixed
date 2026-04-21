QT += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
TARGET   = MUFCalc
TEMPLATE = app

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/mufengine.cpp \
    src/validationengine.cpp \
    src/uncertaintyengine.cpp \
    src/diagnosticsengine.cpp \
    src/reportgenerator.cpp \
    src/dataentrywizard.cpp \
    src/resultspanel.cpp \
    src/aboutdialog.cpp \
    src/historymanager.cpp \
    src/userguidedialog.cpp \
    src/virtualfacilitydialog.cpp

HEADERS += \
    include/datatypes.h \
    include/mainwindow.h \
    include/mufengine.h \
    include/validationengine.h \
    include/uncertaintyengine.h \
    include/diagnosticsengine.h \
    include/reportgenerator.h \
    include/dataentrywizard.h \
    include/resultspanel.h \
    include/aboutdialog.h \
    include/historymanager.h \
    include/styles.h \
    include/watermarkwidget.h \
    include/userguidedialog.h \
    include/virtualfacilitydialog.h \
    include/measurementmethoddb.h \
    include/nucleartransferitemdb.h

RESOURCES += resources/resources.qrc

INCLUDEPATH += include

# Deployment rules
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RC_ICONS = resources/MUFCalc.ico


