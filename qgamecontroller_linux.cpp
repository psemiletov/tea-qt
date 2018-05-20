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
#include "qgamecontroller_linux_p.h"

#include <QDebug>

QGameControllerPrivate::QGameControllerPrivate(uint id, QGameController *q) :
    q_ptr(q)
{
    ID=id;
    Valid=false;
    Axis=0;
    Buttons=0;
//    qDebug("QGameController::QGameController(%i)", ID);
    char number_of_axes=0;
    char number_of_buttons=0;
    QString filename = QString("/dev/input/js%1").arg(QString::number(id));
    qDebug() << "Opening" << filename.toUtf8().data();
    if( ( fd = open( filename.toUtf8().data() , O_NONBLOCK)) == -1 )
    {
        qDebug( "Couldn't open joystick\n" );
        return;
    }
    Valid=true;
    ioctl (fd, JSIOCGAXES, &number_of_axes);
    ioctl (fd, JSIOCGBUTTONS, &number_of_buttons);
    Axis=number_of_axes;
    Buttons=number_of_buttons;
    char name_of_stick[80];
    ioctl (fd, JSIOCGNAME(80), &name_of_stick);
    Description=name_of_stick;
    qDebug("Joystick: \"%s\" has %i axis and %i buttons", name_of_stick, number_of_axes, number_of_buttons);
    readGameController();
}

void QGameControllerPrivate::readGameController()
{
    if (!Valid)
        return;
//    qDebug() << "readJoystick";
    struct js_event e;
    while (read (fd, &e, sizeof(e)) > 0) {
        process_event (e);
    }
    /* EAGAIN is returned when the queue is empty */
    if (errno != EAGAIN) {
        qDebug() << "Error";
        Valid=false;
    }
//    else
//        qDebug() << "No event";
}

void QGameControllerPrivate::process_event(js_event e)
{
    Q_Q(QGameController);
    QGameControllerEvent *event = NULL;
    if (e.type & JS_EVENT_INIT)
    {
//        qDebug() << "process_event" << "event was a JS_EVENT_INIT" << e.number << e.value << e.type;
    }
    qint16 value = e.value;
//    qDebug() << "process_event" << e.number << value << e.type;
    if (e.type & JS_EVENT_BUTTON)
    {
        if (e.value==1)
        {
//            qDebug("Button %i pressed.", e.number);
        }
        else
        {
//            qDebug("Button %i released.", e.number);
        }
        event=new QGameControllerButtonEvent(ID, e.number, value);
        ButtonValues.insert(e.number, value);
        emit(q->gameControllerButtonEvent((QGameControllerButtonEvent*)event));
    } else if (e.type & JS_EVENT_AXIS) {
        float Value;
        if (value<0)
            Value = (float)value/32768.0;
        else
            Value = (float)value/32767.0;
        AxisValues.insert(e.number, Value);
        event=new QGameControllerAxisEvent(ID, e.number, Value);
//        qDebug("Axis %i moved to %f.", e.number , Value);
        emit(q->gameControllerAxisEvent((QGameControllerAxisEvent*)event));
    }
    emit(q->gameControllerEvent(event));
    return;
}

