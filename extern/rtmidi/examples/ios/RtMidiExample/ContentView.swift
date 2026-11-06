import SwiftUI

struct ContentView: View {
    @StateObject private var midi = MidiController()

    // -1 = no selection. A Picker bound directly to an Optional selection
    // crashes on-device (AttributeGraph); these sentinels avoid that.
    @State private var outputSelection = -1
    @State private var inputSelection = -1

    var body: some View {
        NavigationStack {
            GeometryReader { geo in
                HStack(spacing: 0) {
                    controlsArea
                        .frame(width: geo.size.width / 2)
                    Divider()
                    VStack(spacing: 0) {
                        monitorArea
                            .frame(height: geo.size.height * 2 / 3)
                        Divider()
                        statusLogArea
                            .frame(height: geo.size.height / 3)
                    }
                    .frame(width: geo.size.width / 2)
                }
            }
            .navigationTitle("RtMidi Example")
            .toolbar {
                ToolbarItem(placement: .navigationBarTrailing) {
                    Button {
                        midi.refreshPorts()
                    } label: {
                        Label("Refresh", systemImage: "arrow.clockwise")
                    }
                }
            }
        }
        .onAppear {
            // Deferred past the initial view-graph construction; see the
            // note on MidiController.bridge.
            DispatchQueue.main.async { midi.start() }
        }
    }

    private var controlsArea: some View {
        Form {
            Section {
                Picker("Input", selection: $inputSelection) {
                    Text("None").tag(-1)
                    ForEach(Array(midi.inputPorts.enumerated()), id: \.offset) { i, name in
                        Text(name).tag(i)
                    }
                }
                .onChange(of: inputSelection) { newValue in
                    midi.selectedInputIndex = newValue >= 0 ? newValue : nil
                    if newValue >= 0 { midi.openSelectedInput() } else { midi.closeInput() }
                }

                Picker("Output", selection: $outputSelection) {
                    Text("None").tag(-1)
                    ForEach(Array(midi.outputPorts.enumerated()), id: \.offset) { i, name in
                        Text(name).tag(i)
                    }
                }
                .onChange(of: outputSelection) { newValue in
                    midi.selectedOutputIndex = newValue >= 0 ? newValue : nil
                    if newValue >= 0 { midi.openSelectedOutput() } else { midi.closeOutput() }
                }
            }

            Section("Send") {
                Button("Note On") { midi.sendNoteOn() }
                Button("Note Off") { midi.sendNoteOff() }
                Button("SysEx") { midi.sendSysex() }
                Button("Identity Request") { midi.sendIdentityRequest() }
            }
        }
    }

    private var statusLogArea: some View {
        VStack(spacing: 0) {
            Text("Status")
                .font(.headline)
                .frame(maxWidth: .infinity, alignment: .leading)
                .padding(.horizontal)
                .padding(.top, 4)

            List(midi.log.reversed()) { line in
                Text(line.text)
                    .font(.system(.caption2, design: .monospaced))
            }
            .listStyle(.plain)
        }
    }

    private var monitorArea: some View {
        VStack(spacing: 0) {
            HStack {
                Text("MIDI Monitor")
                    .font(.headline)
                Text("\(midi.receivedCount) received")
                    .font(.caption)
                    .foregroundStyle(.secondary)
                Spacer()
                Button("Clear") { midi.clearMonitor() }
                    .font(.caption)
            }
            .padding(.horizontal)
            .padding(.vertical, 6)

            if midi.midiMonitor.isEmpty {
                Spacer()
                Text("No MIDI received yet. Select an input port and send from the attached device.")
                    .font(.footnote)
                    .foregroundStyle(.secondary)
                    .multilineTextAlignment(.center)
                    .padding()
                Spacer()
            } else {
                List(midi.midiMonitor.reversed()) { line in
                    HStack(alignment: .firstTextBaseline) {
                        Text(line.timestamp.formatted(date: .omitted, time: .standard))
                            .font(.caption2)
                            .foregroundStyle(.secondary)
                            .frame(width: 70, alignment: .leading)
                        VStack(alignment: .leading, spacing: 1) {
                            Text(line.summary)
                                .font(.system(.footnote, design: .monospaced))
                                .bold()
                            Text(line.hex)
                                .font(.system(.caption2, design: .monospaced))
                                .foregroundStyle(.secondary)
                                .lineLimit(2)
                        }
                    }
                }
                .listStyle(.plain)
            }
        }
    }
}

#Preview {
    ContentView()
}
