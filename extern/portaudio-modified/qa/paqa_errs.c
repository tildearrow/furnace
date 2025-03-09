/** @file paqa_errs.c
    @ingroup qa_src
    @brief Self Testing Quality Assurance app for PortAudio
    Do lots of bad things to test error reporting.
    @author Phil Burk  http://www.softsynth.com
    Pieter Suurmond adapted to V19 API.
*/
/*
 * $Id$
 *
 * This program uses the PortAudio Portable Audio Library.
 * For more information see: http://www.portaudio.com
 * Copyright (c) 1999-2000 Ross Bencina and Phil Burk
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * The text above constitutes the entire PortAudio license; however,
 * the PortAudio community also makes the following non-binding requests:
 *
 * Any person wishing to distribute modifications to the Software is
 * requested to send the modifications to the original developer so that
 * they can be incorporated into the canonical version. It is also
 * requested that these non-binding requests be included along with the
 * license above.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h> /* for EXIT_SUCCESS and EXIT_FAILURE */
#include <math.h>

#include "portaudio.h"
#include "paqa_macros.h"

/*--------- Definitions ---------*/
#define MODE_INPUT        (0)
#define MODE_OUTPUT       (1)
#define FRAMES_PER_BUFFER (64)
#define SAMPLE_RATE       (44100.0)

PAQA_INSTANTIATE_GLOBALS

typedef struct PaQaData
{
    unsigned long  framesLeft;
    int            numChannels;
    int            bytesPerSample;
    int            mode;
}
PaQaData;

/*-------------------------------------------------------------------------*/
/* This routine will be called by the PortAudio engine when audio is needed.
   It may be called at interrupt level on some machines so don't do anything
   that could mess up the system like calling malloc() or free().
*/
static int QaCallback( const void*                      inputBuffer,
                       void*                            outputBuffer,
                       unsigned long                    framesPerBuffer,
                       const PaStreamCallbackTimeInfo*  timeInfo,
                       PaStreamCallbackFlags            statusFlags,
                       void*                            userData )
{
    unsigned long   i;
    unsigned char*  out = (unsigned char *) outputBuffer;
    PaQaData*       data = (PaQaData *) userData;

    (void)inputBuffer; /* Prevent "unused variable" warnings. */

    /* Zero out buffer so we don't hear terrible noise. */
    if( data->mode == MODE_OUTPUT )
    {
        unsigned long numBytes = framesPerBuffer * data->numChannels * data->bytesPerSample;
        for( i=0; i<numBytes; i++ )
        {
            *out++ = 0;
        }
    }
    /* Are we through yet? */
    if( data->framesLeft > framesPerBuffer )
    {
        data->framesLeft -= framesPerBuffer;
        return 0;
    }
    else
    {
        data->framesLeft = 0;
        return 1;
    }
}

static PaDeviceIndex FindInputOnlyDevice(void)
{
    PaDeviceIndex result = Pa_GetDefaultInputDevice();
    if( result != paNoDevice && Pa_GetDeviceInfo(result)->maxOutputChannels == 0 )
        return result;

    for( result = 0; result < Pa_GetDeviceCount(); ++result )
    {
        if( Pa_GetDeviceInfo(result)->maxOutputChannels == 0 )
            return result;
    }

    return paNoDevice;
}

static PaDeviceIndex FindOutputOnlyDevice(void)
{
    PaDeviceIndex result = Pa_GetDefaultOutputDevice();
    if( result != paNoDevice && Pa_GetDeviceInfo(result)->maxInputChannels == 0 )
        return result;

    for( result = 0; result < Pa_GetDeviceCount(); ++result )
    {
        if( Pa_GetDeviceInfo(result)->maxInputChannels == 0 )
            return result;
    }

    return paNoDevice;
}

