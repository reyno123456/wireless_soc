#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "layer3.h"
#include "debuglog.h"
#include "test_mp3.h"
#include "ff.h"
#include "ff_gen_drv.h"

#define _USE_PCM_FILE_

static short int buffer[2 * SHINE_MAX_SAMPLES] = {0};

/* RISC OS specifics */
#define WAVE  0xfb1      /* Wave filetype */
#define DATA  0xffd      /* Data filetype */

#define MODE_MONO 3

FIL g_inShineFile;

typedef struct 
{
    FIL *file;
    int  has_seek;
    int  channels;
    long length;
	long duration;
} wave_t;

typedef struct {
  char id[4];
  uint32_t length;
} riff_chunk_header_t;

typedef struct {
  riff_chunk_header_t header;
  char wave[4];
} wave_chunk_t;

typedef struct {
  riff_chunk_header_t header;
  uint16_t format;       /* MS PCM = 1 */
  uint16_t channels;     /* channels, mono = 1, stereo = 2 */
  uint32_t sample_rate;  /* samples per second = 44100 */
  uint32_t byte_rate;    /* bytes per second = samp_rate * byte_samp = 176400 */
  uint16_t frame_size;   /* block align (bytes per sample) = channels * bits_per_sample / 8 = 4 */
  uint16_t depth;        /* bits per sample = 16 for MS PCM (format specific) */
} fmt_chunk_t;

static void wave_seek(FIL *file, int has_seek, uint32_t bytes) 
{
  f_lseek(file, bytes);
}

static unsigned char wave_get_chunk_header(FIL *file, int has_seek, const char id[4], riff_chunk_header_t *header)
{
  unsigned char found = 0;
  uint32_t chunk_length;
  uint32_t chunk_read;

  while(!found) {
    if (f_read(file, header, sizeof(riff_chunk_header_t), (UINT*)&chunk_read) != FR_OK) {
      if (f_eof(file))
        return 0;
      else
        dlog_info("Read error\n");
    }

    /* chunks must be word-aligned, chunk data doesn't need to */
    chunk_length = header->length + header->length % 2;

    if (strncmp(header->id, id, 4) == 0)
      return 1;
    
    // wave_seek(file, has_seek, chunk_length);
    f_lseek(file, chunk_length);
  }

  return 1;
}

static void wave_close(wave_t *wave)
{
  f_close(wave->file);
}

/*
 * wave_open:
 * ----------
 * Opens and verifies the header of the Input Wave file. The file pointer is
 * left pointing to the start of the samples.
 */
static unsigned char wave_open(const char *fname, wave_t *wave, shine_config_t *config, FIL *file)
{
  static char *channel_mappings[] = { NULL, "mono", "stereo" };
  wave_chunk_t wave_chunk;
  fmt_chunk_t fmt_chunk;
  riff_chunk_header_t data_chunk;
  uint32_t fmt_data, fmt_length;
  int chunk_read;

  if (f_open(file, (const TCHAR*)fname, FA_READ) != FR_OK)
  {
    dlog_info("f_open in error!\n");
    return 1;
  }
  wave->has_seek = 1;
  wave->file = file;

  if (!wave->file)
  {
    dlog_info("Unable to open file");
    return 1;
  }

  f_read(file, &wave_chunk, sizeof(wave_chunk), (UINT*)&chunk_read);
  if (chunk_read != sizeof(wave_chunk))
  {    
    dlog_info("Invalid header\n");
  }

  if (strncmp(wave_chunk.header.id, "RIFF", 4) != 0)
  {
    dlog_info("Not a MS-RIFF file");
  }

  if (strncmp(wave_chunk.wave, "WAVE", 4) != 0)
  {
    dlog_info("Not a WAVE audio file");
  }

  /* Check the fmt chunk */
  if (!wave_get_chunk_header(wave->file, wave->has_seek, "fmt ", (riff_chunk_header_t *)&fmt_chunk))
  {
    dlog_info("WAVE fmt chunk not found");
  }

  fmt_data = sizeof(fmt_chunk_t) - sizeof(riff_chunk_header_t);

  f_read(file, &fmt_chunk.format, fmt_data, (UINT*)&chunk_read);
  if (chunk_read != fmt_data)
  {
    dlog_info("Read error\n");
  }

  if (fmt_chunk.format != 1)
  {
    dlog_info("Unknown WAVE format");
  }

  if (fmt_chunk.channels > 2)
  {
    dlog_info("More than 2 channels");
  }

  if (fmt_chunk.depth != 16)
  {
    dlog_info("Unsupported PCM bit depth");
  }

  // Skip remaining data.
  fmt_length = fmt_chunk.header.length + fmt_chunk.header.length % 2;
  if (fmt_length > fmt_data)
  {
    wave_seek(file, wave->has_seek, fmt_length - fmt_data);
  }

  /* Position the file pointer at the data chunk */
  if (!wave_get_chunk_header(file, wave->has_seek, "data", &data_chunk))
  {
    dlog_info("WAVE data chunk not found");
  }

  config->wave.channels   = fmt_chunk.channels;
  config->wave.samplerate = fmt_chunk.sample_rate;

  wave->channels = fmt_chunk.channels;
  wave->length   = data_chunk.length;
  wave->duration = data_chunk.length / fmt_chunk.byte_rate;

  dlog_info("%s, %s %ldHz %ldbit, duration: %02ld:%02ld:%02ld\n",
      "WAVE PCM Data", channel_mappings[fmt_chunk.channels], (long)fmt_chunk.sample_rate, (long)fmt_chunk.depth,
      (long)wave->duration / 3600, (long)(wave->duration / 60) % 60, (long)wave->duration % 60);

  return 0;
}

