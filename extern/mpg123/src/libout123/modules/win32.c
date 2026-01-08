/*
    win32: audio output for Windows 32bit

    copyright ?-2013 by the mpg123 project - free software under the terms of the LGPL 2.1
    see COPYING and AUTHORS files in distribution or http://mpg123.org

    initially written (as it seems) by Tony Million
    rewrite of basic functionality for callback-less and properly ringbuffered operation by ravenexp
    Closing buffer playback fixed by David Wohlferd <limegreensocks (*) yahoo dod com>
*/

#include "../out123_int.h"
#include <windows.h>
#include "../../common/debug.h"

/*
    Buffer size and number of buffers in the playback ring
    NOTE: This particular num/size combination performs best under heavy
    loads for my system, however this may not be true for any hardware/OS out there.
    Generally, BUFFER_SIZE < 8k || NUM_BUFFERS > 16 || NUM_BUFFERS < 4 are not recommended.

    Introducing user-settable device buffer. We fix 8 buffers, scale the individual
    buffer size, rounded/truncated a bit to not be too odd. The old default of 64K
    buffers leads to 2.97 s with CD-DA. Quite excessive. We will probably reduce this
    soon.
*/
#define DEFAULT_DEVICE_BUFFER 0.25
// Buffers are multiples of this.
#define BUFFER_GRANULARITY 256
#define NUM_BUFFERS 8

static void wait_for_buffer(WAVEHDR* hdr, HANDLE hEvent);
static void drain_win32(out123_handle *ao);

/* Buffer ring queue state */
struct queue_state
{
    WAVEHDR buffer_headers[NUM_BUFFERS];
    size_t bufsize;
    /* The next buffer to be filled and put in playback */
    int next_buffer;
    /* Buffer playback completion event */
    HANDLE play_done_event;
    HWAVEOUT waveout;
};

static UINT dev_select(out123_handle *ao){
  UINT ret;
  if(ao->device) {
    sscanf(ao->device, "%u", &ret);
  } else {
    ret = WAVE_MAPPER;
  }
  return ret;
}

static int open_win32(out123_handle *ao)
{
    struct queue_state* state;
    int i;
    MMRESULT res;
    WAVEFORMATEX out_fmt;
    UINT dev_id;

    if(!ao) return -1;
    if(ao->rate == -1) return 0;

    /* only 8 and 16 supported */
    if(!(ao->format == MPG123_ENC_SIGNED_8 || ao->format == MPG123_ENC_SIGNED_16)) return -1;
    /* only mono and stereo supported */
    if(!(ao->channels == 1 || ao->channels == 2)) return -1;

    /* Allocate queue state struct for this device */
    state = calloc(1, sizeof(struct queue_state));
    if(!state) return -1;

    ao->userptr = state;

    state->play_done_event = CreateEvent(0,FALSE,FALSE,0);
    if(state->play_done_event == INVALID_HANDLE_VALUE) return -1;

    dev_id = dev_select(ao);

    out_fmt.wFormatTag = WAVE_FORMAT_PCM;
    out_fmt.wBitsPerSample = ao->format == MPG123_ENC_SIGNED_8 ? 8 : 16;
    out_fmt.nChannels = ao->channels;
    out_fmt.nSamplesPerSec = ao->rate;
    out_fmt.nBlockAlign = out_fmt.nChannels*out_fmt.wBitsPerSample/8;
    out_fmt.nAvgBytesPerSec = out_fmt.nBlockAlign*out_fmt.nSamplesPerSec;
    out_fmt.cbSize = 0;

    res = waveOutOpen(&state->waveout, dev_id, &out_fmt,
                      (DWORD_PTR)state->play_done_event, 0, CALLBACK_EVENT);

    switch(res)
    {
        case MMSYSERR_NOERROR:
            break;
        case MMSYSERR_ALLOCATED:
            ereturn(-1, "Audio output device is already allocated.");
        case MMSYSERR_NODRIVER:
            ereturn(-1, "No device driver is present.");
        case MMSYSERR_NOMEM:
            ereturn(-1, "Unable to allocate or lock memory.");
        case WAVERR_BADFORMAT:
            ereturn(-1, "Unsupported waveform-audio format.");
        default:
            ereturn(-1, "Unable to open wave output device.");
    }

    state->bufsize = (size_t)( (double)
      (ao->device_buffer > 0. ? ao->device_buffer : DEFAULT_DEVICE_BUFFER)
    *   out_fmt.nAvgBytesPerSec / NUM_BUFFERS / BUFFER_GRANULARITY);
    if(state->bufsize < 1)
        state->bufsize = 1;
    state->bufsize *= BUFFER_GRANULARITY;

    /* Reset event from the "device open" message */
    ResetEvent(state->play_done_event);
    /* Allocate playback buffers */
    for(i = 0; i < NUM_BUFFERS; i++)
    if(!(state->buffer_headers[i].lpData = (LPSTR)malloc(state->bufsize)))
    {
    ereturn(-1, "Out of memory for playback buffers.");
    }
    else
    {
        /* Tell waveOutPrepareHeader the maximum value of dwBufferLength
        we will ever send */
        state->buffer_headers[i].dwBufferLength = state->bufsize;
        state->buffer_headers[i].dwFlags = 0;
        res = waveOutPrepareHeader(state->waveout, &state->buffer_headers[i], sizeof(WAVEHDR));
        if(res != MMSYSERR_NOERROR) ereturn(-1, "Can't write to audio output device (prepare).");

        /* set the current size of the buffer to 0 */
        state->buffer_headers[i].dwBufferLength = 0;

        /* set flags to unprepared - must reset this to WHDR_PREPARED before calling write */
        state->buffer_headers[i].dwFlags = 0;
    }

    return 0;
}

