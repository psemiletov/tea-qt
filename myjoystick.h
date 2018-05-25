//this code by Peter Semiletov is the public domain

#ifndef MYJOYSTICK_H
#define MYJOYSTICK_H


#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>


#include <QObject>
#include <QEvent>


#if defined(Q_OS_UNIX)
#include <linux/joystick.h>
#endif

//#define J_EVENT_AXIS (QEvent::User + 1)
//#define J_EVENT_BUTTON (QEvent::User + 2)

const QEvent::Type evtJoystickAxis = QEvent::Type(QEvent::User + 1);
const QEvent::Type evtJoystickButton = QEvent::Type(QEvent::User + 2);


class CJoystickButtonEvent: public QEvent
{

public:

  uint button;
  bool pressed;
    
  CJoystickButtonEvent (QEvent::Type type): QEvent (type) {}
};


class CJoystickAxisEvent: public QEvent
{
public:
  
  uint axis;
  qint16 value;
    
  CJoystickAxisEvent (QEvent::Type type): QEvent(type) {}
};


class CJoystick: public QObject
{
  Q_OBJECT
    
public:

    int fd;
    
    QString description;
    uint id;
    bool valid;
    
    //QMap<uint, float> AxisValues;
    //QMap<uint, bool> ButtonValues;
    
    QObject *receiver;
    
    uint axis;
    uint buttons;

    CJoystick (uint idn, QObject *upper_link);
    ~CJoystick(); 
    
#if defined(Q_OS_UNIX)
    void process_event (js_event e);
#endif

    
public slots:    
    void read_joystick();
 /*   
signals:
    void joystickEvent (QEvent *event);
    void joystickButtonEvent(CJoystickButtonEvent *event);
    void joystickAxisEvent(CJoystickAxisEvent *event);    
   */ 
};

#endif
