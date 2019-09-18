//this code by Peter Semiletov is the public domain

#include <QtGlobal>


#if defined(JOYSTICK_SUPPORTED)

#include <QDebug>
#include <QApplication>

#include "myjoystick.h"


CJoystick::~CJoystick()
{
  close (fd);
}


CJoystick::CJoystick (uint idn, QObject *upper_link)
{
  receiver = upper_link;
  id = idn;
  initialized = false;
  number_of_axis = 0;
  number_of_buttons = 0;
  axis_pressed = false;
  etype = 0;

  QString filename = "/dev/input/js" + QString::number (id);

  if ((fd = open (filename.toUtf8().data(), O_NONBLOCK)) == -1)
     {
      qDebug() << "Cannot open " << filename;
      return;
     }

  initialized = true;

  char num_of_axis = 0;
  char num_of_buttons = 0;
  char jname[80];

  ioctl (fd, JSIOCGAXES, &num_of_axis);
  ioctl (fd, JSIOCGBUTTONS, &num_of_buttons);
  ioctl (fd, JSIOCGNAME(80), &jname);

  number_of_axis = num_of_axis;
  number_of_buttons = num_of_buttons;
  description = jname;

  read_joystick();
}


void CJoystick::read_joystick_depr()
{
  struct JS_DATA_TYPE js;
  if (read (fd, &js, JS_RETURN) != JS_RETURN)
     return; //error

  CJoystickAxisEvent *event = new CJoystickAxisEvent (evtJoystickAxis);

  if (js.y != 0)
     {
      event->axis = 0;
      event->value = js.y;
     }

  if (js.x != 0)
     {
      event->axis = 1;
      event->value = js.x;
     }

   QApplication::postEvent (receiver, reinterpret_cast<QEvent*>(event));
}


void CJoystick::read_joystick()
{
  if (! initialized)
     return;

  struct js_event e;

  while (read (fd, &e, sizeof(e)) > 0)
        {
         process_event (e);
        }


  if (errno != EAGAIN)
     {
      qDebug() << "Joystick read error";
      initialized = false;
     }
}


void CJoystick::process_event (js_event e)
{
 // qDebug() << "e.type" << e.type << endl;

  if (e.type & JS_EVENT_BUTTON)
     {
      CJoystickButtonEvent *event = new CJoystickButtonEvent (evtJoystickButton);

      event->button = e.number; //pressed button number
      event->pressed = e.value; //1 if pressed

      QApplication::postEvent (receiver, reinterpret_cast<QEvent*>(event));
     }
  else
  if (e.type & JS_EVENT_AXIS)
     {
      CJoystickAxisEvent *event = new CJoystickAxisEvent (evtJoystickAxis);

      event->axis = e.number;
      event->value = e.value;

      QApplication::postEvent(receiver, reinterpret_cast<QEvent*>(event));
    }
}

#endif
