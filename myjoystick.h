//this code by Peter Semiletov is the public domain

#ifndef MYJOYSTICK_H
#define MYJOYSTICK_H

#include <QObject>

#if defined(Q_OS_UNIX)
#if !defined (Q_OS_MAC) || !defined (Q_OS_MACOS)

#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/joystick.h>

#include <QEvent>


const QEvent::Type evtJoystickAxis = QEvent::Type(QEvent::User + 1);
const QEvent::Type evtJoystickButton = QEvent::Type(QEvent::User + 2);


class CJoystickButtonEvent: public QEvent
{
public:

  uint button;
  bool pressed;
    
  CJoystickButtonEvent (QEvent::Type type): QEvent (type), button (0), pressed (false) {}
};


class CJoystickAxisEvent: public QEvent
{
public:
  
  uint axis;
  qint16 value;
    
  CJoystickAxisEvent (QEvent::Type type): QEvent(type), axis (0), value (0) {}
};


class CJoystick: public QObject
{
Q_OBJECT
    
public:

  QObject *receiver; //link to object that handle joystick events

  int fd; //joystick file descriptor 
  uint id; //joystick id
    
  QString description; //joystick text description
  bool initialized; 
       
  uint number_of_axis;
  uint number_of_buttons;

  CJoystick (uint idn, QObject *upper_link);
  ~CJoystick(); 
    
   void process_event (js_event e);

public slots:

  void read_joystick();
};

#endif
#endif //MAC
#endif