/*-------------------------------------------------------------------------------------------------*/
static int TestBadOpens( void )
{
    PaStream*           stream = NULL;
    PaError             result;
    PaQaData            myData;
    PaStreamParameters  ipp, opp;
    const PaDeviceInfo* info = NULL;


    /* Setup data for synthesis thread. */
    myData.framesLeft = (unsigned long) (SAMPLE_RATE * 100); /* 100 seconds */
    myData.numChannels = 1;
    myData.mode = MODE_OUTPUT;

    /*----------------------------- No devices specified: */
    ipp.device                    = opp.device                    = paNoDevice;
    ipp.channelCount              = opp.channelCount              = 0; /* Also no channels. */
    ipp.hostApiSpecificStreamInfo = opp.hostApiSpecificStreamInfo = NULL;
    ipp.sampleFormat              = opp.sampleFormat              = paFloat32;
    /* Take the low latency of the default device for all subsequent tests. */
    info = Pa_GetDeviceInfo(Pa_GetDefaultInputDevice());
    ipp.suggestedLatency          = info ? info->defaultLowInputLatency : 0.100;
    info = Pa_GetDeviceInfo(Pa_GetDefaultOutputDevice());
    opp.suggestedLatency          = info ? info->defaultLowOutputLatency : 0.100;
    HOPEFOR(((result = Pa_OpenStream(&stream, &ipp, &opp,
                                     SAMPLE_RATE, FRAMES_PER_BUFFER,
                                     paClipOff, QaCallback, &myData )) == paInvalidDevice));

    /*----------------------------- No devices specified #2: */
    HOPEFOR(((result = Pa_OpenStream(&stream, NULL, NULL,
                                     SAMPLE_RATE, FRAMES_PER_BUFFER,
                                     paClipOff, QaCallback, &myData )) == paInvalidDevice));

    /*----------------------------- Out of range input device specified: */
    ipp.hostApiSpecificStreamInfo = opp.hostApiSpecificStreamInfo = NULL;
    ipp.sampleFormat              = opp.sampleFormat              = paFloat32;
    ipp.channelCount = 0;           ipp.device = Pa_GetDeviceCount(); /* And no output device, and no channels. */
    opp.channelCount = 0;           opp.device = paNoDevice;
    HOPEFOR(((result = Pa_OpenStream(&stream, &ipp, NULL,
                                     SAMPLE_RATE, FRAMES_PER_BUFFER,
                                     paClipOff, QaCallback, &myData )) == paInvalidDevice));

    /*----------------------------- Out of range output device specified: */
    ipp.hostApiSpecificStreamInfo = opp.hostApiSpecificStreamInfo = NULL;
    ipp.sampleFormat              = opp.sampleFormat              = paFloat32;
    ipp.channelCount = 0;           ipp.device = paNoDevice; /* And no input device, and no channels. */
    opp.channelCount = 0;           opp.device = Pa_GetDeviceCount();
    HOPEFOR(((result = Pa_OpenStream(&stream, NULL, &opp,
                                     SAMPLE_RATE, FRAMES_PER_BUFFER,
                                     paClipOff, QaCallback, &myData )) == paInvalidDevice));

    if (Pa_GetDefaultInputDevice() != paNoDevice) {
        /*----------------------------- Zero input channels: */
        ipp.hostApiSpecificStreamInfo = opp.hostApiSpecificStreamInfo = NULL;
        ipp.sampleFormat              = opp.sampleFormat              = paFloat32;
        ipp.channelCount = 0;           ipp.device = Pa_GetDefaultInputDevice();
        opp.channelCount = 0;           opp.device = paNoDevice;    /* And no output device, and no output channels. */
        HOPEFOR(((result = Pa_OpenStream(&stream, &ipp, NULL,
                                         SAMPLE_RATE, FRAMES_PER_BUFFER,
                                         paClipOff, QaCallback, &myData )) == paInvalidChannelCount));
    }

    if (Pa_GetDefaultOutputDevice() != paNoDevice) {
        /*----------------------------- Zero output channels: */
        ipp.hostApiSpecificStreamInfo = opp.hostApiSpecificStreamInfo = NULL;
        ipp.sampleFormat              = opp.sampleFormat              = paFloat32;
        ipp.channelCount = 0;           ipp.device = paNoDevice; /* And no input device, and no input channels. */
        opp.channelCount = 0;           opp.device = Pa_GetDefaultOutputDevice();
        HOPEFOR(((result = Pa_OpenStream(&stream, NULL, &opp,
                                         SAMPLE_RATE, FRAMES_PER_BUFFER,
                                         paClipOff, QaCallback, &myData )) == paInvalidChannelCount));
    }
    /*----------------------------- Nonzero input and output channels but no output device: */
    ipp.hostApiSpecificStreamInfo = opp.hostApiSpecificStreamInfo = NULL;
    ipp.sampleFormat              = opp.sampleFormat              = paFloat32;
    ipp.channelCount = 2;           ipp.device = Pa_GetDefaultInputDevice();        /* Both stereo. */
    opp.channelCount = 2;           opp.device = paNoDevice;
    HOPEFOR(((result = Pa_OpenStream(&stream, &ipp, &opp,
                                     SAMPLE_RATE, FRAMES_PER_BUFFER,
                                     paClipOff, QaCallback, &myData )) == paInvalidDevice));

    /*----------------------------- Nonzero input and output channels but no input device: */
    ipp.hostApiSpecificStreamInfo = opp.hostApiSpecificStreamInfo = NULL;
    ipp.sampleFormat              = opp.sampleFormat              = paFloat32;
    ipp.channelCount = 2;           ipp.device = paNoDevice;
    opp.channelCount = 2;           opp.device = Pa_GetDefaultOutputDevice();
    HOPEFOR(((result = Pa_OpenStream(&stream, &ipp, &opp,
                                     SAMPLE_RATE, FRAMES_PER_BUFFER,
                                     paClipOff, QaCallback, &myData )) == paInvalidDevice));

    if (Pa_GetDefaultOutputDevice() != paNoDevice) {
        /*----------------------------- NULL stream pointer: */
        ipp.hostApiSpecificStreamInfo = opp.hostApiSpecificStreamInfo = NULL;
        ipp.sampleFormat              = opp.sampleFormat              = paFloat32;
        ipp.channelCount = 0;           ipp.device = paNoDevice;           /* Output is more likely than input. */
        opp.channelCount = 2;           opp.device = Pa_GetDefaultOutputDevice();    /* Only 2 output channels. */
        HOPEFOR(((result = Pa_OpenStream(NULL, &ipp, &opp,
                                         SAMPLE_RATE, FRAMES_PER_BUFFER,
                                         paClipOff, QaCallback, &myData )) == paBadStreamPtr));

        /*----------------------------- Low sample rate: */
        ipp.hostApiSpecificStreamInfo = opp.hostApiSpecificStreamInfo = NULL;
        ipp.sampleFormat              = opp.sampleFormat              = paFloat32;
        ipp.channelCount = 0;           ipp.device = paNoDevice;
        opp.channelCount = 2;           opp.device = Pa_GetDefaultOutputDevice();
        HOPEFOR(((result = Pa_OpenStream(&stream, NULL, &opp,
                                         1.0, FRAMES_PER_BUFFER, /* 1 cycle per second (1 Hz) is too low. */
                                         paClipOff, QaCallback, &myData )) == paInvalidSampleRate));

        /*----------------------------- High sample rate: */
        ipp.hostApiSpecificStreamInfo = opp.hostApiSpecificStreamInfo = NULL;
        ipp.sampleFormat              = opp.sampleFormat              = paFloat32;
        ipp.channelCount = 0;           ipp.device = paNoDevice;
        opp.channelCount = 2;           opp.device = Pa_GetDefaultOutputDevice();
        HOPEFOR(((result = Pa_OpenStream(&stream, NULL, &opp,
                                         10000000.0, FRAMES_PER_BUFFER, /* 10^6 cycles per second (10 MHz) is too high. */
                                         paClipOff, QaCallback, &myData )) == paInvalidSampleRate));

        /*----------------------------- NULL callback: */
        /* NULL callback is valid in V19 -- it means use blocking read/write stream

        ipp.hostApiSpecificStreamInfo = opp.hostApiSpecificStreamInfo = NULL;
        ipp.sampleFormat              = opp.sampleFormat              = paFloat32;
        ipp.channelCount = 0;           ipp.device = paNoDevice;
        opp.channelCount = 2;           opp.device = Pa_GetDefaultOutputDevice();
        HOPEFOR(((result = Pa_OpenStream(&stream, NULL, &opp,
                                         SAMPLE_RATE, FRAMES_PER_BUFFER,
                                         paClipOff,
                                         NULL,
                                         &myData )) == paNullCallback));
        */

        /*----------------------------- Bad flag: */
        ipp.hostApiSpecificStreamInfo = opp.hostApiSpecificStreamInfo = NULL;
        ipp.sampleFormat              = opp.sampleFormat              = paFloat32;
        ipp.channelCount = 0;           ipp.device = paNoDevice;
        opp.channelCount = 2;           opp.device = Pa_GetDefaultOutputDevice();
        HOPEFOR(((result = Pa_OpenStream(&stream, NULL, &opp,
                                         SAMPLE_RATE, FRAMES_PER_BUFFER,
                                         255,                      /* Is 8 maybe legal V19 API? */
                                         QaCallback, &myData )) == paInvalidFlag));
    }

    /*----------------------------- using input device as output device: */
    if( FindInputOnlyDevice() != paNoDevice )
    {
        ipp.hostApiSpecificStreamInfo = opp.hostApiSpecificStreamInfo = NULL;
        ipp.sampleFormat              = opp.sampleFormat              = paFloat32;
        ipp.channelCount = 0;           ipp.device = paNoDevice; /* And no input device, and no channels. */
        opp.channelCount = 2;           opp.device = FindInputOnlyDevice();
        HOPEFOR(((result = Pa_OpenStream(&stream, NULL, &opp,
                                         SAMPLE_RATE, FRAMES_PER_BUFFER,
                                         paClipOff, QaCallback, &myData )) == paInvalidChannelCount));
    }

    /*----------------------------- using output device as input device: */
    if( FindOutputOnlyDevice() != paNoDevice )
    {
        ipp.hostApiSpecificStreamInfo = opp.hostApiSpecificStreamInfo = NULL;
        ipp.sampleFormat              = opp.sampleFormat              = paFloat32;
        ipp.channelCount = 2;           ipp.device = FindOutputOnlyDevice();
        opp.channelCount = 0;           opp.device = paNoDevice;  /* And no output device, and no channels. */
        HOPEFOR(((result = Pa_OpenStream(&stream, &ipp, NULL,
                                         SAMPLE_RATE, FRAMES_PER_BUFFER,
                                         paClipOff, QaCallback, &myData )) == paInvalidChannelCount));

    }

    if( stream != NULL ) Pa_CloseStream( stream );
    return result;
}

