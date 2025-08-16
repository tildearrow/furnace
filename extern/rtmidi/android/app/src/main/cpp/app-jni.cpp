#include <jni.h>
#include <string>
#include "RtMidi.h"

static RtMidiIn* midiLib = new RtMidiIn( RtMidi::Api::ANDROID_AMIDI );
static jobject callbackObj = NULL;
static jmethodID callbackMethod;
static JavaVM* jvm;

extern "C" JNIEXPORT jobjectArray JNICALL
Java_com_rtmidi_yellowlab_midireader_MidiViewModel_portNames(
        JNIEnv* env,
        jobject /* this */) {

    auto portCount = midiLib->getPortCount();
    auto names = env->NewObjectArray(portCount, env->FindClass("java/lang/String"), NULL);

    for (int i=0; i<portCount; i++) {
        auto name = midiLib->getPortName(i);
        env->SetObjectArrayElement(names, i, env->NewStringUTF(name.c_str()));
    }

    return names;
}

static void midicallback( double deltatime, std::vector< unsigned char > *message, void */*userData*/ ) {
    unsigned int numBytes = message->size();

    JNIEnv* env;
    jvm->AttachCurrentThread(&env, NULL);

    // Allocate the Java array and fill with received data
    jbyteArray ret = env->NewByteArray(numBytes);
    env->SetByteArrayRegion(ret, 0, numBytes, (jbyte*)message);

    // send it to the (Java) callback
    env->CallVoidMethod(callbackObj, callbackMethod, deltatime, ret);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_rtmidi_yellowlab_midireader_MidiViewModel_openPort(
        JNIEnv *env,
        jobject /* this */,
        jint port,
        jobject listener) {

    env->GetJavaVM(&jvm);

    midiLib->setCallback( midicallback );
    midiLib->openPort(port);

    if (callbackObj) env->DeleteGlobalRef(callbackObj);
    callbackObj = env->NewGlobalRef(listener);
    callbackMethod = env->GetMethodID(env->GetObjectClass(listener), "onMidiMessage", "(D[B)V");
}

extern "C" JNIEXPORT void JNICALL
Java_com_rtmidi_yellowlab_midireader_MidiViewModel_closePort(
        JNIEnv *env,
        jobject /* this */) {

    midiLib->closePort();
    if (callbackObj) env->DeleteGlobalRef(callbackObj);
    callbackObj = NULL;
    callbackMethod = NULL;
}
