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

#ifndef DEBIANSYSTEMCONTROLLER_H
#define DEBIANSYSTEMCONTROLLER_H

#include <QObject>

#include "platform/platformsystemcontroller.h"

class DebianSystemController : public PlatformSystemController
{
    Q_OBJECT
public:
    explicit DebianSystemController(QObject *parent = nullptr);

    bool powerManagementAvailable() const override;

    bool reboot() override;
    bool shutdown() override;

private:
    bool m_canControlPower = false;;
};

#endif // DEBIANLINUXSYSTEMCONTROLLER_H
