#ifdef SPEECH_ENABLE

#include <iostream>

#include "speech.h"


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


// Callback for Speech Dispatcher notifications
void cbk_end_of_speech (size_t msg_id, size_t client_id, SPDNotificationType type)
{

  if (type == SPD_EVENT_END)
       g_position++;
/*
  if (type == SPD_EVENT_CANCEL)
     {
      addstr("*SPD_CANCEL*");
     }
*/

//  sem_post(&g_semaphore);
}


void cbk_cancel_of_speech (size_t msg_id, size_t client_id, SPDNotificationType type)
{
  if (type == SPD_EVENT_CANCEL)
     {
     // addstr("*SPD_EVENT_CANCEL*");
     }
}


CSpeech::CSpeech()
{
  initialized = false;
  spd_connection = 0;

  locale_only = 1;
  current_voice_index = -1;

  g_state = SPCH_STATE_STOPPED;

  std::string temp_locale = setlocale(LC_ALL, "");
  locale_name = temp_locale.substr (0, 2);

//  std::cout << "LOC: " << locale_name << std::endl;
  //sem_init (&g_semaphore, 0, 0);
}


CSpeech::~CSpeech()
{
  if (initialized && spd_connection)
     spd_close (spd_connection);
}


void CSpeech::done()
{
   if (initialized && spd_connection)
      {
       spd_close (spd_connection);
       spd_connection = 0;
       initialized = false;
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

      spd_set_notification_on (spd_connection, SPD_END);

      char *s = NULL;

      s = spd_get_output_module (spd_connection);

      if (s)
         {
          output_module_name = s;
         // std::cout << "output_module_name: " << output_module_name  << std::endl;

          free (s);
         }

      s = spd_get_language (spd_connection);
      if (s)
         {
          language_name = s;
//          std::cout << "language_name: " << language_name  << std::endl;
          free (s);
         }

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
  if (! initialized)
      return;

  voices.clear();

  char  **voices_array = (char**)spd_list_synthesis_voices (spd_connection);

  //for > 0.15 API
    //char  **voices_array = (char**)spd_list_synthesis_voices2 (spd_connection, "ru", NULL);

  if (voices_array == NULL)
     return;

  std::string lang_name_short;

  int i = 0;
  while (voices_array[i] != NULL)
        {
         SPDVoice* voice = (SPDVoice*)voices_array[i]; // Приведение типа к SPDVoice*

         if (locale_only == 1)
            {
             std::string voice_name = voice->name;

             lang_name_short = voice->language;
             lang_name_short = lang_name_short.substr(0, 2);
             // std::cout << "v.language: " << v.language << std::endl;
             //std::cout << "language_name: " << language_name << std::endl;

             if (lang_name_short == language_name)
                voices.push_back (voice_name);
            }
         else
             {
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

 std::cout << "voices count: " << voices.size() << std::endl;

 free_spd_voices((SPDVoice**)voices_array);
}


void CSpeech::set_voice_by_index (int index)
{
  if (index == -1)
      return;

  if (index > voices.size() - 1)
     return;

  if (spd_set_synthesis_voice (spd_connection, voices[index].c_str()))
      std::cout << "ERRRRR" << std::endl;

 // std::cout << "spd_set_synthesis_voice: " <<  voices[index].name << std::endl;
}


#endif