/*
 * read_samples:
 * -------------
 */
/* TODO: respect data chunk length */
static int read_samples(int16_t *sample_buffer, int frame_size, FIL *file)
{
  int samples_read = 0;

  if(f_read(file, sample_buffer,sizeof(short int) * frame_size, (UINT*)&samples_read) != FR_OK)
  {
    dlog_info("read samples error\n");
    return 0;
  }

  if(samples_read < frame_size && samples_read > 0) /* Pad sample with zero's */
    while(samples_read<frame_size) sample_buffer[samples_read++] = 0;

#ifdef SHINE_BIG_ENDIAN
  swap_buffer(sample_buffer, samples_read);
#endif

  return samples_read;
}

/*
 * wave_get:
 * ---------
 * Expects an interleaved 16bit pcm stream from read_samples, which it
 * reads into the given buffer.
 */
static int wave_get(int16_t *buffer, wave_t *wave, int samp_per_pass)
{
  return read_samples(buffer,samp_per_pass*wave->channels, wave->file);
}

static int test_pcm(void)
{
    shine_config_t config;
    int read, write, written;
    unsigned char  *data;
    FRESULT f_ret;
    FIL outFile;
    FIL inFile;
    shine_t        s;

    if(f_open(&outFile, (const TCHAR*)("0:filep.mp3"), FA_CREATE_ALWAYS | FA_WRITE) != FR_OK)
    {
        dlog_info("open outfile fail!\n");
        return 1;
    }

    /* Set the default MPEG encoding paramters - basically init the struct */
    shine_set_config_mpeg_defaults(&(config.mpeg));

    if (f_open(&inFile, (const TCHAR*)("0:file.pcm"), FA_READ) != FR_OK)
    {
        dlog_info("f_open infile error!\n");
        return 1;
    }

    config.wave.channels   = 2;
    config.wave.samplerate = 48000;
    config.mpeg.mode = STEREO;

    s = shine_initialise(&config);
    int samples_per_pass = shine_samples_per_pass(s);
    unsigned int frame_size = sizeof(short int) * samples_per_pass * config.wave.channels;

    dlog_info("read frame_size %d\n", frame_size);

    int samples_read = 0;
    do 
    {
        if(f_read(&inFile, buffer, frame_size, (UINT*)&samples_read) != FR_OK)
        {
            dlog_info("read samples error\n");
            return 0;
        }
        
        if(samples_read < frame_size && samples_read > 0) /* Pad sample with zero's */
        {
            while(samples_read<frame_size) buffer[samples_read++] = 0;
        }

        data = shine_encode_buffer_interleaved(s, buffer, &write);

        if (f_write(&outFile, data, sizeof(unsigned char) * write, (UINT*)&written) != FR_OK )
        {
             dlog_info("shineenc: write error\n");
             return 1;
        }
    }
    while (samples_read != 0);

    /* Flush and write remaining data. */
    data = shine_flush(s, &write);
    f_write(&outFile, data, sizeof(unsigned char) * write, (UINT*)&write);

    shine_close(s);
    f_close(&inFile);
    f_close(&outFile);
}

static int test_wav(void)
{
    wave_t         wave;
    shine_config_t config;
    int read, write, written;
    unsigned char  *data;
    FRESULT f_ret;
    FIL outFile;
    FIL inFile;
    shine_t s;

    if(f_open(&outFile, (const TCHAR*)("0:filew.mp3"), FA_CREATE_ALWAYS | FA_WRITE) != FR_OK)
    {
        dlog_info("open outfile fail!\n");
        return 1;
    }

    /* Set the default MPEG encoding paramters - basically init the struct */
    shine_set_config_mpeg_defaults(&(config.mpeg));

    /* Open the input file and fill the config shine_wave_t header */
    if (wave_open("0:file.wav", &wave, &config, &inFile) != 0)
    {
        dlog_info("Could not open WAVE file\n");
        return 1;
    }

    /* Set to stereo mode if wave data is stereo, mono otherwise. */
    if (config.wave.channels > 1)
        config.mpeg.mode = STEREO;
    else
        config.mpeg.mode = MONO;

    /* Initiate encoder */
    s = shine_initialise(&config);

    int samples_per_pass = shine_samples_per_pass(s);

    /* All the magic happens here */
    while (wave_get(buffer, &wave, samples_per_pass)) 
    {
        data = shine_encode_buffer_interleaved(s, buffer, &write);

        if ( f_write(&outFile,data, sizeof(unsigned char) * write, (UINT*)&written) != FR_OK )
        {
           dlog_info("shineenc: write error\n");
           return 1;
        }
    }

    /* Flush and write remaining data. */
    data = shine_flush(s, &write);
    f_write(&outFile, data, sizeof(unsigned char) * write, (UINT*)&write);

    /* Close encoder. */
    shine_close(s);
    f_close(&outFile);
    wave_close(&wave);
}

static int test_shine(unsigned char uc_option)
{
    if (uc_option == 0)
    {
        test_pcm();
    }

    if (uc_option == 1)
    {
        test_wav();
    } 

    dlog_info("finish\n");
}

uint32_t command_TestMP3Encoder(char* uc_option)
{
    test_shine(strtoul(uc_option, NULL, 0));
}

