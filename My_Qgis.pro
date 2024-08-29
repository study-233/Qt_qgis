QT       += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += D:\\OSGeo4W\\apps\\qgis-ltr-dev\\include
LIBS += -L"D:\\OSGeo4W\\apps\\qgis-ltr-dev\\lib" -lqgis_app -lqgis_core -lqgis_gui -lqgis_native

SOURCES += \
    main.cpp \
    mainwindows.cpp \
    qgis_devlayertreeviewmenuprovider.cpp \
    segmentation.cpp

HEADERS += \
    mainwindows.h \
    qgis_devlayertreeviewmenuprovider.h \
    segmentation.h

FORMS += \
    mainwindows.ui \
    segmentation.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
