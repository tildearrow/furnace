// The native half of RtMidiBridge.kt: one RtMidiIn and one RtMidiOut, the same
// as the iOS example's RtMidiBridge.mm.
//
// Every RtMidiError is caught here and returned as a string: a C++ exception
// that crosses back into Java aborts the process.

#include <jni.h>
#include <string>
#include <vector>
#include "RtMidi.h"

static JavaVM *jvm = NULL;
static jobject bridge = NULL;       // the RtMidiBridge, for received messages
static jmethodID receivedMethod = NULL;

// Constructed on first use rather than in a static initializer: a static
// initializer runs while System.loadLibrary() is still loading this library,
// before the JNI environment the Android backend needs is usable.
static RtMidiIn *midiIn = NULL;
static RtMidiOut *midiOut = NULL;

// RtMidi calls this on its own input thread.
static void received( double deltaTime, std::vector<unsigned char> *message, void * ) {
    if ( bridge == NULL || message->empty() ) return;

    // The JVM knows nothing about RtMidi's thread until it is attached.  Attach
    // once and stay attached: attaching per message would leak each time.
    JNIEnv *env = NULL;
    if ( jvm->GetEnv( (void **) &env, JNI_VERSION_1_6 ) != JNI_OK &&
         jvm->AttachCurrentThread( &env, NULL ) != JNI_OK )
        return;

    jbyteArray bytes = env->NewByteArray( (jsize) message->size() );
    if ( bytes == NULL ) return;
    env->SetByteArrayRegion( bytes, 0, (jsize) message->size(), (const jbyte *) message->data() );
    env->CallVoidMethod( bridge, receivedMethod, bytes, deltaTime );
    // This thread never returns to Java, so local references are never freed
    // unless deleted here.
    env->DeleteLocalRef( bytes );
}

static RtMidiIn *getIn( JNIEnv *env, jobject thiz ) {
    if ( midiIn == NULL ) {
        env->GetJavaVM( &jvm );
        bridge = env->NewGlobalRef( thiz );
        receivedMethod = env->GetMethodID( env->GetObjectClass( thiz ), "received", "([BD)V" );
        midiIn = new RtMidiIn( RtMidi::ANDROID_AMIDI );
        midiIn->ignoreTypes( false, false, false );  // show everything, SysEx included
        midiIn->setCallback( received );
    }
    return midiIn;
}

static RtMidiOut *getOut() {
    if ( midiOut == NULL ) midiOut = new RtMidiOut( RtMidi::ANDROID_AMIDI );
    return midiOut;
}

static jobjectArray portNames( JNIEnv *env, RtMidi *midi ) {
    unsigned int count = midi->getPortCount();
    jobjectArray names = env->NewObjectArray( count, env->FindClass( "java/lang/String" ), NULL );
    for ( unsigned int i = 0; i < count; i++ ) {
        jstring name = env->NewStringUTF( midi->getPortName( i ).c_str() );
        env->SetObjectArrayElement( names, i, name );
        env->DeleteLocalRef( name );
    }
    return names;
}

// Closes the port if open, then opens the one at index.  Null on success,
// otherwise RtMidi's error message.
static jstring openPort( JNIEnv *env, RtMidi *midi, jint index ) {
    try {
        if ( midi->isPortOpen() ) midi->closePort();
        midi->openPort( (unsigned int) index );
    }
    catch ( RtMidiError &error ) {
        return env->NewStringUTF( error.getMessage().c_str() );
    }
    return NULL;
}

extern "C" {

JNIEXPORT jobjectArray JNICALL
Java_com_rtmidi_yellowlab_midireader_RtMidiBridge_outputPortNames( JNIEnv *env, jobject ) {
    return portNames( env, getOut() );
}

JNIEXPORT jobjectArray JNICALL
Java_com_rtmidi_yellowlab_midireader_RtMidiBridge_inputPortNames( JNIEnv *env, jobject thiz ) {
    return portNames( env, getIn( env, thiz ) );
}

JNIEXPORT jstring JNICALL
Java_com_rtmidi_yellowlab_midireader_RtMidiBridge_openOutputPort( JNIEnv *env, jobject, jint index ) {
    return openPort( env, getOut(), index );
}

JNIEXPORT jstring JNICALL
Java_com_rtmidi_yellowlab_midireader_RtMidiBridge_openInputPort( JNIEnv *env, jobject thiz, jint index ) {
    return openPort( env, getIn( env, thiz ), index );
}

JNIEXPORT void JNICALL
Java_com_rtmidi_yellowlab_midireader_RtMidiBridge_closeOutputPort( JNIEnv *, jobject ) {
    if ( midiOut && midiOut->isPortOpen() ) midiOut->closePort();
}

JNIEXPORT void JNICALL
Java_com_rtmidi_yellowlab_midireader_RtMidiBridge_closeInputPort( JNIEnv *, jobject ) {
    if ( midiIn && midiIn->isPortOpen() ) midiIn->closePort();
}

// Null on success, otherwise the reason it was not sent.
JNIEXPORT jstring JNICALL
Java_com_rtmidi_yellowlab_midireader_RtMidiBridge_sendBytes( JNIEnv *env, jobject, jbyteArray bytes ) {
    if ( !getOut()->isPortOpen() ) return env->NewStringUTF( "no output port is open" );
    std::vector<unsigned char> message( env->GetArrayLength( bytes ) );
    env->GetByteArrayRegion( bytes, 0, (jsize) message.size(), (jbyte *) message.data() );
    try {
        midiOut->sendMessage( &message );
    }
    catch ( RtMidiError &error ) {
        return env->NewStringUTF( error.getMessage().c_str() );
    }
    return NULL;
}

}
