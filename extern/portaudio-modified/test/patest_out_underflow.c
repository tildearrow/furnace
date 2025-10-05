/** @file patest_out_underflow.c
    @ingroup test_src
    @brief Count output underflows (using paOutputUnderflow flag)
    under overloaded and normal conditions.
    @author Ross Bencina <rossb@audiomulch.com>
    @author Phil Burk <philburk@softsynth.com>
*/
/*
 * $Id$
 *
 * This program uses the PortAudio Portable Audio Library.
 * For more information see: http://www.portaudio.com
 * Copyright (c) 1999-2004 Ross Bencina and Phil Burk
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

#include <stdio.h>
#include <math.h>
#include "portaudio.h"

#define MAX_SINES     (1000)
#define MAX_LOAD      (1.2)
#define SAMPLE_RATE   (44100)
#define FRAMES_PER_BUFFER  (512)
#ifndef M_PI
#define M_PI  (3.14159265)
#endif
#define TWOPI (M_PI * 2.0)

typedef struct paTestData
{
    int sineCount;
    double phases[MAX_SINES];
    int countUnderflows;
    int outputUnderflowCount;
}
paTestData;

/* This routine will be called by the PortAudio engine when audio is needed.
** It may called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
static int patestCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{
    paTestData *data = (paTestData*)userData;
    float *out = (float*)outputBuffer;
    unsigned long i;
    int j;
    int finished = paContinue;
    (void) timeInfo;    /* Prevent unused variable warning. */
    (void) inputBuffer; /* Prevent unused variable warning. */


    if( data->countUnderflows && (statusFlags & paOutputUnderflow) )
    {
        data->outputUnderflowCount++;
    }
    for( i=0; i<framesPerBuffer; i++ )
    {
        float output = 0.0;
        double phaseInc = 0.02;
        double phase;

        for( j=0; j<data->sineCount; j++ )
        {
            /* Advance phase of next oscillator. */
            phase = data->phases[j];
            phase += phaseInc;
            if( phase > TWOPI ) phase -= TWOPI;

            phaseInc *= 1.02;
            if( phaseInc > 0.5 ) phaseInc *= 0.5;

            /* This is not a very efficient way to calc sines. */
            output += (float) sin( phase );
            data->phases[j] = phase;
        }
        *out++ = (float) (output / data->sineCount);
    }

    return finished;
}

/*******************************************************************/
int main(void);
int main(void)
{
    PaStreamParameters outputParameters;
    PaStream *stream;
    PaError err;
    int safeSineCount, stressedSineCount;
    int sineCount;
    int safeUnderflowCount, stressedUnderflowCount;
    paTestData data = {0};
    double load;
    double suggestedLatency;


    printf("PortAudio Test: output sine waves, count underflows. SR = %d, BufSize = %d. MAX_LOAD = %f\n",
        SAMPLE_RATE, FRAMES_PER_BUFFER, (float)MAX_LOAD );

    err = Pa_Initialize();
    if( err != paNoError ) goto error;

    outputParameters.device = Pa_GetDefaultOutputDevice();  /* default output device */
    if (outputParameters.device == paNoDevice) {
        fprintf(stderr,"Error: No default output device.\n");
        goto error;
    }
    outputParameters.channelCount = 1;                      /* mono output */
    outputParameters.sampleFormat = paFloat32;              /* 32 bit floating point output */
    suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
    outputParameters.suggestedLatency = suggestedLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(
              &stream,
              NULL,         /* no input */
              &outputParameters,
              SAMPLE_RATE,
              FRAMES_PER_BUFFER,
              paClipOff,    /* we won't output out of range samples so don't bother clipping them */
              patestCallback,
              &data );
    if( err != paNoError ) goto error;
    err = Pa_StartStream( stream );
    if( err != paNoError ) goto error;

    printf("Establishing load conditions...\n" );

    /* Determine number of sines required to get to 50% */
    do
    {
        Pa_Sleep( 100 );

        load = Pa_GetStreamCpuLoad( stream );
        printf("sineCount = %d, CPU load = %f\n", data.sineCount, load );

        if( load < 0.3 )
        {
            data.sineCount += 10;
        }
        else if( load < 0.4 )
        {
            data.sineCount += 2;
        }
        else
        {
            data.sineCount += 1;
        }
    }
    while( load < 0.5 && data.sineCount < (MAX_SINES-1));

    safeSineCount = data.sineCount;

    /* Calculate target stress value then ramp up to that level*/
    stressedSineCount = (int) (2.0 * data.sineCount * MAX_LOAD );
    if( stressedSineCount > MAX_SINES )
        stressedSineCount = MAX_SINES;
    sineCount = data.sineCount;
    for( ; sineCount < stressedSineCount; sineCount+=4 )
    {
        data.sineCount = sineCount;
        Pa_Sleep( 100 );
        load = Pa_GetStreamCpuLoad( stream );
        printf("STRESSING: sineCount = %d, CPU load = %f\n", sineCount, load );
    }

    printf("Counting underflows for 2 seconds.\n");
    data.countUnderflows = 1;
    Pa_Sleep( 2000 );

    stressedUnderflowCount = data.outputUnderflowCount;

    data.countUnderflows = 0;
    data.sineCount = safeSineCount;

    printf("Resuming safe load...\n");
    Pa_Sleep( 1500 );
    data.outputUnderflowCount = 0;
    Pa_Sleep( 1500 );
    load = Pa_GetStreamCpuLoad( stream );
    printf("sineCount = %d, CPU load = %f\n", data.sineCount, load );

    printf("Counting underflows for 5 seconds.\n");
    data.countUnderflows = 1;
    Pa_Sleep( 5000 );

    safeUnderflowCount = data.outputUnderflowCount;

    printf("Stop stream.\n");
    err = Pa_StopStream( stream );
    if( err != paNoError ) goto error;

    err = Pa_CloseStream( stream );
    if( err != paNoError ) goto error;

    Pa_Terminate();

    printf("suggestedLatency = %f\n", suggestedLatency);

    // Report pass or fail
    if( stressedUnderflowCount == 0 )
        printf("Test FAILED, no output underflows detected under stress.\n");
    else
        printf("Test %s, %d expected output underflows detected under stress, "
               "%d unexpected underflows detected under safe load.\n",
               (safeUnderflowCount == 0) ? "PASSED" : "FAILED",
               stressedUnderflowCount, safeUnderflowCount );

    return err;
error:
    Pa_Terminate();
    fprintf( stderr, "An error occurred while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", err );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
    return err;
}
