QT -= gui
QT += dbus

TARGET = $$qtLibraryTarget(nymea_systempluginsystemd)
TEMPLATE = lib

greaterThan(QT_MAJOR_VERSION, 5) {
    message("Building using Qt6 support")
    CONFIG *= c++17
    QMAKE_LFLAGS *= -std=c++17
    QMAKE_CXXFLAGS *= -std=c++17
} else {
    message("Building using Qt5 support")
    CONFIG *= c++11
    QMAKE_LFLAGS *= -std=c++11
    QMAKE_CXXFLAGS *= -std=c++11
    DEFINES += QT_DISABLE_DEPRECATED_UP_TO=0x050F00
}

CONFIG += plugin link_pkgconfig
PKGCONFIG += nymea

SOURCES += \
    systemcontrollersystemd.cpp

HEADERS += \
    systemcontrollersystemd.h


target.path = $$[QT_INSTALL_LIBS]/nymea/platform/
INSTALLS += target
