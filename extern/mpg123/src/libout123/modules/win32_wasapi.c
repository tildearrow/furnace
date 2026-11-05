/*
	win32_wasapi: audio output for Windows wasapi exclusive mode audio

	copyright ?-2013 by the mpg123 project - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org

	based on win32.c
*/
#if  defined(_WIN32_WINNT)
# if _WIN32_WINNT < 0x0601
#  error "wrong _WIN32_WINNT value"
# endif
#else
# define _WIN32_WINNT 0x0601
#endif
#define COBJMACROS 1
#include "../out123_int.h"
#include <inttypes.h>
#include <initguid.h>
#include <audioclient.h>
#include <mmdeviceapi.h>
#include <avrt.h>
#include <functiondiscoverykeys_devpkey.h>
#include "../../common/debug.h"

#ifdef _MSC_VER

/* When compiling C code with MSVC it is only possible to declare, but not
   define the WASAPI interface GUIDs using MS headers. So we define them 
   ourselves. */

#ifndef GUID_SECT
#define GUID_SECT
#endif

#define __DEFINE_GUID(n,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) static const GUID n GUID_SECT = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}
#define __DEFINE_IID(n,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) static const IID n GUID_SECT = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}
#define __DEFINE_CLSID(n,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) static const CLSID n GUID_SECT = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}
#define __DEFINE_PROPERTYKEY(n,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8,pid) static const PROPERTYKEY n GUID_SECT = {{l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}},pid}
#define MPG123_DEFINE_CLSID(className, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    __DEFINE_CLSID(mpg123_CLSID_##className, 0x##l, 0x##w1, 0x##w2, 0x##b1, 0x##b2, 0x##b3, 0x##b4, 0x##b5, 0x##b6, 0x##b7, 0x##b8)
#define MPG123_DEFINE_IID(interfaceName, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    __DEFINE_IID(mpg123_IID_##interfaceName, 0x##l, 0x##w1, 0x##w2, 0x##b1, 0x##b2, 0x##b3, 0x##b4, 0x##b5, 0x##b6, 0x##b7, 0x##b8)
#define MPG123_DEFINE_PROPERTYKEY(interfaceName, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8, pid) \
    __DEFINE_PROPERTYKEY(mpg123_PKEY_##interfaceName, 0x##l, 0x##w1, 0x##w2, 0x##b1, 0x##b2, 0x##b3, 0x##b4, 0x##b5, 0x##b6, 0x##b7, 0x##b8, pid)

// "1CB9AD4C-DBFA-4c32-B178-C2F568A703B2"
MPG123_DEFINE_IID(IAudioClient, 1cb9ad4c, dbfa, 4c32, b1, 78, c2, f5, 68, a7, 03, b2);
// "A95664D2-9614-4F35-A746-DE8DB63617E6"
MPG123_DEFINE_IID(IMMDeviceEnumerator, a95664d2, 9614, 4f35, a7, 46, de, 8d, b6, 36, 17, e6);
// "BCDE0395-E52F-467C-8E3D-C4579291692E"
MPG123_DEFINE_CLSID(IMMDeviceEnumerator, bcde0395, e52f, 467c, 8e, 3d, c4, 57, 92, 91, 69, 2e);
// "F294ACFC-3146-4483-A7BF-ADDCA7C260E2"
MPG123_DEFINE_IID(IAudioRenderClient, f294acfc, 3146, 4483, a7, bf, ad, dc, a7, c2, 60, e2);
MPG123_DEFINE_PROPERTYKEY(Device_FriendlyName, a45c254e, df1c, 4efd, 80, 20, 67, d1, 46, a8, 50, e0, 14);
#else
#define mpg123_IID_IAudioClient IID_IAudioClient
#define mpg123_IID_IMMDeviceEnumerator IID_IMMDeviceEnumerator
#define mpg123_CLSID_IMMDeviceEnumerator CLSID_MMDeviceEnumerator
#define mpg123_IID_IAudioRenderClient IID_IAudioRenderClient
#define mpg123_PKEY_Device_FriendlyName PKEY_Device_FriendlyName
#endif

/* Push mode does not work right yet, noisy audio, probably something to do with timing and buffers */
#define MOD_STRING "Experimental Audio output for Windows"
#define BUFFER_TIME 20000000.0

static int init_win32(out123_handle* ao);
static void flush_win32(out123_handle *ao);
/*
	Module information data structure
*/
mpg123_module_t mpg123_output_module_info = {
	/* api_version */	MPG123_MODULE_API_VERSION,
	/* name */			"win32_wasapi",
	/* description */	MOD_STRING,
	/* revision */		"$Rev:$",
	/* handle */		NULL,
	/* init_output */	init_win32,
};

// REFERENCE_TIME time units per second and per millisecond
#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { goto Exit; }

/* todo: move into handle struct */
typedef struct _wasapi_state_struct {
  IMMDeviceEnumerator *pEnumerator;
  IMMDevice *pDevice;
  IAudioClient *pAudioClient;
  IAudioRenderClient *pRenderClient;
  BYTE *pData;
  UINT32 bufferFrameCount, numFramesAvailable;
  REFERENCE_TIME hnsRequestedDuration;
  HANDLE hEvent;
  HANDLE hTask;
  size_t pData_off;
  DWORD taskIndex;
  char is_playing;
  DWORD framesize;
  DWORD renderMode;
  char is_EventMode;
} wasapi_state_struct;

/* setup endpoints */
static int open_win32(out123_handle *ao){
  HRESULT hr = 0;
  wchar_t *device = NULL;
  int devlen;
  wasapi_state_struct *state;

  debug1("%s",__FUNCTION__);
  if(!ao || ao->userptr) return -1; /* userptr should really be null */
  state = calloc(sizeof(*state),1);

  if(!state) return -1;
  state->hnsRequestedDuration = REFTIMES_PER_SEC;
  ao->userptr = (void *)state;

  state->renderMode = AUDCLNT_SHAREMODE_SHARED;
  state->is_EventMode = 1;

  hr = CoCreateInstance(&mpg123_CLSID_IMMDeviceEnumerator,NULL,CLSCTX_ALL, &mpg123_IID_IMMDeviceEnumerator,(void**)&state->pEnumerator);
  mdebug("CoCreateInstance %x", hr);
  EXIT_ON_ERROR(hr)

  if (ao->device) {
    devlen = INT123_win32_utf8_wide(ao->device, &device, NULL);
    if(device && devlen > 0) {
      hr = IMMDeviceEnumerator_GetDevice(state->pEnumerator, device, &state->pDevice);
      mdebug("IMMDeviceEnumerator_GetDevice %x", hr);
      free(device);
    } else {
      debug("IMMDeviceEnumerator_GetDevice convert fail");
      return -1;
    }
  } else {
    hr = IMMDeviceEnumerator_GetDefaultAudioEndpoint(state->pEnumerator,eRender, eConsole, &state->pDevice);
    mdebug("IMMDeviceEnumerator_GetDefaultAudioEndpoint %x", hr);
  }
  EXIT_ON_ERROR(hr)

  hr = IMMDeviceActivator_Activate(state->pDevice,
                  &mpg123_IID_IAudioClient, CLSCTX_ALL,
                  NULL, (void**)&state->pAudioClient);
  debug("IMMDeviceActivator_Activate");
  EXIT_ON_ERROR(hr)

  return 0;
  Exit:
  mdebug("%s failed with %lx", __FUNCTION__, hr);
  return 1;
}

static int formats_generator(const out123_handle * const ao, const int waveformat, WAVEFORMATEX *const format){
  DWORD bytes_per_sample = 0;
  WORD tag = WAVE_FORMAT_PCM;
  mdebug("%s",__FUNCTION__);
  int ret = waveformat;
  switch(waveformat){
    case MPG123_ENC_ULAW_8:
      tag = WAVE_FORMAT_MULAW;
      bytes_per_sample = 1;
      break;
    case MPG123_ENC_ALAW_8:
      tag = WAVE_FORMAT_ALAW;
      bytes_per_sample = 1;
      break;
    case MPG123_ENC_SIGNED_8:
      bytes_per_sample = 1;
      break;
    case MPG123_ENC_FLOAT_32:
      tag = WAVE_FORMAT_IEEE_FLOAT;
      bytes_per_sample = 4;
    case MPG123_ENC_SIGNED_32:
      bytes_per_sample = 4;
      break;
    case MPG123_ENC_SIGNED_16:
      bytes_per_sample = 2;
      break;
    case MPG123_ENC_SIGNED_24:
      bytes_per_sample = 3;
      break;
    default:
      mdebug("uh oh unknown %x",waveformat);
      ret = 0;
      break;
  }
  format->wFormatTag = tag;
  format->nChannels = ao->channels;
  format->nSamplesPerSec = ao->rate;
  format->nAvgBytesPerSec = ao->channels * bytes_per_sample * ao->rate;
  format->nBlockAlign = ao->channels * bytes_per_sample;
  format->wBitsPerSample = bytes_per_sample * 8;
  format->cbSize = 0;
  return ret;
}

/* check supported formats */
typedef struct {
  unsigned int format;
  const char *name;
} tknown_formats;

static const tknown_formats known_formats[] = {
  { .format = MPG123_ENC_SIGNED_8,  .name = "MPG123_ENC_SIGNED_8"  },
  { .format = MPG123_ENC_SIGNED_16, .name = "MPG123_ENC_SIGNED_16" },
  { .format = MPG123_ENC_SIGNED_24, .name = "MPG123_ENC_SIGNED_24" },
  { .format = MPG123_ENC_SIGNED_32, .name = "MPG123_ENC_SIGNED_32" },
  { .format = MPG123_ENC_FLOAT_32,  .name = "MPG123_ENC_FLOAT_16"  },
  { .format = MPG123_ENC_ALAW_8,    .name = "MPG123_ENC_ALAW_8"    },
  { .format = MPG123_ENC_ULAW_8,    .name = "MPG123_ENC_ULAW_8"    },
  { .format = 0, .name = NULL }
};

static int get_formats_win32(out123_handle *ao){
  /* PLEASE check with write_init and write_win32 buffer size calculation in case it is able to support something other than 16bit */
  HRESULT hr;
  WAVEFORMATEX wf, *pClosestMatch = NULL;
  int ret = 0, i;
  mdebug("%s",__FUNCTION__);

  if(!ao || !ao->userptr) return -1;
  wasapi_state_struct *state = (wasapi_state_struct *) ao->userptr;
  mdebug("format %lx, channels %d, rate %ld",ao->format, ao->channels, ao->rate);

  for(i = 0; known_formats[i].name; i++) {
    if(ao->format & known_formats[i].format) {
      formats_generator(ao, known_formats[i].format, &wf);
      if((hr = IAudioClient_IsFormatSupported(state->pAudioClient, state->renderMode, &wf, &pClosestMatch)) == S_OK) {
        mdebug("OK format %s rate %ld supported", known_formats[i].name, ao->rate);
        ret |= known_formats[i].format;
      } else {
        if(hr & AUDCLNT_E_UNSUPPORTED_FORMAT) debug2("format %s rate %ld not supported", known_formats[i].name, ao->rate);
        if(pClosestMatch) {
          mdebug("Suggested: wFormatTag %x", pClosestMatch->wFormatTag);
          mdebug("Suggested: nChannels %u",  pClosestMatch->nChannels);
          mdebug("Suggested: nSamplesPerSec %lu", pClosestMatch->nSamplesPerSec);
          mdebug("Suggested: nAvgBytesPerSec %lu", pClosestMatch->nAvgBytesPerSec);
          mdebug("Suggested: nBlockAlign %u", pClosestMatch->nBlockAlign);
          mdebug("Suggested: wBitsPerSample %u", pClosestMatch->wBitsPerSample);
          mdebug("Suggested: cbSize %u", pClosestMatch->cbSize);
        }
      }
      if(pClosestMatch) CoTaskMemFree(pClosestMatch);
      pClosestMatch = NULL;
    }
  }
  mdebug("supported format %lx, channels %d, rate %ld",ret, ao->channels, ao->rate);

  return ret; /* afaik only 16bit 44.1kHz/48kHz has been known to work */
}

/* setup with agreed on format, for now only MPG123_ENC_SIGNED_16 */
static int write_init(out123_handle *ao){
  HRESULT hr;
  double offset = 0.5;

  mdebug("%s",__FUNCTION__);
  if(!ao || !ao->userptr) return -1;
  wasapi_state_struct *state = (wasapi_state_struct *) ao->userptr;

  WAVEFORMATEX s16;
  formats_generator(ao,ao->format,&s16);
  state->framesize = s16.nBlockAlign;
  mdebug("block size %ld", state->framesize);
  /* cargo cult code */
  hr = IAudioClient_GetDevicePeriod(state->pAudioClient,NULL, &state->hnsRequestedDuration);
  mdebug("IAudioClient_GetDevicePeriod %x", hr);
  reinit:
  hr = IAudioClient_Initialize(state->pAudioClient,
                       state->renderMode,
                       state->is_EventMode ? AUDCLNT_STREAMFLAGS_EVENTCALLBACK : 0,
                       state->hnsRequestedDuration,
                       state->renderMode == AUDCLNT_SHAREMODE_EXCLUSIVE ? state->hnsRequestedDuration : 0,
                       &s16,
                       NULL);
  mdebug("IAudioClient_Initialize %x", hr);
  if(hr == AUDCLNT_E_DEVICE_IN_USE){
    debug("AUDCLNT_SHAREMODE_EXCLUSIVE not possible, another application is using IAudioClient");
  }
  /* something about buffer sizes on Win7, fixme might loop forever */
  if (hr == AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED){
    if (offset > 10.0) goto Exit; /* is 10 enough to break out of the loop?*/
    debug("AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED");
    IAudioClient_GetBufferSize(state->pAudioClient,&state->bufferFrameCount);
    /* double buffered */
	state->hnsRequestedDuration = (REFERENCE_TIME)((BUFFER_TIME / s16.nSamplesPerSec * state->bufferFrameCount) + offset);
    offset += 0.5;
	IAudioClient_Release(state->pAudioClient);
	state->pAudioClient = NULL;
	hr = IMMDeviceActivator_Activate(state->pDevice,
                  &mpg123_IID_IAudioClient, CLSCTX_ALL,
                  NULL, (void**)&state->pAudioClient);
    mdebug("IMMDeviceActivator_Activate %x", hr);
    goto reinit;
  }
  EXIT_ON_ERROR(hr)
  hr = IAudioClient_GetService(state->pAudioClient,
                        &mpg123_IID_IAudioRenderClient,
                        (void**)&state->pRenderClient);
  mdebug("IAudioClient_GetService %x", hr);
  EXIT_ON_ERROR(hr)
  if (state->is_EventMode) {
    state->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    mdebug("CreateEvent %p", state->hEvent);
    if(!state->hEvent) goto Exit;
    hr = IAudioClient_SetEventHandle(state->pAudioClient,state->hEvent);
    mdebug("IAudioClient_SetEventHandle %x", hr);
    EXIT_ON_ERROR(hr);
  }
  hr = IAudioClient_GetBufferSize(state->pAudioClient,&state->bufferFrameCount);
  mdebug("IAudioClient_GetBufferSize %x", hr);
  EXIT_ON_ERROR(hr)
  return 0;
Exit:
  mdebug("%s failed with %lx", __FUNCTION__, hr);
  return 1;
}

/* Set play mode if unset, also raise thread priority */
static HRESULT play_init(out123_handle *ao){
  HRESULT hr = S_OK;
  if(!ao || !ao->userptr) return -1;
  wasapi_state_struct *state = (wasapi_state_struct *) ao->userptr;
  if(!state->is_playing){
    mdebug("%s",__FUNCTION__);
    state->hTask = AvSetMmThreadCharacteristicsW(L"Pro Audio", &state->taskIndex);
    hr = IAudioClient_Start(state->pAudioClient);
    mdebug("IAudioClient_Start %x", hr);
    EXIT_ON_ERROR(hr)
    state->is_playing = 1;
  }
  return hr;
Exit:
  mdebug("%s failed with %lx", __FUNCTION__, hr);
  return hr;
}

/* copy audio into IAudioRenderClient provided buffer */
static int write_win32(out123_handle *ao, unsigned char *buf, int len){
  HRESULT hr;
  UINT32 numFramesPadding = 0;
  size_t to_copy = 0;
  mdebug("%s",__FUNCTION__);
  if(!ao || !ao->userptr) return -1;
  wasapi_state_struct *state = (wasapi_state_struct *) ao->userptr;
  if(!len) return 0;
  if(!state->pRenderClient) write_init(ao);
  size_t frames_in = len/state->framesize; /* Frames in buf, is framesize even correct? */
  mdebug("%s mode entered", state->is_EventMode ? "Event" : "Push");

  if(state->is_EventMode) {
    /* Event mode WASAPI */
    DWORD retval = -1;
    int flag = 0; /* Silence flag */
feed_again_event:
    if(!state->pData){
      /* Acquire buffer */
      hr = IAudioClient_GetCurrentPadding(state->pAudioClient,&numFramesPadding);
      mdebug("IAudioClient_GetCurrentPadding %x", hr);
      EXIT_ON_ERROR(hr)

      state->numFramesAvailable = state->bufferFrameCount - numFramesPadding;

      hr = IAudioRenderClient_GetBuffer(state->pRenderClient, state->numFramesAvailable, &state->pData);
      mdebug("IAudioRenderClient_GetBuffer %lu %x", state->numFramesAvailable, hr);
      EXIT_ON_ERROR(hr)
    }
    if(frames_in){ /* Did we get half a frame?? non-zero len smaller than framesize? */
      /* We must put in exactly the amount of frames specified by IAudioRenderClient_GetBuffer */
      mdebug("frames_in %"PRIu64", state->pData_off %u, numFramesAvailable %u", frames_in, state->pData_off, state->numFramesAvailable);
      while(state->pData_off < state->numFramesAvailable){
        to_copy = state->numFramesAvailable - state->pData_off;
        mdebug("pData_off %"PRId64", numFramesAvailable %d, to_copy %"PRIu64, state->pData_off, state->numFramesAvailable, to_copy);
        if(to_copy > frames_in){
          /* buf can fit in provided buffer space */
          mdebug("all buffers copied, %"PRIu64, frames_in);
          memcpy(state->pData+state->pData_off*state->framesize,buf,state->framesize*(frames_in));
          state->pData_off += frames_in;
          frames_in = 0;
          break;
        } else {
          /* buf too big, needs spliting */
          mdebug("partial buffers %"PRIu64, to_copy);
          memcpy(state->pData+state->pData_off*state->framesize,buf,state->framesize*(to_copy));
          state->pData_off += to_copy;
          buf+=(to_copy*state->framesize);
          frames_in -= to_copy;
        }
      }
    } else {
      debug("AUDCLNT_BUFFERFLAGS_SILENT");
      /* In case we ever get half a frame, is it possible? */
      flag = AUDCLNT_BUFFERFLAGS_SILENT;
    }
    mdebug("Copied %"PRIu64", left %"PRIu64, state->pData_off, frames_in);
    if(state->pData_off == state->numFramesAvailable) {
      /* Tell IAudioRenderClient that buffer is filled and released */
      hr = IAudioRenderClient_ReleaseBuffer(state->pRenderClient,state->pData_off, flag);
      state->pData_off = 0;
      state->pData = NULL;
      debug("IAudioRenderClient_ReleaseBuffer");
      EXIT_ON_ERROR(hr)
      if(!state->is_playing){
        hr = play_init(ao);
        EXIT_ON_ERROR(hr)
      }
      /* wait for next pull event */
      retval = WaitForSingleObject(state->hEvent, 2000);
      if (retval != WAIT_OBJECT_0){
        /* Event handle timed out after a 2-second wait, something went very wrong */
        IAudioClient_Stop(state->pAudioClient);
        hr = ERROR_TIMEOUT;
        goto Exit;
      }
    }
    if(frames_in > 0)
      goto feed_again_event;
  } else { /* PUSH mode code */
      mdebug("block size %"PRIu64", %d len, %"PRIu64" number of frames", state->framesize, len, frames_in);
feed_again_push:
      /* How much buffer do we get to use? */
      hr = IAudioClient_GetBufferSize(state->pAudioClient,&state->bufferFrameCount);
      mdebug("IAudioClient_GetBufferSize %x", hr);
      EXIT_ON_ERROR(hr)
      hr = IAudioClient_GetCurrentPadding(state->pAudioClient,&numFramesPadding);
      mdebug("IAudioClient_GetCurrentPadding %x", hr);
      EXIT_ON_ERROR(hr)
      /* How much buffer is writable at the moment? */
      state->numFramesAvailable = state->bufferFrameCount - numFramesPadding;
      mdebug("numFramesAvailable %d, bufferFrameCount %d, numFramesPadding %d", state->numFramesAvailable, state->bufferFrameCount, numFramesPadding);

      if(state->numFramesAvailable > frames_in){
        /* can fit all frames now */
        state->pData_off = 0;
        to_copy = frames_in;
      } else {
        /* copy whatever that fits in the buffer */
        state->pData_off = frames_in - state->numFramesAvailable;
        to_copy = state->numFramesAvailable;
      }

      /* Acquire buffer */
      hr = IAudioRenderClient_GetBuffer(state->pRenderClient, to_copy, &state->pData);
      mdebug("IAudioRenderClient_GetBuffer %"PRIu64" frames", to_copy);
      EXIT_ON_ERROR(hr)

      /* Copy buffer */
      memcpy(state->pData, buf + state->pData_off * state->framesize, to_copy * state->framesize);

      /* Release buffer */
      hr = IAudioRenderClient_ReleaseBuffer(state->pRenderClient, to_copy, 0);
      mdebug("IAudioRenderClient_ReleaseBuffer %x", hr);

      EXIT_ON_ERROR(hr)
      if(!state->is_playing){
        hr = play_init(ao);
        EXIT_ON_ERROR(hr)
      }
      frames_in -= to_copy;
      /* Wait sometime for buffer to empty? */
      DWORD sleeptime = ((double)REFTIMES_PER_SEC) * to_copy / ao->rate / REFTIMES_PER_MILLISEC / 2;
      mdebug("Sleeping %ld msec", sleeptime);
      Sleep(sleeptime);
      if (frames_in > 0)
        goto feed_again_push;
  }
  return len;
Exit:
  mdebug("%s failed with %lx", __FUNCTION__, hr);
  return -1;
}

static void flush_win32(out123_handle *ao){
  /* Wait for the last buffer to play before stopping. */
  mdebug("%s",__FUNCTION__);
  if(!ao || !ao->userptr) return;
  wasapi_state_struct *state = (wasapi_state_struct *) ao->userptr;
  HRESULT hr;
  if(!state->pAudioClient) return;
  state->pData = NULL;
  hr = IAudioClient_Stop(state->pAudioClient);
  EXIT_ON_ERROR(hr)
  IAudioClient_Reset(state->pAudioClient);
  EXIT_ON_ERROR(hr)
  return;
  Exit:
  mdebug("%s IAudioClient_Stop with %lx", __FUNCTION__, hr);
}

static int close_win32(out123_handle *ao)
{
  mdebug("%s",__FUNCTION__);
  if(!ao || !ao->userptr) return -1;
  wasapi_state_struct *state = (wasapi_state_struct *) ao->userptr;
  if(state->is_EventMode && state->pData){
    /* Play all in buffer before closing */
    debug("Flushing remaining buffers");
    IAudioRenderClient_ReleaseBuffer(state->pRenderClient,state->bufferFrameCount, 0);
    WaitForSingleObject(state->hEvent, 2000);
    state->pData = NULL;
  }
  if(state->pAudioClient) IAudioClient_Stop(state->pAudioClient);
  if(state->pRenderClient) IAudioRenderClient_Release(state->pRenderClient);
  if(state->pAudioClient) IAudioClient_Release(state->pAudioClient);
  if(state->hTask) AvRevertMmThreadCharacteristics(state->hTask);
  if(state->pEnumerator) IMMDeviceEnumerator_Release(state->pEnumerator);
  if(state->pDevice) IMMDevice_Release(state->pDevice);
  free(state);
  ao->userptr = NULL;
  return 0;
}

static int enumerate_win32( out123_handle *ao, int (*store_device)(void *devlist
,       const char *name, const char *description), void *devlist )
{
	char *pszID = NULL, *pszDesc = NULL;
	HRESULT hr = S_OK;
	UINT pcDevices = 0, i = 0;
	LPWSTR pwszID = NULL;
	IMMDeviceEnumerator *pEnumerator = NULL;
	IMMDeviceCollection *pCollection = NULL;
	IPropertyStore *pProps = NULL;
	IMMDevice *pDevice = NULL;
	PROPVARIANT varName;

	hr = CoCreateInstance(&mpg123_CLSID_IMMDeviceEnumerator,NULL,CLSCTX_ALL, &mpg123_IID_IMMDeviceEnumerator,(void**)&pEnumerator);
	if(FAILED(hr) || pEnumerator == NULL) goto Exit;
	hr = IMMDeviceEnumerator_EnumAudioEndpoints(pEnumerator, eRender, DEVICE_STATE_ACTIVE, &pCollection);
	if(FAILED(hr) || pCollection == NULL) goto Exit;
	hr = IMMDeviceCollection_GetCount(pCollection, &pcDevices);
	EXIT_ON_ERROR(hr)

	for(i = 0; i < pcDevices; i++) {
		/* init */
		hr = IMMDeviceCollection_Item(pCollection, i, &pDevice);
		if(FAILED(hr) || pDevice == NULL) goto Exit;
		hr = IMMDevice_GetId(pDevice, &pwszID);
		if(FAILED(hr) || pwszID == NULL) goto Exit;
		hr = IMMDevice_OpenPropertyStore(pDevice, STGM_READ, &pProps);
		if(FAILED(hr) || pProps == NULL) goto Exit;

		/* get ID */
		INT123_win32_wide_utf8(pwszID, &pszID, NULL);
		if(pszID == NULL) goto Exit;

		/* get Property */
		PropVariantInit(&varName);
		hr = IPropertyStore_GetValue(pProps, &mpg123_PKEY_Device_FriendlyName, &varName);
		if(FAILED(hr)) {
			PropVariantClear(&varName);
			goto Exit;
		}

		/* get Description*/
		INT123_win32_wide_utf8(varName.pwszVal, &pszDesc, NULL);
		PropVariantClear(&varName);
		if(pszDesc == NULL) goto Exit;

		/* store */
		if(store_device(devlist, pszID, pszDesc))
			goto Exit;

		/* release */
		free(pszID);
		pszID = NULL;
		free(pszDesc);
		pszDesc = NULL;
		CoTaskMemFree(pwszID);
		pwszID = NULL;
		IMMDeviceEnumerator_Release(pDevice);
	}

	IMMDeviceCollection_Release(pCollection);
	IMMDeviceEnumerator_Release(pEnumerator);
        return 0;

	Exit:
	if(pszDesc) free(pszDesc);
	if(pszID) free(pszID);
	if(pwszID) CoTaskMemFree(pwszID);
	if(pProps) IPropertyStore_Release(pProps);
	if(pDevice) IMMDeviceEnumerator_Release(pDevice);
	if(pCollection) IMMDeviceCollection_Release(pCollection);
	if(pEnumerator) IMMDeviceEnumerator_Release(pEnumerator);
	return -1;
}
static void deinit_win32(out123_handle* ao)
{
	/* Deinitialize things from init_win32(). */
	CoUninitialize();
}

static int init_win32(out123_handle* ao){
	HRESULT hr;
	mdebug("%s",__FUNCTION__);
	if(!ao) return -1;

	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	mdebug("CoInitializeEx %x", hr);
	if(!(hr == S_OK || hr == S_FALSE))
		return -1;

	/* Set callbacks */
	ao->open = open_win32;
	ao->flush = flush_win32;
	ao->write = write_win32;
	ao->get_formats = get_formats_win32;
	ao->close = close_win32;
	ao->userptr = NULL;
	ao->enumerate = enumerate_win32;
	ao->deinit = deinit_win32;

	/* Success */
	return 0;
}