static void wait_for_buffer(WAVEHDR* hdr, HANDLE hEvent)
{
    /* At this point there are several possible states:
    1) Empty or partial buffer (unqueued) - dwFlags == 0
    2) Buffer queued or being played - dwFlags == WHDR_PREPARED | WHDR_INQUEUE
    3) Buffer unqueued and finished being played - dwFlags == WHDR_PREPARED | WHDR_DONE
    4) Buffer removed from queue, but not yet marked as done - dwFlags == WHDR_PREPARED
    */

    /* Check buffer header and wait if it's being played. */
    if (hdr->dwFlags & WHDR_PREPARED)
    {
        while(!(hdr->dwFlags & WHDR_DONE))
        {
            /*debug1("waiting for buffer %i...", state->next_buffer);*/
            /* Waits for *a* buffer to finish.  May not be the one we
            want, so check again */
            WaitForSingleObject(hEvent, INFINITE);
        }
        hdr->dwFlags = 0;
        hdr->dwBufferLength = 0;
    }
}

static int get_formats_win32(out123_handle *ao)
{
    WAVEOUTCAPSA caps;
    int ret = 0;
    UINT dev_id = dev_select(ao);

    MMRESULT mr = waveOutGetDevCapsA(dev_id, &caps, sizeof(caps));
    if(mr != MMSYSERR_NOERROR)
      return 0; /* no formats? */

    if(ao->channels == 1) {
      switch(ao->rate) {
        case 44100:
          if(caps.dwFormats & WAVE_FORMAT_4M08)
            ret |= MPG123_ENC_SIGNED_8;
          if(caps.dwFormats & WAVE_FORMAT_4M16)
            ret |= MPG123_ENC_SIGNED_16;
          break;
        case 22050:
          if(caps.dwFormats & WAVE_FORMAT_2M08)
            ret |= MPG123_ENC_SIGNED_8;
          if(caps.dwFormats & WAVE_FORMAT_2M16)
            ret |= MPG123_ENC_SIGNED_16;
          break;
        case 11025:
          if(caps.dwFormats & WAVE_FORMAT_1M08)
            ret |= MPG123_ENC_SIGNED_8;
          if(caps.dwFormats & WAVE_FORMAT_1M16)
            ret |= MPG123_ENC_SIGNED_16;
          break;
       }
    }

    if(ao->channels == 2) {
      switch(ao->rate) {
        case 44100:
          if(caps.dwFormats & WAVE_FORMAT_4S08)
            ret |= MPG123_ENC_SIGNED_8;
          if(caps.dwFormats & WAVE_FORMAT_4S16)
            ret |= MPG123_ENC_SIGNED_16;
          break;
        case 22050:
          if(caps.dwFormats & WAVE_FORMAT_2S08)
            ret |= MPG123_ENC_SIGNED_8;
          if(caps.dwFormats & WAVE_FORMAT_2S16)
            ret |= MPG123_ENC_SIGNED_16;
          break;
        case 11025:
          if(caps.dwFormats & WAVE_FORMAT_2S08)
            ret |= MPG123_ENC_SIGNED_8;
          if(caps.dwFormats & WAVE_FORMAT_2S16)
            ret |= MPG123_ENC_SIGNED_16;
          break;
        }
     }
    return ret;
}

