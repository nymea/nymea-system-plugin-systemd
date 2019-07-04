QT -= gui
QT += dbus

TARGET = nymea_systempluginsystemd
TEMPLATE = lib

CONFIG += plugin link_pkgconfig c++11
PKGCONFIG += nymea

SOURCES += \
    systemcontrollersystemd.cpp


HEADERS += \
    systemcontrollersystemd.h


target.path = $$[QT_INSTALL_LIBS]/nymea/platform/
INSTALLS += target
