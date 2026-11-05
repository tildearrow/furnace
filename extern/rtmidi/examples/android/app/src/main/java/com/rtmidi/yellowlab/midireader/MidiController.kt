package com.rtmidi.yellowlab.midireader

import android.app.Application
import android.content.Context
import android.media.midi.MidiDeviceInfo
import android.media.midi.MidiManager
import android.os.Handler
import android.os.Looper
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateListOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.AndroidViewModel
import java.util.Date
import java.util.concurrent.Executors

data class LogLine(val timestamp: Date, val text: String)

data class MidiMonitorLine(val timestamp: Date, val summary: String, val hex: String)

// The app's state and logic, the same as the iOS example's MidiController.
class MidiController(application: Application) : AndroidViewModel(application) {
    private val bridge = RtMidiBridge()

    // RtMidi calls block (see RtMidiBridge), so they run here, one at a time
    // and in order, and their results are posted back to the main thread.
    private val midiThread = Executors.newSingleThreadExecutor()
    private val main = Handler(Looper.getMainLooper())

    val outputPorts = mutableStateListOf<String>()
    val inputPorts = mutableStateListOf<String>()
    var selectedOutputIndex by mutableStateOf<Int?>(null)
    var selectedInputIndex by mutableStateOf<Int?>(null)
    var openedOutputPortName by mutableStateOf<String?>(null)
    var openedInputPortName by mutableStateOf<String?>(null)
    val log = mutableStateListOf<LogLine>()
    val midiMonitor = mutableStateListOf<MidiMonitorLine>()
    var receivedCount by mutableStateOf(0)

    // Android reports MIDI devices arriving and leaving; refresh on either, so
    // an unplugged device's port is closed and a new device is listed.
    //
    // TODO(RtMidi 7.0.0): RtMidi itself should detect devices arriving and
    // leaving, on every backend, and tell the application. Until then each
    // application does it, as here; this and the "no longer connected" handling
    // in refreshPorts() move into RtMidi then.
    private val midiManager =
        application.getSystemService(Context.MIDI_SERVICE) as MidiManager
    private val deviceCallback = object : MidiManager.DeviceCallback() {
        override fun onDeviceAdded(device: MidiDeviceInfo) = refreshPorts()
        override fun onDeviceRemoved(device: MidiDeviceInfo) = refreshPorts()
    }

    init {
        bridge.receiveHandler = { data, _ -> main.post { handleReceived(data) } }
        // Deprecated from API 33 in favour of a form that also takes a
        // transport; this app supports API 29.
        @Suppress("DEPRECATION")
        midiManager.registerDeviceCallback(deviceCallback, main)
        refreshPorts()
    }

    override fun onCleared() {
        midiManager.unregisterDeviceCallback(deviceCallback)
        midiThread.shutdown()
    }

    fun refreshPorts() {
        midiThread.execute {
            val outs = bridge.outputPortNames().toList()
            val ins = bridge.inputPortNames().toList()
            main.post {
                outputPorts.clear(); outputPorts.addAll(outs)
                inputPorts.clear(); inputPorts.addAll(ins)
                appendLog("Ports refreshed: ${outs.size} out, ${ins.size} in")

                // Reopen what was open, by name: its index may have changed.
                // A port whose device has gone is closed instead. (To move into
                // RtMidi in 7.0.0 with device detection; see deviceCallback.)
                val outIndex = outs.indexOf(openedOutputPortName)
                selectedOutputIndex = outIndex.takeIf { it >= 0 }
                if (outIndex >= 0) openSelectedOutput()
                else openedOutputPortName?.let { gone ->
                    openedOutputPortName = null
                    midiThread.execute { bridge.closeOutputPort() }
                    appendLog("Closed output port: $gone is no longer connected")
                }

                val inIndex = ins.indexOf(openedInputPortName)
                selectedInputIndex = inIndex.takeIf { it >= 0 }
                if (inIndex >= 0) openSelectedInput()
                else openedInputPortName?.let { gone ->
                    openedInputPortName = null
                    midiThread.execute { bridge.closeInputPort() }
                    appendLog("Closed input port: $gone is no longer connected")
                }
            }
        }
    }

    // null is "None", which closes the port.
    fun selectOutput(index: Int?) {
        selectedOutputIndex = index
        if (index != null) openSelectedOutput()
        else if (openedOutputPortName != null) {
            openedOutputPortName = null
            midiThread.execute { bridge.closeOutputPort() }
            appendLog("Closed output port")
        }
    }

    fun selectInput(index: Int?) {
        selectedInputIndex = index
        if (index != null) openSelectedInput()
        else if (openedInputPortName != null) {
            openedInputPortName = null
            midiThread.execute { bridge.closeInputPort() }
            appendLog("Closed input port")
        }
    }

    private fun openSelectedOutput() {
        val i = selectedOutputIndex ?: return
        val name = outputPorts.getOrNull(i) ?: return
        midiThread.execute {
            val error = bridge.openOutputPort(i)
            main.post {
                if (error == null) {
                    openedOutputPortName = name
                    appendLog("Opened output port $i: $name")
                } else {
                    appendLog("Failed to open output port $i: $error")
                    // Show what is actually open: the previous port, or None.
                    selectedOutputIndex = outputPorts.indexOf(openedOutputPortName).takeIf { it >= 0 }
                }
            }
        }
    }

    private fun openSelectedInput() {
        val i = selectedInputIndex ?: return
        val name = inputPorts.getOrNull(i) ?: return
        midiThread.execute {
            val error = bridge.openInputPort(i)
            main.post {
                if (error == null) {
                    openedInputPortName = name
                    appendLog("Opened input port $i: $name")
                } else {
                    appendLog("Failed to open input port $i: $error")
                    // Show what is actually open: the previous port, or None.
                    selectedInputIndex = inputPorts.indexOf(openedInputPortName).takeIf { it >= 0 }
                }
            }
        }
    }

