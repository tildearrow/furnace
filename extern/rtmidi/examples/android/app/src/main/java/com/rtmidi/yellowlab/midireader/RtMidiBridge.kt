package com.rtmidi.yellowlab.midireader

// A thin wrapper over one RtMidiIn and one RtMidiOut, implemented in
// app-jni.cpp. The same shape as the iOS example's RtMidiBridge.
//
// Every call blocks: opening a port waits for Android to open the device, up
// to five seconds when it never answers. Call from a background thread.
class RtMidiBridge {
    // Called on RtMidi's input thread with each received message.
    var receiveHandler: ((bytes: ByteArray, deltaTime: Double) -> Unit)? = null

    external fun outputPortNames(): Array<String>
    external fun inputPortNames(): Array<String>

    // Null on success, otherwise RtMidi's error message.
    external fun openOutputPort(index: Int): String?
    external fun openInputPort(index: Int): String?

    external fun closeOutputPort()
    external fun closeInputPort()

    // Null on success, otherwise the reason the message was not sent.
    external fun sendBytes(bytes: ByteArray): String?

    @Suppress("unused")  // called from app-jni.cpp
    private fun received(bytes: ByteArray, deltaTime: Double) {
        receiveHandler?.invoke(bytes, deltaTime)
    }

    companion object {
        init {
            System.loadLibrary("midireader")
        }
    }
}
