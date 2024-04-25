#ifdef SPEECH_ENABLE


#include <iostream>
//#include <ncurses.h>

#include "speech.h"


//sem_t g_semaphore;


//bool saying;


int g_state;
int g_position;



void f_signal_handler (int signal)
{
  std::cout <<  "f_signal_handler " << std::endl;

  g_signal = signal;

//  shutdown (sockfd, 2);
  //close (sockfd);

  std::cout << "Exiting by the signal" << std::endl;
}


/* Callback for Speech Dispatcher notifications */
void cbk_end_of_speech (size_t msg_id, size_t client_id, SPDNotificationType type)
{

 //if (type == SPD_EVENT_END)
     //  g_position++;

//  sem_post(&g_semaphore);

 if (type == SPD_EVENT_END)
    g_state = SPCH_STATE_STOPPED;


}


void cbk_cancel_of_speech (size_t msg_id, size_t client_id, SPDNotificationType type)
{
//  addstr("*cbk_cancel_of_speech*");

  if (type == SPD_EVENT_CANCEL)
     {
     // addstr("*SPD_EVENT_CANCEL*");
     }

}


CSpeech::CSpeech()
{
  initialized = false;
  spd_connection = 0;
  locale_only = 0;


  current_voice_index = -1;

  //g_state = SPCH_STATE_STOPPED;

  g_state = SPCH_STATE_STOPPED;

  char* locval = setlocale(LC_ALL, "");

  std::string temp_locale;

  if (locval)
    {
     temp_locale = locval;
     locale_name = temp_locale.substr (0, 2);
    }
  else
      locale_name = "en";
//  std::cout << "LOC: " << locale_name << std::endl;


  //sem_init (&g_semaphore, 0, 0);

}


CSpeech::~CSpeech()
{
  if (initialized)
     if (spd_connection)
         spd_close (spd_connection);

}


void CSpeech::done()
{
  if (initialized)
     if (spd_connection)
        {
         spd_close (spd_connection);
         spd_connection = 0;
         initialized = false;
         //sem_close (&g_semaphore);
         //sem_destroy (&g_semaphore);
        }

}


void CSpeech::init (const char* client_name)
{


  spd_connection = spd_open (client_name,
                             "main",
                             NULL, //username
                             SPD_MODE_THREADED);

  if (spd_connection)
     {
      initialized = true;

      spd_set_language (spd_connection, locale_name.c_str());

      spd_connection->callback_end = cbk_end_of_speech;
     // spd_connection->callback_cancel = cbk_cancel_of_speech;

      spd_set_notification_on(spd_connection, SPD_END);

      char *s = NULL;

      s = spd_get_output_module (spd_connection);

      if (s)
        {
         output_module_name = s;
         free (s);
        }

      s = spd_get_language (spd_connection);
      if (s)
         {
          language_name = s;
          free (s);
         }

    //   spd_set_language(spd_connection, "ru");


    //  get_voices();

      current_voice_index = 0;
     }
}


void CSpeech::say (const char* text)
{
  if (! initialized)
      return;

  g_state = SPCH_STATE_SAYING;

  int result = spd_say (spd_connection, SPD_TEXT, text);

  if (result == -1)
     std::cout << "say error!" << std::endl;

   //А ЕСЛИ НЕ ЖДАТЬ?
  //sem_wait (&g_semaphore);
}


void CSpeech::stop()
{
  if (! initialized)
      return;

   spd_stop (spd_connection);
   g_position = 0;
   g_state = SPCH_STATE_STOPPED;
}


void CSpeech::pause()
{
  if (! initialized)
     return;

  if (spd_pause (spd_connection) != -1)
     g_state = SPCH_STATE_PAUSED;
}


void CSpeech::play()
{
   if (! initialized)
      return;

  g_state = SPCH_STATE_SAYING;
  g_position = 0;
}



void CSpeech::resume()
{
  if (! initialized)
     return;

  if (! g_state == SPCH_STATE_PAUSED)
      return;

  if (spd_resume (spd_connection) != -1)
      g_state = SPCH_STATE_SAYING;
}



void CSpeech::cancel()
{
  if (! initialized)
      return;

  spd_cancel (spd_connection);
//  spd_stop (spd_connection);

  g_state = SPCH_STATE_STOPPED;

}


void CSpeech::get_voices (int locale_only)
{
//  char  **voices_array = spd_list_synthesis_voices2 (spd_connection,
  //                                                    setlocale(LC_ALL, NULL),
    //

  if (! initialized)
      return;

  voices.clear();

  char  **voices_array = (char**)spd_list_synthesis_voices (spd_connection);

  //for > 0.15 API
    //char  **voices_array = (char**)spd_list_synthesis_voices2 (spd_connection, "ru", NULL);


  if (voices_array == NULL)
     return;


  std::string lang_name_short;

  //SPDVoice  **voice = arr_voices;

  int i = 0;
  while (voices_array[i] != NULL)
       {
        SPDVoice* voice = (SPDVoice*)voices_array[i]; // Приведение типа к SPDVoice*

    /*   CVoice v;
           v.name = voice->name;
           v.language = voice->language;

           voices.push_back (v);
*/

       if (locale_only == 1)
          {
           /*CVoice v;
           v.name = voice->name;

           lang_name_short = voice->language;
           lang_name_short = lang_name_short.substr(0, 2);
           v.language = lang_name_short;

           if (v.language == language_name)
              voices.push_back (v);*/

           std::string voice_name = voice->name;

           lang_name_short = voice->language;
           lang_name_short = lang_name_short.substr(0, 2);

           if (lang_name_short == language_name)
               voices.push_back (voice_name);

          }
       else
          {
           /*CVoice v;
           v.name = voice->name;

           lang_name_short = voice->language;
           lang_name_short = lang_name_short.substr(0, 2);
           v.language = lang_name_short;

           voices.push_back (v);*/
           std::string voice_name = voice->name;
            voices.push_back (voice_name);

          }


       current_voice_index = 0;


    // Вывод информации о голосе
/*
    printf("Voice %d:\n", i + 1);
    printf("Name: %s\n", voice->name);
    printf("Language: %s\n", voice->language);
    printf("Variant: %s\n", voice->variant);
    printf("\n");
*/
    ++i;
}


 //std::cout << "voices count: " << voices.size() << std::endl;

 free_spd_voices((SPDVoice**)voices_array);

}


void CSpeech::set_voice_by_index (int index)
{
  if (index == -1)
      return;

  if (index > voices.size() - 1)
     return;

//  if (spd_set_synthesis_voice (spd_connection, voices[index].name.c_str()))
  //    std::cout << "ERRRRR" << std::endl;
  if (spd_set_synthesis_voice (spd_connection, voices[index].c_str()))
      std::cout << "ERRRRR" << std::endl;


 // std::cout << "spd_set_synthesis_voice: " <<  voices[index].name << std::endl;
}



// int spd_set_synthesis_voice(SPDConnection* connection, const char* voice_name);
#endif