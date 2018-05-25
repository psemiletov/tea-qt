//this code by Peter Semiletov is the public domain

#include "myjoystick.h"

#include <QDebug>
#include <QApplication>


CJoystick::~Joystick()
{
  close (fd);   
} 


CJoystick::CJoystick (uint idn, QObject *upper_link)
{
#if defined(Q_OS_UNIX)

  receiver = upper_link;

  id = idn;
  
  valid = false;
  axis = 0;
  buttons = 0;
    
  QString filename = "/dev/input/js%1" + QString::number(id);
 
  if (( fd = open (filename.toUtf8().data(), O_NONBLOCK)) == -1)
     {
      qDebug() << "Couldn't open " << filename;
      return;
     }
    
  valid = true;
    
  char number_of_axes = 0;
  char number_of_buttons = 0;
    
  ioctl (fd, JSIOCGAXES, &number_of_axes);
  ioctl (fd, JSIOCGBUTTONS, &number_of_buttons);
    
  axis = number_of_axes;
  buttons = number_of_buttons;
    
  char jname[80];
    
  ioctl (fd, JSIOCGNAME(80), &jname);
  description = jname;
    
  read_joystick();
#endif    
}

void CJoystick::read_joystick()
{
#if defined(Q_OS_UNIX)

  if (! valid)
     return;
        
    
  struct js_event e;
    
  while (read (fd, &e, sizeof(e)) > 0) 
        {
         process_event (e);
        }
 
    if (errno != EAGAIN) 
       {
        qDebug() << "Joystick read error";
        valid = false;
        }
#endif
}


void CJoystick::process_event (js_event e)
{
#if defined(Q_OS_UNIX)
    
  if (e.type & JS_EVENT_INIT)
      {
  //        qDebug() << "process_event" << "event was a JS_EVENT_INIT" << e.number << e.value << e.type;
       }
    
//  qint16 value = e.value;

  if (e.type & JS_EVENT_BUTTON)
    {
     if (e.value == 1)
       {
//            qDebug("Button %i pressed.", e.number);
       }
        else
            {
//            qDebug("Button %i released.", e.number);
             }
        
     //event = new CJoystickButtonEvent (QEvent::Type (QEvent::User + 1), e.number, value);

     CJoystickButtonEvent *event = new CJoystickButtonEvent (evtJoystickButton);
     event->button = e.number;
     event->pressed = e.value;
    
    
    
     //ButtonValues.insert (e.number, value);
    
  //  emit (gameControllerButtonEvent((QGameControllerButtonEvent*)event));
     //event->my_data = "test";
     
     //QApplication::postEvent(this, reinterpret_cast<QEvent*>(event));
     QApplication::postEvent(receiver, reinterpret_cast<QEvent*>(event));
     
     
     
    } 
     else if (e.type & JS_EVENT_AXIS) 
         {
         /*
        float val;
        if (value < 0)
            val = (float)value/32768.0;
        else
            val = (float)value/32767.0;
        */
        CJoystickAxisEvent *event=new CJoystickAxisEvent (evtJoystickAxis);
        event->axis = e.number;
        //event->value = val;
        event->value = e.value;
        
       QApplication::postEvent(receiver, reinterpret_cast<QEvent*>(event));
     
       
        //qDebug("Axis %i moved to %f.", e.number , val);
    }
    
#endif    
}
