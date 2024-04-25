#ifndef SPEECH_H
#define SPEECH_H

#ifdef SPEECH_ENABLE

/*

 usr/include/speech-dispatcher/libspeechd.h
usr/include/speech-dispatcher/libspeechd_version.h
usr/include/speech-dispatcher/spd_audio_plugin.h
usr/include/speech-dispatcher/speechd_defines.h
usr/include/speech-dispatcher/speechd_types.h


 */


#include <libspeechd.h>
#include <semaphore.h>
#include <signal.h>

#include <vector>
#include <string>


#define SPCH_STATE_STOPPED 0
#define SPCH_STATE_SAYING 1
#define SPCH_STATE_PAUSED 2
#define SPCH_STATE_NEXT 3

/*
class CVoice
{
public:

  std::string name;
  std::string language;


};
*/

class CSpeech
{
  public:

  int locale_only;

  //int state;

  SPDConnection *spd_connection;

  std::vector <std::string> voices; //module-specific

//  std::vector <CVoice> voices; //module-specific

  std::string locale_name;

  bool initialized;

  std::string output_module_name;
  std::string language_name;

  int current_voice_index;

  CSpeech();
  ~CSpeech();

  void init (const char* client_name);
  void say (const char* text);

  void done();


  void stop();
  void pause();
  void play();
  void resume();
  void cancel();

  void get_voices (int locale_only = 1); //fills voices vector
  void set_voice_by_index (int index);

};


//sig_atomic_t g_signal;

//

namespace
{
//  volatile std::sig_atomic_t g_signal;
    volatile sig_atomic_t g_signal;

//  volatile int sockfd;
}


void f_signal_handler (int signal);

#endif


#endif
