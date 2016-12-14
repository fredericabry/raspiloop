#ifndef PARAMETERS
#define PARAMETERS

#endif // PARAMETERS



//soundcard parameters
#define RATE 48000




//capture parameters
#define CAPTURE_CHANNEL_WIDTH 256 //number of elements in a a frame for ONE channel
#define CAPTURE_HW_BUFFER_SIZE 3*CAPTURE_CHANNEL_WIDTH //alsa capture buffer size for ONE channel
#define RINGBUFSIZE_CAPTURE RATE*1 //size of the capture ringbuffer : 1 seconds
#define NFILE_CAPTURE 256*2// CAPTURE_CHANNEL_WIDTH//file writting buffer size
#define CAPTURE_READBUF_SLEEP (unsigned long)5000 //usec
#define CAPTURE_WRITEFILE_SLEEP (unsigned long)4000 //us
#define CAPTURE_AVAIL_MIN       5*CAPTURE_HW_BUFFER_SIZE/10 //when the device buffer data is bigger than this limit, an interrupt is issued
#define CAPTURE_LOOP_BUFSIZE NFILE_CAPTURE//44100 //number of elements of each capture loop ring buffer



//playback parameters
#define PLAYBACK_CHANNEL_WIDTH 256 //number of elements in a a frame for ONE channel
#define NFILE_PLAYBACK PLAYBACK_CHANNEL_WIDTH*10 //file reading buffer size
#define PLAYBACK_READBUF_SLEEP 5000 //usec
#define RINGBUFSIZE_PLAYBACK 2000 //size of the playback ringbuffer
#define PLAYBACK_MIX_SLEEP 5 //ms

#define PLAYBACK_HW_BUFFER_SIZE             4*PLAYBACK_CHANNEL_WIDTH  //alsa playback buffer size for ONE channel
#define PLAYBACK_AVAIL_MIN                   2*PLAYBACK_HW_BUFFER_SIZE/10  //when the device buffer data is smaller than this limit, an interrupt is issued
#define PLAYBACK_BUFSIZE    3*PLAYBACK_CHANNEL_WIDTH
#define CAPTURE_READFILE_SLEEP (unsigned long)4000 //us



//#define DIRECTORY "/home/pi/usb/"
//#define DIRECTORY "/home/pi/usb1/"
#define DIRECTORY ""


#define SOUND_FORMAT SND_PCM_FORMAT_S16_LE



