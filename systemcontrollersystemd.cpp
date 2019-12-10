/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2019 Michael Zanetti <michael.zanetti@nymea.io>          *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *                                                                         *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <QDBusInterface>
#include <QDBusPendingReply>
#include <QTimeZone>

#include "systemcontrollersystemd.h"

#include "loggingcategories.h"

SystemControllerSystemd::SystemControllerSystemd(QObject *parent):
    PlatformSystemController(parent)
{

    QDBusInterface logind("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", QDBusConnection::systemBus());

    QDBusPendingReply<QString> canPowerOff = logind.callWithArgumentList(QDBus::Block, "CanPowerOff", {});
    canPowerOff.waitForFinished();

    if (canPowerOff.isError()) {
        qCWarning(dcPlatform) << "DBus call to logind failed:" << canPowerOff.error().name() <<  canPowerOff.error().message();
    } else if (canPowerOff.value() == "yes") {
        m_canControlPower = true;
    }

    QDBusInterface timedated("org.freedesktop.timedate1", "/org/freedesktop/timedate1", "org.freedesktop.DBus.Peer", QDBusConnection::systemBus());
    QDBusPendingReply<void> ping = timedated.call("Ping");
    if (ping.isError()) {
        qCWarning(dcPlatform()) << "DBus call to timedated failed:" << ping.error().name() << ping.error().message();
    } else {
        m_canControlTimeZone = true;
    }
}

bool SystemControllerSystemd::powerManagementAvailable() const
{
    return m_canControlPower;
}

bool SystemControllerSystemd::reboot()
{
    qCDebug(dcPlatform) << "Rebooting";
    QDBusInterface logind("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", QDBusConnection::systemBus());
    QDBusPendingReply<> powerOff = logind.callWithArgumentList(QDBus::Block, "Reboot", {true, });
    powerOff.waitForFinished();
    if (powerOff.isError()) {
        const auto error = powerOff.error();
        qCWarning(dcPlatform) << "Error calling reboot on logind.";
        return false;
    }
    return true;
}

bool SystemControllerSystemd::shutdown()
{
    qCDebug(dcPlatform()) << "Shutting down...";
    QDBusInterface logind("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", QDBusConnection::systemBus());
    QDBusPendingReply<> powerOff = logind.callWithArgumentList(QDBus::Block, "PowerOff", {true, });
    powerOff.waitForFinished();
    if (powerOff.isError()) {
        qCWarning(dcPlatform) << "Error calling poweroff on logind.";
        return false;
    }
    return true;
}

bool SystemControllerSystemd::timeZoneManagementAvailable() const
{
    return m_canControlTimeZone;
}

bool SystemControllerSystemd::setTimeZone(const QTimeZone &timeZone)
{
    QDBusInterface timedated("org.freedesktop.timedate1", "/org/freedesktop/timedate1", "org.freedesktop.timedate1", QDBusConnection::systemBus());
    QDBusPendingReply<> setTimeZone = timedated.callWithArgumentList(QDBus::Block, "SetTimezone", {QString(timeZone.id()), false});
    if (setTimeZone.isError()) {
        qCWarning(dcPlatform()) << "Error setting time zone:" << setTimeZone.error().name() << setTimeZone.error().message();
        return false;
    }
    return true;
}
