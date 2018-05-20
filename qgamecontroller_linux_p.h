/***************************************************************************
 *   Copyright (C) 2014 M Wellings                                         *
 *   info@openforeveryone.co.uk                                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundation                             *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef QGAMECONTROLLER_LINUX_P_H
#define QGAMECONTROLLER_LINUX_P_H

#include <QMap>

#include "qgamecontroller.h"
#include "qgamecontroller_p.h"

#include <iostream>
#include <unistd.h>
#include <fcntl.h>
//#include <sstream>
#include <errno.h>
#include <linux/joystick.h>

QT_BEGIN_NAMESPACE

class QGameControllerPrivate
{
    Q_DECLARE_PUBLIC(QGameController)
public:
    explicit QGameControllerPrivate(uint id, QGameController *q);
    QGameController * const q_ptr;
protected:
    void process_event(js_event e);
public:
    int fd;
    QString Description;
    uint ID;
    bool Valid;
    QMap<uint, float> AxisValues;
    QMap<uint, bool> ButtonValues;
    uint Axis;
    uint Buttons;

    void readGameController();
};

QT_END_NAMESPACE

#endif // QGAMECONTROLLER_LINUX_P_H
