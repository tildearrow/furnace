package com.rtmidi.yellowlab.midireader

import androidx.compose.runtime.mutableStateListOf
import androidx.lifecycle.ViewModel

data class MidiEvent(
    val time: Double = 0.0,
    val data: String = ""
)

@ExperimentalUnsignedTypes
fun ByteArray.toHex2(): String = asUByteArray().joinToString("") { it.toString(radix = 16).padStart(2, '0') }

class MidiViewModel : ViewModel() {

    // JNI functions in app-jni.cpp
    external fun portNames(): Array<String>
    external fun openPort(port: Int, listener: Any)
    external fun closePort(): Unit

    val ports = mutableStateListOf<String>()
    val events = mutableStateListOf<MidiEvent>()

    init {
        ports.add("NONE")
    }

    fun enumerateMidiPorts() {
        ports.clear()
        ports.add("NONE")
        val midiPorts = portNames()
        ports.addAll(midiPorts)
    }

    fun selectPort(id: Int) {
        events.clear()

        if (id == 0) {
            closePort()
            return
        }

        openPort(id - 1, this)
    }

    companion object {
        init {
            System.loadLibrary("midireader")
        }
    }

    // The JNI code in app-jni.cpp calls this method back when it receives a
    // MIDI message from RrMidi.
    //
    // This callback is not part of the RtMidi library, it's how this demo app
    // uses RtMidi to update the UI.
    @Suppress("unused")
    fun onMidiMessage(time: Double, message: ByteArray) {
        var textMsg = message.toHex2()
        events.add(MidiEvent(time = time, data = textMsg))
    }
}