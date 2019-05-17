QT -= gui
QT += dbus

TARGET = nymea_systempluginlogind
TEMPLATE = lib

CONFIG += link_pkgconfig c++11
PKGCONFIG += nymea

SOURCES += \
    systemcontrollerlogind.cpp


HEADERS += \
    systemcontrollerlogind.h


target.path = $$[QT_INSTALL_LIBS]/nymea/platform/
INSTALLS += target
