#ifndef WAVINFO_H
#define WAVINFO_H

#include <QObject>
#include <QtGlobal>
#include <QString>


typedef struct {
                char chunk_id[4];
                quint32 chunk_size;
               } t_wav_chunk_hdr;


typedef struct {
                quint16 format;      // = 1 (PCM/uncompressed)
                quint16 num_channels; // mono = 1, stereo = 2, etc
                quint32 sample_rate;  // 8000, 44100, 48000 etc
                quint32 byte_rate;    // SampleRate * NumChannels * BitsPerSample/8
                quint16 block_align;  // NumChannels * BitsPerSample / 8
                quint16 bits_per_sample; //8, 16 etc
               } t_wav_chunk_fmt;


class CWavReader: public QObject
{
public:

  t_wav_chunk_fmt wav_chunk_fmt;
  double rms;
  bool get_info (const QString &fname);
};


#endif // WAVINFO_H
