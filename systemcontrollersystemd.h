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

    bool reboot() override;
    bool shutdown() override;

    bool timeManagementAvailable() const override;
    bool setTime(const QDateTime &dateTime) override;
    bool setTimeZone(const QTimeZone &timeZone) override;
    bool automaticTimeAvailable() const override;
    bool automaticTime() const override;
    bool setAutomaticTime(bool automaticTime);

private slots:
    void timePropertiesChanged(const QString &interface, const QVariantMap &changedProperties, const QStringList &invalidatedProperties);
private:
    bool m_canControlPower = false;
    bool m_canControlTime = false;
};

#endif // SYSTEMCONTROLLERSYSTEMD_H
