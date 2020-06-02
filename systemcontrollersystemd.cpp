/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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

bool SystemControllerSystemd::restart()
{
    QDBusInterface systemd("org.freedesktop.systemd1", "/org/freedesktop/systemd1/unit/nymead_2eservice", "org.freedesktop.systemd1.Unit", QDBusConnection::systemBus());
    QDBusPendingReply<> restart = systemd.callWithArgumentList(QDBus::Block, "Restart", {"replace"});
    restart.waitForFinished();
    if (restart.isError()) {
        const auto error = restart.error();
        qCWarning(dcPlatform()) << "Error restarting nymea:" << error.message();
    }
    qCDebug(dcPlatform()) << "Restarting nymea...";
    return true;
}

bool SystemControllerSystemd::reboot()
{
    QDBusInterface logind("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", QDBusConnection::systemBus());
    QDBusPendingReply<> powerOff = logind.callWithArgumentList(QDBus::Block, "Reboot", {true, });
    powerOff.waitForFinished();
    if (powerOff.isError()) {
        const auto error = powerOff.error();
        qCWarning(dcPlatform) << "Error calling reboot on logind:" << error.message();
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
        const auto error = powerOff.error();
        qCWarning(dcPlatform) << "Error calling poweroff on logind:" << error.message();
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
    QDBusPendingReply<> setNtp = timedated.callWithArgumentList(QDBus::Block, "SetNTP", {automaticTime, false});
    if (setNtp.isError()) {
        qCWarning(dcPlatform()) << "Error disabling NTP:" << setNtp.error().name() << setNtp.error().message();
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