/*-----------------------------------------------------------------------------------------*/
static int TestBadActions( void )
{
    PaStream*           stream = NULL;
    const PaDeviceInfo* deviceInfo = NULL;
    PaError             result = 0;
    PaQaData            myData;
    PaStreamParameters  opp;
    const PaDeviceInfo* info = NULL;

    /* Setup data for synthesis thread. */
    myData.framesLeft = (unsigned long)(SAMPLE_RATE * 100); /* 100 seconds */
    myData.numChannels = 1;
    myData.mode = MODE_OUTPUT;

    opp.device                    = Pa_GetDefaultOutputDevice(); /* Default output. */
    opp.channelCount              = 2;                           /* Stereo output.  */
    opp.hostApiSpecificStreamInfo = NULL;
    opp.sampleFormat              = paFloat32;
    info = Pa_GetDeviceInfo(opp.device);
    opp.suggestedLatency          = info ? info->defaultLowOutputLatency : 0.100;

    if (opp.device != paNoDevice) {
        HOPEFOR(((result = Pa_OpenStream(&stream, NULL, /* Take NULL as input parame-     */
                                         &opp,          /* ters, meaning try only output. */
                                         SAMPLE_RATE, FRAMES_PER_BUFFER,
                                         paClipOff, QaCallback, &myData )) == paNoError));
    }

    HOPEFOR(((deviceInfo = Pa_GetDeviceInfo(paNoDevice))    == NULL));
    HOPEFOR(((deviceInfo = Pa_GetDeviceInfo(87654))    == NULL));
    HOPEFOR(((result = Pa_StartStream(NULL))    == paBadStreamPtr));
    HOPEFOR(((result = Pa_StopStream(NULL))     == paBadStreamPtr));
    HOPEFOR(((result = Pa_IsStreamStopped(NULL)) == paBadStreamPtr));
    HOPEFOR(((result = Pa_IsStreamActive(NULL)) == paBadStreamPtr));
    HOPEFOR(((result = Pa_CloseStream(NULL))    == paBadStreamPtr));
    HOPEFOR(((result = Pa_SetStreamFinishedCallback(NULL, NULL)) == paBadStreamPtr));
    HOPEFOR(((result = !Pa_GetStreamInfo(NULL))));
    HOPEFOR(((result = Pa_GetStreamTime(NULL))  == 0.0));
    HOPEFOR(((result = Pa_GetStreamCpuLoad(NULL))  == 0.0));
    HOPEFOR(((result = Pa_ReadStream(NULL, NULL, 0))  == paBadStreamPtr));
    HOPEFOR(((result = Pa_WriteStream(NULL, NULL, 0))  == paBadStreamPtr));

    /** @todo test Pa_GetStreamReadAvailable and Pa_GetStreamWriteAvailable */

    if (stream != NULL) Pa_CloseStream(stream);
    return result;
}

/*---------------------------------------------------------------------*/
int main(void);
int main(void)
{
    PaError result;

    printf("-----------------------------\n");
    printf("paqa_errs - PortAudio QA test\n");
    ASSERT_EQ(paNoError, (result = Pa_Initialize()));
    TestBadOpens();
    TestBadActions();
error:
    Pa_Terminate();

    PAQA_PRINT_RESULT;
    return PAQA_EXIT_RESULT;
}