/* Stores audio data to the fixed size buffers and pushes them into the playback queue.
   I have one grief with that: The last piece of a track may not reach the output,
   only full buffers sent... But we don't get smooth audio otherwise. */
static int write_win32(out123_handle *ao, unsigned char *buf, int len)
{
    struct queue_state* state;
    MMRESULT res;
    WAVEHDR* hdr;

    int rest_len; /* Input data bytes left for next recursion. */
    int bufill;   /* Bytes we stuff into buffer now. */

    if(!ao || !ao->userptr) return -1;
    if(!buf || len <= 0) return 0;

    state = (struct queue_state*)ao->userptr;
    hdr = &state->buffer_headers[state->next_buffer];

    wait_for_buffer(hdr, state->play_done_event);

    /* Now see how much we want to stuff in and then stuff it in. */
    bufill = state->bufsize - hdr->dwBufferLength;
    if(len < bufill) bufill = len;

    rest_len = len - bufill;
    memcpy(hdr->lpData + hdr->dwBufferLength, buf, bufill);
    hdr->dwBufferLength += bufill;
    if(hdr->dwBufferLength == state->bufsize)
    { /* Send the buffer out when it's full. */
        hdr->dwFlags |= WHDR_PREPARED;

        res = waveOutWrite(state->waveout, hdr, sizeof(WAVEHDR));
        if(res != MMSYSERR_NOERROR) ereturn(-1, "Can't write to audio output device.");

        /* Cycle to the next buffer in the ring queue */
        state->next_buffer = (state->next_buffer + 1) % NUM_BUFFERS;
    }
    /* I'd like to propagate error codes or something... but there are no catchable surprises left.
       Anyhow: Here is the recursion that makes ravenexp happy;-) */
    if(rest_len && write_win32(ao, buf + bufill, rest_len) < 0) /* Write the rest. */
    return -1;
    else    
    return len;
}

     /* Flush means abort any pending playback */
static void flush_win32(out123_handle *ao)
{
    struct queue_state* state;
    WAVEHDR* hdr;

    if(!ao || !ao->userptr) return;
    state = (struct queue_state*)ao->userptr;

    /* Cancel any buffers in queue.  Ignore errors since we are void and
    can't return them anyway */
    waveOutReset(state->waveout);

    /* Discard any partial buffer */
    hdr = &state->buffer_headers[state->next_buffer];

    /* If WHDR_PREPARED is not set, this is (potentially) a partial buffer */
    if (!(hdr->dwFlags & WHDR_PREPARED))
    hdr->dwBufferLength = 0;

    /* Finish processing the buffers */
    drain_win32(ao);
}

