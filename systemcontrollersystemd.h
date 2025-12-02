// SPDX-License-Identifier: GPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea-system-plugin-systemd.
*
* nymea-system-plugin-systemd is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* nymea-system-plugin-systemd is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with nymea-system-plugin-systemd. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef SYSTEMCONTROLLERSYSTEMD_H
#define SYSTEMCONTROLLERSYSTEMD_H

#include <QObject>

#include "platform/platformsystemcontroller.h"

class SystemControllerSystemd: public PlatformSystemController
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "io.nymea.PlatformSystemController")
    Q_INTERFACES(PlatformSystemController)
public:
    explicit SystemControllerSystemd(QObject *parent = nullptr);

    bool powerManagementAvailable() const override;

    bool restart() override;
    bool reboot() override;
    bool shutdown() override;

    bool timeManagementAvailable() const override;
    bool setTime(const QDateTime &dateTime) override;
    bool setTimeZone(const QTimeZone &timeZone) override;
    bool automaticTimeAvailable() const override;
    bool automaticTime() const override;
    bool setAutomaticTime(bool automaticTime) override;

private slots:
    void timePropertiesChanged(const QString &interface, const QVariantMap &changedProperties, const QStringList &invalidatedProperties);
private:
    bool m_canControlPower = false;
    bool m_canControlTime = false;
};

#endif // SYSTEMCONTROLLERSYSTEMD_H
