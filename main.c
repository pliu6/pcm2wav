#include "stdio.h"

typedef unsigned char ID[4];

typedef struct
{
  ID             chunkID;  /* {'f', 'm', 't', ' '} */
  long           chunkSize;

  short          wFormatTag;
  unsigned short wChannels;
  unsigned long  dwSamplesPerSec;
  unsigned long  dwAvgBytesPerSec;
  unsigned short wBlockAlign;
  unsigned short wBitsPerSample;
  /* Note: there may be additional fields here, 
     depending upon wFormatTag. */
} FormatChunk;

typedef struct
{
  ID             chunkID;  /* {'d', 'a', 't', 'a'}  */
  long           chunkSize;
  unsigned char  waveformData[];
} DataChunk;

void usage(char *command)
{
  printf("usage:\n"
         "\t%s pcmfile wavfile channel samplerate bitspersample\n", command);
}

int main(int argc, char *argv[])
{
  FILE *pcmfile, *wavfile;
  long  pcmfile_size, chunk_size;
  FormatChunk formatchunk;
  DataChunk   datachunk;
  int i, read_len;
  char buf[1024];
  short tmp;

  if (argc != 6) {
    usage(argv[0]);
    return 1;
  }
 
  pcmfile = fopen(argv[1], "rb");
  if (pcmfile == NULL) {
    printf("!Error: Can't open pcmfile.\n");
    return 1;
  }
  fseek(pcmfile, 0, SEEK_END);
  pcmfile_size = ftell(pcmfile);
  fseek(pcmfile, 0, SEEK_SET);

  wavfile = fopen(argv[2], "wb");
  if (wavfile == NULL) {
    printf("!Error: Can't create wavfile.\n");
    return 1;
  }

  fwrite("RIFF", 1, 4, wavfile);
  fwrite("xxxx", 1, 4, wavfile);  //reserved for the total chunk size
  fwrite("WAVE", 1, 4, wavfile);

  formatchunk.chunkID[0] = 'f';
  formatchunk.chunkID[1] = 'm';
  formatchunk.chunkID[2] = 't';
  formatchunk.chunkID[3] = ' ';
  formatchunk.chunkSize  = sizeof(FormatChunk) - sizeof(ID) - sizeof(long);
  formatchunk.wFormatTag = 1;   /* uncompressed */
  formatchunk.wChannels = atoi(argv[3]);
  formatchunk.dwSamplesPerSec = atoi(argv[4]);
  formatchunk.wBitsPerSample = atoi(argv[5]);
  formatchunk.wBlockAlign = formatchunk.wChannels * (formatchunk.wBitsPerSample >> 3);
  formatchunk.dwAvgBytesPerSec =  formatchunk.wBlockAlign * formatchunk.dwSamplesPerSec;
  fwrite(&formatchunk, 1, sizeof(formatchunk), wavfile);

  datachunk.chunkID[0] = 'd';
  datachunk.chunkID[1] = 'a';
  datachunk.chunkID[2] = 't';
  datachunk.chunkID[3] = 'a';
  datachunk.chunkSize = pcmfile_size;
  fwrite(&datachunk, 1, sizeof(ID)+sizeof(long), wavfile);

  while((read_len = fread(buf, 1, sizeof(buf), pcmfile)) != 0) {
    /* revert the endiean */
#if 0
    for (i=0; i<read_len; i+=2) {
      tmp = buf[i];
      buf[i] = buf[i+1];
      buf[i+1] = tmp;
    }
#endif
    fwrite(buf, 1, read_len, wavfile);
  }

  fseek(wavfile, 4, SEEK_SET);
  chunk_size = 4 + (8 + formatchunk.chunkSize) + (8 + datachunk.chunkSize);
  fwrite(&chunk_size, 1, 4, wavfile);

  fclose(pcmfile);
  fclose(wavfile);
}
