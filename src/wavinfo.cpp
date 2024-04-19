#include <QFile>
#include <QDataStream>

#include <math.h>

#include "wavinfo.h"
#include "utils.h"


//no range check, use in wavinfo only
bool str_check (char *s1, char *s2, int size)
{
  for (int i = 0; i < size; i++)
       if (s1[i] != s2[i])
          return false;

  return true;
}


bool CWavReader::get_info (const QString &fname)
{
  QFile fl (fname);

  if (! fl.open(QIODevice::ReadOnly))
     return false;

  QDataStream ds (&fl);

  t_wav_chunk_hdr hdr;

  while (! ds.atEnd())
        {
         ds.readRawData ((char *)&hdr, sizeof (hdr));

         if (str_check (hdr.chunk_id, (char*)"RIFF", 4))
            {
             char riff_type[4];
             ds.readRawData ((char *)&riff_type, sizeof (riff_type));
           }
         else
         if (str_check (hdr.chunk_id, (char*)"fmt ", 4))
            {
             ds.readRawData ((char *)&wav_chunk_fmt, sizeof (wav_chunk_fmt));
             if (wav_chunk_fmt.bits_per_sample != 16)
                return false;
            }
         else
         if (str_check (hdr.chunk_id, (char*)"data", 4))
            {
             int nsamples = hdr.chunk_size / (wav_chunk_fmt.bits_per_sample / 8);

             double sqr_sum = 0.0;

             qint16 *ch_both = new qint16[nsamples];
             ds.readRawData ((char *)ch_both, hdr.chunk_size);
             for (int i = 0; i < nsamples; i++)
                  sqr_sum += (ch_both[i] * ch_both[i]);

             double srms = sqrt (sqr_sum / nsamples);
             rms = 20 * log10 (srms / 32767);

             delete [] ch_both;
             return true;
            }
        else
            ds.skipRawData (hdr.chunk_size);
       }

  return false;
}
