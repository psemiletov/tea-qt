#include <string>
#include <iostream>
#include <sstream>
#include <list>
//#include <algorithm>
//#include <iterator>
#include <ctype.h>
//#include <locale>
#include <math.h>

#include "utils.h"


using namespace std;

class CItem
{

public:

  char op;
  double val;

  CItem (char a_op, double a_val);
};


CItem::CItem (char a_op, double a_val)
{
  op = a_op;
  val = a_val;
}


double calculate (string expression)
{
  list <CItem> items;

  lconv *l = localeconv();

  string sep_need = l->decimal_point;
  string sep_find;

  if (sep_need == ".")   
      sep_find = ",";
  else
      sep_find = ".";

  size_t position = expression.find (sep_find); 

  while (position != string::npos) 
        {
         expression.replace (position, 1, sep_need);
         position = expression.find (sep_find, position + 1);
        } 

//open braces

  size_t end_pos = expression.find (')');
  
//  cout << " end pos = " << end_pos << endl;    
    
  size_t start_pos = 0;
  if (end_pos != string::npos)
      do 
        {
         for (size_t i = end_pos; i-- > 0 ;)
             {
              if (expression[i] == '(')
                 {
                  start_pos = i;
                  break;
                 }
             }
     
         string s_temp_value = expression.substr (start_pos + 1, end_pos - start_pos - 1); 
 
        //cout << "start_pos = " << start_pos << " end pos = " << end_pos << endl;    
        //cout << "s_temp_value = " << s_temp_value << endl;    
 
        double f_temp_value = calculate (s_temp_value);
 
        std::ostringstream float_stream;
        float_stream << f_temp_value;
        string temp_s (float_stream.str());
     
        //cout << "temp_s = " << temp_s << endl;
 
        expression = expression.replace (start_pos + 1, end_pos - start_pos - 1, temp_s);
       }
     while (end_pos == string::npos);        
     
       
//parse expression to list:

  bool new_operator = false;

  string t_operand;
  char t_operator = '0';

  size_t stop_size = expression.length() - 1;
  
  for (size_t i = 0; i < expression.length(); i++)
      {
       char t = expression[i];

       if (isdigit (t) || t == '.' || t == ',')  
          t_operand += t;

       if (t == '+' || t == '-' ||
           t == '/' || t == '*' || 
           t == '^' || t == '%' || 
           i == stop_size)
          {
           new_operator = true;
           t_operator = t;

           if (i == stop_size)
              t_operator = '0';
          }

       if (new_operator)
          {
           //добавляем в items

           double f = atof (t_operand.c_str());

           items.push_back (CItem (t_operator, f));

           t_operand = "";
           t_operator = '0';
           new_operator = false;
          }
     }


  list<CItem>::iterator p = items.begin();

//степень и процент
  do {
      CItem current = *p;

      list<CItem>::iterator t = p;
      ++t;

      CItem next = *t;

      if (current.op == '^' || current.op == '%')
         {

          if (current.op == '^')
             {
              next.val = pow (current.val, next.val);             
              *t = next;
             }
          else    
              if (current.op == '%')
                 {
                  next.val = (float) get_value (current.val, next.val);             
                  *t = next; 
                 }

          p = items.erase (p);
            
          continue;
         }

      ++p;
     }
  while (p != items.end());


//умножаем и делим
  p = items.begin();

  do {
      CItem current = *p;

      list<CItem>::iterator t = p;
      ++t;

      CItem next = *t;

      if (current.op == '*' || current.op == '/')
         {

          if (current.op == '*')
             {
              next.val = current.val * next.val;             
              *t = next;
             }
          else    
              if (current.op == '/') 
                 {
                  next.val = current.val / next.val;             
                  *t = next; 
                 }

          p = items.erase (p);
            
          continue;
         }

      ++p;
     }
  while (p != items.end());

//складываем и вычитаем

  p = items.begin();

  do {
      CItem current = *p;

      list<CItem>::iterator t = p;
      ++t;

      CItem next = *t;

      if (current.op == '+' || current.op == '-')
         {
          if (current.op == '+')
             {
              next.val = current.val + next.val;
              *t = next;
             }
          else
              if (current.op == '-')
                 {
                  next.val = current.val - next.val;             
                  *t = next; 
                 }

           p = items.erase (p);
                
           continue;
          }

         ++p;
        }
  while (p != items.end());

  list<CItem>::iterator start = items.begin();
  CItem item = *start;

  return item.val;
}
