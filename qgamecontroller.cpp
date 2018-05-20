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

#include "qgamecontroller.h"
#include "qgamecontroller_p.h"

#ifdef Q_OS_LINUX
#include "qgamecontroller_linux_p.h"
#elif defined (Q_OS_WIN)
#include "qgamecontroller_win_p.h"
#elif defined (Q_OS_MAC)
#include "qgamecontroller_mac_p.h"
#else
#error Unsupported OS
#endif

#include <QDebug>

QGameControllerEvent::QGameControllerEvent(uint controllerId)
    :d_ptr(new QGameControllerEventPrivate(this))
{
    Q_D(QGameControllerEvent);
    d->ControllerId=controllerId;
}

QGameControllerEvent::QGameControllerEvent(uint controllerId, QGameControllerEventPrivate &d)
    :d_ptr(&d)
{
    d.ControllerId=controllerId;
}

uint QGameControllerEvent::controllerId()
{
    Q_D(QGameControllerEvent);
    return d->ControllerId;
}

QGameControllerEvent::~QGameControllerEvent()
{
    delete d_ptr;
}

QGameControllerButtonEvent::QGameControllerButtonEvent(uint controllerId, uint button, bool pressed)
    : QGameControllerEvent(controllerId, *new QGameControllerButtonEventPrivate(this))
{
    Q_D(QGameControllerButtonEvent);
    d->Button=button;
    d->Pressed=pressed;
}

uint QGameControllerButtonEvent::button()
{
    Q_D(QGameControllerButtonEvent);
    return d->Button;
}

bool QGameControllerButtonEvent::pressed()
{
    Q_D(QGameControllerButtonEvent);
    return d->Pressed;
}

QGameControllerAxisEvent::QGameControllerAxisEvent(uint controllerId, uint axis, float value)
    : QGameControllerEvent(controllerId, *new QGameControllerAxisEventPrivate(this))
{
    Q_D(QGameControllerAxisEvent);
    d->Axis=axis;
    d->Value=value;
}

uint QGameControllerAxisEvent::axis()
{
    Q_D(QGameControllerAxisEvent);
    return d->Axis;
}

float QGameControllerAxisEvent::value()
{
    Q_D(QGameControllerAxisEvent);
    return d->Value;
}

QGameController::QGameController(uint id, QObject *parent) :
    QObject(parent), d_ptr(new QGameControllerPrivate(id, this))
{

}

uint QGameController::axisCount()
{
    Q_D(QGameController);
    return d->Axis;
}

uint QGameController::buttonCount()
{
    Q_D(QGameController);
    return d->Buttons;
}

float QGameController::axisValue(uint axis)
{
    Q_D(QGameController);
    return d->AxisValues.value(axis);
}

bool QGameController::buttonValue(uint button)
{
    Q_D(QGameController);
    return d->ButtonValues.value(button);
}

QString QGameController::description()
{
    Q_D(QGameController);
    return d->Description;
}

uint QGameController::id()
{
    Q_D(QGameController);
    return d->ID;
}

bool QGameController::isValid()
{
    Q_D(QGameController);
    return d->Valid;
}

void QGameController::readGameController()
{
    Q_D(QGameController);
    d->readGameController();
}
