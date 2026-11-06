import Foundation
import AVFoundation

struct LogLine: Identifiable {
    let id = UUID()
    let timestamp: Date
    let text: String
}

struct MidiMonitorLine: Identifiable {
    let id = UUID()
    let timestamp: Date
    let summary: String
    let hex: String
}

@MainActor
final class MidiController: ObservableObject {
    // Constructing RtMidiOut/RtMidiIn calls MIDIClientCreate(), which must
    // not happen inside a @StateObject's synchronous init (it races
    // SwiftUI's own view-graph construction). Created lazily in start().
    private var bridge: RtMidiBridge?

    @Published var outputPorts: [String] = []
    @Published var inputPorts: [String] = []
    @Published var selectedOutputIndex: Int? = nil
    @Published var selectedInputIndex: Int? = nil
    @Published var openedOutputPortName: String? = nil
    @Published var openedInputPortName: String? = nil
    @Published var log: [LogLine] = []
    @Published var midiMonitor: [MidiMonitorLine] = []
    @Published var receivedCount = 0

    func start() {
        let b = RtMidiBridge()
        bridge = b
        b.setReceiveHandler { [weak self] data, _ in
            self?.handleReceived(data)
        }

        // CoreMIDI I/O on iOS needs an active audio session.
        do {
            let session = AVAudioSession.sharedInstance()
            try session.setCategory(.playback, options: [.mixWithOthers])
            try session.setActive(true)
        } catch {
            appendLog("AVAudioSession setup failed: \(error.localizedDescription)")
        }

        refreshPorts()
    }

    func refreshPorts() {
        guard let bridge else { return }
        outputPorts = bridge.outputPortNames()
        inputPorts = bridge.inputPortNames()
        appendLog("Ports refreshed: \(outputPorts.count) out, \(inputPorts.count) in")

        if let name = openedOutputPortName, let idx = outputPorts.firstIndex(of: name) {
            selectedOutputIndex = idx
            openSelectedOutput()
        } else {
            openedOutputPortName = nil
        }

        if let name = openedInputPortName, let idx = inputPorts.firstIndex(of: name) {
            selectedInputIndex = idx
            openSelectedInput()
        } else {
            openedInputPortName = nil
        }
    }

    func openSelectedOutput() {
        guard let i = selectedOutputIndex, let bridge, outputPorts.indices.contains(i) else { return }
        do {
            try bridge.openOutputPort(at: UInt(i))
            openedOutputPortName = outputPorts[i]
            appendLog("Opened output port \(i): \(outputPorts[i])")
        } catch {
            appendLog("Failed to open output port \(i): \(error.localizedDescription)")
        }
    }

    func openSelectedInput() {
        guard let i = selectedInputIndex, let bridge, inputPorts.indices.contains(i) else { return }
        do {
            try bridge.openInputPort(at: UInt(i))
            openedInputPortName = inputPorts[i]
            appendLog("Opened input port \(i): \(inputPorts[i])")
        } catch {
            appendLog("Failed to open input port \(i): \(error.localizedDescription)")
        }
    }

    func closeOutput() {
        bridge?.closeOutputPort()
        if let name = openedOutputPortName { appendLog("Closed output port: \(name)") }
        openedOutputPortName = nil
    }

    func closeInput() {
        bridge?.closeInputPort()
        if let name = openedInputPortName { appendLog("Closed input port: \(name)") }
        openedInputPortName = nil
    }

    func sendNoteOn() { send([0x90, 0x3C, 0x60], label: "Note On") }
    func sendNoteOff() { send([0x80, 0x3C, 0x60], label: "Note Off") }
    func sendSysex() { send([0xF0, 0x7D, 0x01, 0x02, 0x03, 0xF7], label: "SysEx") }

    // Universal Non-Realtime Identity Request, broadcast device ID.
    func sendIdentityRequest() { send([0xF0, 0x7E, 0x7F, 0x06, 0x01, 0xF7], label: "Identity Request") }

    func clearMonitor() {
        midiMonitor.removeAll()
        receivedCount = 0
        log.removeAll()
    }

    private func send(_ bytes: [UInt8], label: String) {
        guard let bridge else { return }
        bridge.sendBytes(Data(bytes))
        appendLog("Sent \(label) (\(bytes.count) bytes)")
    }

    private func handleReceived(_ data: Data) {
        receivedCount += 1
        let hex = data.map { String(format: "%02X", $0) }.joined(separator: " ")
        midiMonitor.append(MidiMonitorLine(timestamp: Date(), summary: Self.summarize(data), hex: hex))
        if midiMonitor.count > 500 { midiMonitor.removeFirst(midiMonitor.count - 500) }
    }

    private static func summarize(_ data: Data) -> String {
        guard let status = data.first else { return "empty" }
        switch status {
        case 0xF0: return parseIdentityReply(data) ?? "SysEx (\(data.count)B)"
        case 0x80...0x8F: return "Note Off ch\((status & 0x0F) + 1)"
        case 0x90...0x9F: return "Note On ch\((status & 0x0F) + 1)"
        case 0xA0...0xAF: return "Poly Aftertouch ch\((status & 0x0F) + 1)"
        case 0xB0...0xBF: return "Control Change ch\((status & 0x0F) + 1)"
        case 0xC0...0xCF: return "Program Change ch\((status & 0x0F) + 1)"
        case 0xD0...0xDF: return "Channel Aftertouch ch\((status & 0x0F) + 1)"
        case 0xE0...0xEF: return "Pitch Bend ch\((status & 0x0F) + 1)"
        case 0xF8: return "Timing Clock"
        case 0xFA: return "Start"
        case 0xFB: return "Continue"
        case 0xFC: return "Stop"
        case 0xFE: return "Active Sensing"
        default: return String(format: "Status 0x%02X", status)
        }
    }

    // F0 7E <devID> 06 02 <manufacturer> <family LSB/MSB> <member LSB/MSB>
    // <4-byte software revision> F7.
    private static func parseIdentityReply(_ data: Data) -> String? {
        let bytes = [UInt8](data)
        guard bytes.count >= 6, bytes[0] == 0xF0, bytes[1] == 0x7E, bytes[3] == 0x06, bytes[4] == 0x02 else {
            return nil
        }

        let deviceId = bytes[2]
        var idx = 5
        let manufacturer: [UInt8]
        if idx < bytes.count, bytes[idx] == 0x00 {
            guard bytes.count >= idx + 3 else { return nil }
            manufacturer = Array(bytes[idx...idx + 2])
            idx += 3
        } else {
            guard idx < bytes.count else { return nil }
            manufacturer = [bytes[idx]]
            idx += 1
        }
        guard bytes.count >= idx + 9 else { return nil }

        let family = Int(bytes[idx]) | (Int(bytes[idx + 1]) << 7)
        let member = Int(bytes[idx + 2]) | (Int(bytes[idx + 3]) << 7)
        let rev = bytes[(idx + 4)..<(idx + 8)].map { String($0) }.joined(separator: ".")
        let manuHex = manufacturer.map { String(format: "%02X", $0) }.joined(separator: " ")

        return "ID Reply: manufacturer=\(manuHex) family=\(family) member=\(member) rev=\(rev) (devID \(deviceId))"
    }

    private func appendLog(_ text: String) {
        log.append(LogLine(timestamp: Date(), text: text))
        if log.count > 500 { log.removeFirst(log.count - 500) }
    }
}