/* output final buffer (if any) */
static void write_final_buffer(struct queue_state *state)
{
    WAVEHDR* hdr;
    hdr = &state->buffer_headers[state->next_buffer];
    if((!(hdr->dwFlags & WHDR_PREPARED)) && (hdr->dwBufferLength != 0))
    {
        hdr->dwFlags |= WHDR_PREPARED;
        /* ignore any errors */
        waveOutWrite(state->waveout, hdr, sizeof(WAVEHDR));

        /* Cycle to the next buffer in the ring queue */
        state->next_buffer = (state->next_buffer + 1) % NUM_BUFFERS;
    }
}

/* Note: I tried to fix this stuff without testing.
   There were some obvious errors in the code.
   Someone run this on a win32 machine! -- ThOr */
static void drain_win32(out123_handle *ao)
{
    int i, z;
    struct queue_state* state;

    if(!ao || !ao->userptr) return;
    state = (struct queue_state*)ao->userptr;

    /* output final buffer (if any) */
    write_final_buffer(state);

    /* I _think_ I understood how this should work. -- ThOr */
    z = state->next_buffer;
    for(i = 0; i < NUM_BUFFERS; i++)
    {
        wait_for_buffer(&state->buffer_headers[z], state->play_done_event);
        z = (z + 1) % NUM_BUFFERS;
    }
}

static int close_win32(out123_handle *ao)
{
    int i;
    struct queue_state* state;

    if(!ao || !ao->userptr) return -1;
    state = (struct queue_state*)ao->userptr;

    /* wait for all active buffers to complete */
    drain_win32(ao);
    CloseHandle(state->play_done_event);

    for(i = 0; i < NUM_BUFFERS; i++) 
    {
        state->buffer_headers[i].dwFlags |= WHDR_PREPARED;
        waveOutUnprepareHeader(state->waveout, &state->buffer_headers[i], sizeof(WAVEHDR));
        free(state->buffer_headers[i].lpData);
    }

    waveOutClose(state->waveout);
    free(ao->userptr);
    ao->userptr = 0;
    return 0;
}

static int enumerate_win32( out123_handle *ao, int (*store_device)(void *devlist
,	const char *name, const char *description), void *devlist )
{
  char id[10];
  WAVEOUTCAPSA caps;
  MMRESULT mr;
  UINT i, devices = waveOutGetNumDevs();
  for(i = 0; i < devices; i++){
    memset(id, 0, sizeof(id));
    memset(&caps, 0, sizeof(caps));
    mr = waveOutGetDevCapsA(i, &caps, sizeof(caps));
    if (mr != MMSYSERR_NOERROR) {
      switch(mr) {
        case MMSYSERR_BADDEVICEID:
          error("enumerate_win32: Specified device identifier is out of range.");
          break;
        case MMSYSERR_NODRIVER:
          error("enumerate_win32: No device driver is present.");
          break;
        case MMSYSERR_NOMEM:
          error("enumerate_win32: Unable to allocate or lock memory.");
          break;
        default:
          merror("enumerate_win32: Uknown error 0x%x.", mr);
        }
    }
    mdebug("waveOutGetDevCaps mr %x", mr);
    snprintf(id, sizeof(id) - 1, "%u", i);
    store_device(devlist, id, caps.szPname);
  }
  return 0;
}

static int init_win32(out123_handle* ao)
{
    if(!ao) return -1;

    /* Set callbacks */
    ao->open = open_win32;
    ao->flush = flush_win32;
    ao->write = write_win32;
    ao->get_formats = get_formats_win32;
    ao->close = close_win32;
    ao->enumerate = enumerate_win32;

    /* Success */
    return 0;
}

/* 
    Module information data structure
*/
mpg123_module_t mpg123_output_module_info = {
    /* api_version */    MPG123_MODULE_API_VERSION,
    /* name */            "win32",                        
    /* description */    "Audio output for Windows (winmm).",
    /* revision */        "$Rev:$",                        
    /* handle */        NULL,
    
    /* init_output */    init_win32,                        
};
