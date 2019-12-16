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
#include <QTimer>

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

    m_canControlTime = QDBusConnection::systemBus().connect("org.freedesktop.timedate1", "/org/freedesktop/timedate1", "org.freedesktop.DBus.Properties", "PropertiesChanged", this, SLOT(timePropertiesChanged(const QString &, const QVariantMap &, const QStringList &)));
}

bool SystemControllerSystemd::powerManagementAvailable() const
{
    return m_canControlPower;
}

bool SystemControllerSystemd::reboot()
{
    QDBusInterface logind("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", QDBusConnection::systemBus());
    QDBusPendingReply<> powerOff = logind.callWithArgumentList(QDBus::Block, "Reboot", {true, });
    powerOff.waitForFinished();
    if (powerOff.isError()) {
        const auto error = powerOff.error();
        qCWarning(dcPlatform) << "Error calling reboot on logind.";
        return false;
    }
    qCDebug(dcPlatform) << "Rebooting...";
    return true;
}

bool SystemControllerSystemd::shutdown()
{
    QDBusInterface logind("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", QDBusConnection::systemBus());
    QDBusPendingReply<> powerOff = logind.callWithArgumentList(QDBus::Block, "PowerOff", {true, });
    powerOff.waitForFinished();
    if (powerOff.isError()) {
        qCWarning(dcPlatform) << "Error calling poweroff on logind.";
        return false;
    }
    qCDebug(dcPlatform()) << "Shutting down...";
    return true;
}

bool SystemControllerSystemd::timeManagementAvailable() const
{
    return m_canControlTime;
}

bool SystemControllerSystemd::setTime(const QDateTime &dateTime)
{
    QDBusInterface timedated("org.freedesktop.timedate1", "/org/freedesktop/timedate1", "org.freedesktop.timedate1", QDBusConnection::systemBus());
    QDBusPendingReply<> setTimeZone = timedated.callWithArgumentList(QDBus::Block, "SetTime", {dateTime.toMSecsSinceEpoch() * 1000, false, false});
    if (setTimeZone.isError()) {
        qCWarning(dcPlatform()) << "Error setting time zone:" << setTimeZone.error().name() << setTimeZone.error().message();
        return false;
    }
    qCDebug(dcPlatform()) << "System time changed:" << dateTime;
    emit timeConfigurationChanged();
    return true;
}

bool SystemControllerSystemd::setTimeZone(const QTimeZone &timeZone)
{
    QDBusInterface timedated("org.freedesktop.timedate1", "/org/freedesktop/timedate1", "org.freedesktop.timedate1", QDBusConnection::systemBus());
    QDBusPendingReply<> setTimeZone = timedated.callWithArgumentList(QDBus::Block, "SetTimezone", {QString(timeZone.id()), false});
    if (setTimeZone.isError()) {
        qCWarning(dcPlatform()) << "Error setting time zone:" << setTimeZone.error().name() << setTimeZone.error().message();
        return false;
    }
    qCDebug(dcPlatform()) << "Time zone set to" << timeZone;
    emit timeConfigurationChanged();
    return true;
}

bool SystemControllerSystemd::automaticTimeAvailable() const
{
    QDBusInterface timedated("org.freedesktop.timedate1", "/org/freedesktop/timedate1", "org.freedesktop.timedate1", QDBusConnection::systemBus());
    return timedated.property("CanNTP").toBool();
}

bool SystemControllerSystemd::automaticTime() const
{
    QDBusInterface timedated("org.freedesktop.timedate1", "/org/freedesktop/timedate1", "org.freedesktop.timedate1", QDBusConnection::systemBus());
    return timedated.property("NTP").toBool();
}

bool SystemControllerSystemd::setAutomaticTime(bool automaticTime)
{
    QDBusInterface timedated("org.freedesktop.timedate1", "/org/freedesktop/timedate1", "org.freedesktop.timedate1", QDBusConnection::systemBus());
    QDBusPendingReply<> disableNtp = timedated.callWithArgumentList(QDBus::Block, "SetNTP", {automaticTime, false});
    if (disableNtp.isError()) {
        qCWarning(dcPlatform()) << "Error disabling NTP:" << disableNtp.error().name() << disableNtp.error().message();
        return false;
    }
    qCDebug(dcPlatform()) << "Automatic time updates" << (automaticTime ? "enabled" : "disabled");

    if (automaticTime) {
        // If enabling NTP, wait 2 secs before emitting changed again, NTP should have processed by then...
        // If not, it probably failed anyways
        QTimer::singleShot(2000, this, [this](){
            emit timeConfigurationChanged();
        });
    }
    emit timeConfigurationChanged();
    return true;
}

void SystemControllerSystemd::timePropertiesChanged(const QString &interface, const QVariantMap &changedProperties, const QStringList &invalidatedProperties)
{
    qCDebug(dcPlatform()) << "Time configuration changed" << interface << changedProperties << invalidatedProperties;
    emit timeConfigurationChanged();
}