    fun sendNoteOn() = send(bytes(0x90, 0x3C, 0x60), "Note On")
    fun sendNoteOff() = send(bytes(0x80, 0x3C, 0x60), "Note Off")
    fun sendSysex() = send(bytes(0xF0, 0x7D, 0x01, 0x02, 0x03, 0xF7), "SysEx")

    // Universal Non-Realtime Identity Request, broadcast device ID.
    fun sendIdentityRequest() = send(bytes(0xF0, 0x7E, 0x7F, 0x06, 0x01, 0xF7), "Identity Request")

    // A long SysEx message, F0 7D <counting data> F7 (0x7D is the
    // non-commercial manufacturer ID), for checking that large messages
    // arrive whole.
    fun sendLargeSysex(size: Int) {
        val message = ByteArray(size) { (it and 0x7F).toByte() }
        message[0] = 0xF0.toByte()
        message[1] = 0x7D
        message[size - 1] = 0xF7.toByte()
        send(message, "SysEx")
    }

    // Asks for a SysEx message exactly `size` bytes long; only PZ-Echo-4x4 answers (see README).
    fun requestDump(size: Int) = send(
        bytes(0xF0, 0x7D, 0x50, 0x5A, 0x01,
              (size shr 14) and 0x7F, (size shr 7) and 0x7F, size and 0x7F, 0xF7),
        "request for a $size-byte SysEx dump")

    fun clearMonitor() {
        midiMonitor.clear()
        receivedCount = 0
    }

    fun appendLog(text: String) {
        log.add(LogLine(Date(), text))
        if (log.size > 500) log.removeRange(0, log.size - 500)
    }

    private fun send(bytes: ByteArray, label: String) {
        midiThread.execute {
            val error = bridge.sendBytes(bytes)
            main.post {
                appendLog(if (error == null) "Sent $label (${bytes.size} bytes)"
                          else "Failed to send $label: $error")
            }
        }
    }

    private fun handleReceived(data: ByteArray) {
        receivedCount += 1
        verifyDump(data)?.let { appendLog(it) }
        midiMonitor.add(MidiMonitorLine(Date(), summarize(data), hex(data)))
        if (midiMonitor.size > 500) midiMonitor.removeRange(0, midiMonitor.size - 500)
    }

    companion object {
        private fun bytes(vararg values: Int) = ByteArray(values.size) { values[it].toByte() }

        // Long messages are cut short: the summary already gives the length.
        private fun hex(data: ByteArray): String {
            val shown = data.take(48).joinToString(" ") { "%02X".format(it) }
            return if (data.size > 48) "$shown …" else shown
        }

        // Checks a requestDump() reply (payload byte k = k & 0x7F); null for any other message.
        private fun verifyDump(data: ByteArray): String? {
            val header = bytes(0xF0, 0x7D, 0x50, 0x5A, 0x02)
            if (data.size < header.size + 1 || !data.copyOfRange(0, header.size).contentEquals(header))
                return null
            if (data.last() != 0xF7.toByte())
                return "Dump: ${data.size} bytes, no F7 at the end"
            for (k in 0 until data.size - header.size - 1)
                if (data[header.size + k].toInt() != (k and 0x7F))
                    return "Dump: ${data.size} bytes, payload wrong at offset $k"
            return "Dump: ${data.size} bytes, every byte correct"
        }

        private fun summarize(data: ByteArray): String {
            if (data.isEmpty()) return "empty"
            val status = data[0].toInt() and 0xFF
            val ch = (status and 0x0F) + 1
            return when (status) {
                0xF0 -> parseIdentityReply(data) ?: "SysEx (${data.size}B)"
                in 0x80..0x8F -> "Note Off ch$ch"
                in 0x90..0x9F -> "Note On ch$ch"
                in 0xA0..0xAF -> "Poly Aftertouch ch$ch"
                in 0xB0..0xBF -> "Control Change ch$ch"
                in 0xC0..0xCF -> "Program Change ch$ch"
                in 0xD0..0xDF -> "Channel Aftertouch ch$ch"
                in 0xE0..0xEF -> "Pitch Bend ch$ch"
                0xF8 -> "Timing Clock"
                0xFA -> "Start"
                0xFB -> "Continue"
                0xFC -> "Stop"
                0xFE -> "Active Sensing"
                else -> "Status 0x%02X".format(status)
            }
        }

        // F0 7E <devID> 06 02 <manufacturer> <family LSB/MSB> <member LSB/MSB>
        // <4-byte software revision> F7.
        private fun parseIdentityReply(data: ByteArray): String? {
            val b = data.map { it.toInt() and 0xFF }
            if (b.size < 6 || b[0] != 0xF0 || b[1] != 0x7E || b[3] != 0x06 || b[4] != 0x02) return null

            val deviceId = b[2]
            var idx = 5
            val manufacturer: List<Int>
            if (b[idx] == 0x00) {
                if (b.size < idx + 3) return null
                manufacturer = b.subList(idx, idx + 3)
                idx += 3
            } else {
                manufacturer = listOf(b[idx])
                idx += 1
            }
            if (b.size < idx + 9) return null

            val family = b[idx] or (b[idx + 1] shl 7)
            val member = b[idx + 2] or (b[idx + 3] shl 7)
            val rev = b.subList(idx + 4, idx + 8).joinToString(".")
            val manuHex = manufacturer.joinToString(" ") { "%02X".format(it) }

            return "ID Reply: manufacturer=$manuHex family=$family member=$member rev=$rev (devID $deviceId)"
        }
    }
}
