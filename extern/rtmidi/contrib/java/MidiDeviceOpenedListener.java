package com.yellowlab.rtmidi;

import android.media.midi.MidiDevice;
import android.media.midi.MidiManager;

/**
 * This class must be included in the Android app using rtmidi. It is used by the
 * C++ code in the Android library because a Java listener class is required by the
 * Android Midi API.
 */
public class MidiDeviceOpenedListener implements MidiManager.OnDeviceOpenedListener {
    private long nativeId;
    private boolean isOutput;

    public MidiDeviceOpenedListener(long id, boolean output) {
        nativeId = id;
        isOutput = output;
    }

    @Override
    public void onDeviceOpened(MidiDevice midiDevice) {
        midiDeviceOpened(midiDevice, nativeId, isOutput);
    }

    private native static void midiDeviceOpened(MidiDevice midiDevice, long id, boolean isOutput);
}
