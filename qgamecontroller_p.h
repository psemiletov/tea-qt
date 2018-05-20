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

#ifndef QGAMECONTROLLER_P_H
#define QGAMECONTROLLER_P_H

#include <QMap>

#include "qgamecontroller.h"

QT_BEGIN_NAMESPACE

struct QGameControllerEventPrivate
{    
    Q_DECLARE_PUBLIC(QGameControllerEvent)
    QGameControllerEventPrivate(QGameControllerEvent *q) : q_ptr(q) { }
    QGameControllerEvent * const q_ptr;
    uint ControllerId;
};


struct QGameControllerButtonEventPrivate : public QGameControllerEventPrivate
{
    Q_DECLARE_PUBLIC(QGameControllerButtonEvent)
    QGameControllerButtonEventPrivate(QGameControllerEvent *q) : QGameControllerEventPrivate(q) { }
    uint Button;
    bool Pressed;
};

struct QGameControllerAxisEventPrivate : public QGameControllerEventPrivate
{
    Q_DECLARE_PUBLIC(QGameControllerAxisEvent)
    QGameControllerAxisEventPrivate(QGameControllerEvent *q) : QGameControllerEventPrivate(q) { }
    uint Axis;
    float Value;
};

QT_END_NAMESPACE

#endif // QGAMECONTROLLER_P_H
