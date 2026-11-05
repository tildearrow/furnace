package com.rtmidi.yellowlab.midireader

import android.content.Intent
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.viewModels
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.ColumnScope
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ArrowDropDown
import androidx.compose.material.icons.filled.Check
import androidx.compose.material.icons.filled.Refresh
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.material3.TopAppBar
import androidx.compose.material3.TopAppBarDefaults
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.rtmidi.yellowlab.midireader.ui.theme.MidiReaderTheme
import com.yellowlab.rtmidi.RtMidiUsbPermission
import java.text.DateFormat

class MainActivity : ComponentActivity() {
    private val midi by viewModels<MidiController>()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            MidiReaderTheme {
                ContentView(midi, onAllowUsbAccess = ::allowUsbAccess)
            }
        }
    }

    // USB access is optional: MIDI works without it. With it, RtMidi reads each
    // USB device's own port names, and on Android 12 and earlier its true input
    // count. RtMidi never asks by itself; see contrib/java/RtMidiUsbPermission.java.
    private fun allowUsbAccess() {
        RtMidiUsbPermission.request(this) { granted, denied ->
            midi.appendLog(if (granted + denied == 0) "No USB MIDI device needs access"
                           else "USB access: $granted allowed, $denied refused")
            midi.refreshPorts()
        }
    }

    // A USB MIDI device was plugged in while the app was open (singleTop).
    override fun onNewIntent(intent: Intent) {
        super.onNewIntent(intent)
        midi.refreshPorts()
    }
}

// The same layout as the iOS example: controls on the left; the MIDI monitor
// over the status log on the right, as separate panels on the form background.
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ContentView(midi: MidiController, onAllowUsbAccess: () -> Unit) {
    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text("RtMidi Example", fontWeight = FontWeight.Bold) },
                actions = {
                    IconButton(onClick = { midi.refreshPorts() }) {
                        Icon(Icons.Filled.Refresh, contentDescription = "Refresh",
                             tint = MaterialTheme.colorScheme.primary)
                    }
                },
                colors = TopAppBarDefaults.topAppBarColors(
                    containerColor = MaterialTheme.colorScheme.surface),
            )
        }
    ) { padding ->
        Row(Modifier.padding(padding).fillMaxSize()
                .background(MaterialTheme.colorScheme.background)) {
            ControlsArea(midi, onAllowUsbAccess, Modifier.weight(1f).fillMaxHeight())
            // The form's sections stop PanelGap short of this column, so the
            // panels below sit the same distance from them as from each other.
            Column(Modifier.weight(1f).fillMaxHeight()
                       .padding(top = PanelGap, end = PanelGap, bottom = PanelGap)) {
                Panel(Modifier.weight(2f)) { MonitorArea(midi, Modifier.fillMaxSize()) }
                Spacer(Modifier.height(PanelGap))
                Panel(Modifier.weight(1f)) { StatusLogArea(midi, Modifier.fillMaxSize()) }
            }
        }
    }
}

@Composable
fun ControlsArea(midi: MidiController, onAllowUsbAccess: () -> Unit, modifier: Modifier) {
    Column(modifier.background(MaterialTheme.colorScheme.background)
                   .verticalScroll(rememberScrollState())) {
        FormSection("Input port" + (midi.openedInputPortName?.let { ": $it" } ?: "")) {
            PickerRow("Input", midi.inputPorts, midi.selectedInputIndex, midi::selectInput)
        }
        FormSection("Output port" + (midi.openedOutputPortName?.let { ": $it" } ?: "")) {
            PickerRow("Output", midi.outputPorts, midi.selectedOutputIndex, midi::selectOutput)
        }
        FormSection("Send") {
            ButtonRow("Note On") { midi.sendNoteOn() }
            ButtonRow("Note Off") { midi.sendNoteOff() }
            ButtonRow("SysEx") { midi.sendSysex() }
            ButtonRow("Identity Request", last = true) { midi.sendIdentityRequest() }
        }
        val echo = BuildConfig.ENABLE_PROTOZOA_4x4_ECHO
        FormSection("Large SysEx",
                    footer = "Checks that long messages arrive whole. 66312 bytes is " +
                             "a real device dump, measured on Linux ALSA." +
                             if (echo) " A dump request asks the device to send that much " +
                                       "back; only the PZ-Echo-4x4 test firmware answers." else "") {
            ButtonRow("SysEx 8192 bytes") { midi.sendLargeSysex(8192) }
            ButtonRow("SysEx 66312 bytes", last = !echo) { midi.sendLargeSysex(66312) }
            if (echo) {
                ButtonRow("Request 8192-byte dump") { midi.requestDump(8192) }
                ButtonRow("Request 66312-byte dump", last = true) { midi.requestDump(66312) }
            }
        }
        FormSection("USB access",
                    footer = "Optional: MIDI works without it. With it, RtMidi shows " +
                             "each USB device's own port names.") {
            ButtonRow("Allow USB access", last = true, onClick = onAllowUsbAccess)
        }
        Spacer(Modifier.height(16.dp))
    }
}

@Composable
fun MonitorArea(midi: MidiController, modifier: Modifier) {
    Column(modifier) {
        Row(Modifier.fillMaxWidth().padding(start = 16.dp, end = 4.dp),
            verticalAlignment = Alignment.CenterVertically) {
            Text("MIDI Monitor", fontSize = 17.sp, fontWeight = FontWeight.SemiBold)
            Spacer(Modifier.width(8.dp))
            Text("${midi.receivedCount} received", fontSize = 12.sp,
                 color = MaterialTheme.colorScheme.onSurfaceVariant)
            Spacer(Modifier.weight(1f))
            TextButton(onClick = { midi.clearMonitor() }) { Text("Clear", fontSize = 12.sp) }
        }

        if (midi.midiMonitor.isEmpty()) {
            Box(Modifier.fillMaxSize().padding(16.dp), contentAlignment = Alignment.Center) {
                Text("No MIDI received yet. Select an input port and send from the attached device.",
                     fontSize = 13.sp, textAlign = TextAlign.Center,
                     color = MaterialTheme.colorScheme.onSurfaceVariant)
            }
        } else {
            LazyColumn {
                items(midi.midiMonitor.asReversed()) { line ->
                    Row(Modifier.padding(horizontal = 16.dp, vertical = 6.dp)) {
                        Text(timeFormat.format(line.timestamp), fontSize = 11.sp,
                             color = MaterialTheme.colorScheme.onSurfaceVariant,
                             modifier = Modifier.width(76.dp))
                        Column {
                            Text(line.summary, fontSize = 13.sp, fontFamily = FontFamily.Monospace,
                                 fontWeight = FontWeight.Bold)
                            Text(line.hex, fontSize = 11.sp, fontFamily = FontFamily.Monospace,
                                 color = MaterialTheme.colorScheme.onSurfaceVariant,
                                 maxLines = 2, overflow = TextOverflow.Ellipsis)
                        }
                    }
                    HorizontalDivider(Modifier.padding(start = 16.dp))
                }
            }
        }
    }
}

@Composable
fun StatusLogArea(midi: MidiController, modifier: Modifier) {
    Column(modifier) {
        Text("Status", fontSize = 12.sp, color = MaterialTheme.colorScheme.onSurfaceVariant,
             modifier = Modifier.padding(start = 16.dp, top = 4.dp))
        LazyColumn {
            items(midi.log.asReversed()) { line ->
                Text(line.text, fontSize = 11.sp, fontFamily = FontFamily.Monospace,
                     modifier = Modifier.padding(horizontal = 16.dp, vertical = 6.dp))
                HorizontalDivider(Modifier.padding(start = 16.dp))
            }
        }
    }
}

private val timeFormat: DateFormat = DateFormat.getTimeInstance(DateFormat.MEDIUM)

//
// Form pieces, after SwiftUI's Form: a grouped list of rounded sections, each
// with a small uppercase header.
//

@Composable
fun FormSection(title: String, footer: String? = null, content: @Composable ColumnScope.() -> Unit) {
    Text(title.uppercase(), fontSize = 13.sp, color = MaterialTheme.colorScheme.onSurfaceVariant,
         maxLines = 1, overflow = TextOverflow.Ellipsis,
         modifier = Modifier.padding(start = 32.dp, end = 16.dp, top = 20.dp, bottom = 6.dp))
    Surface(shape = RoundedCornerShape(10.dp), color = MaterialTheme.colorScheme.surface,
            modifier = Modifier.padding(horizontal = 16.dp).fillMaxWidth()) {
        Column(content = content)
    }
    if (footer != null) {
        Text(footer, fontSize = 13.sp, color = MaterialTheme.colorScheme.onSurfaceVariant,
             modifier = Modifier.padding(start = 32.dp, end = 16.dp, top = 6.dp))
    }
}

// A label on the left and the current choice on the right; tapping opens the
// choices, with "None" first.
@Composable
fun PickerRow(label: String, options: List<String>, selected: Int?, onSelect: (Int?) -> Unit) {
    var expanded by remember { mutableStateOf(false) }
    Box {
        Row(Modifier.fillMaxWidth().clickable { expanded = true }
                .padding(start = 16.dp, end = 8.dp, top = 12.dp, bottom = 12.dp),
            verticalAlignment = Alignment.CenterVertically) {
            Text(label, fontSize = 17.sp)
            Text(selected?.let { options.getOrNull(it) } ?: "None", fontSize = 17.sp,
                 color = MaterialTheme.colorScheme.onSurfaceVariant, textAlign = TextAlign.End,
                 maxLines = 1, overflow = TextOverflow.Ellipsis,
                 modifier = Modifier.weight(1f).padding(start = 16.dp))
            Icon(Icons.Filled.ArrowDropDown, contentDescription = null,
                 tint = MaterialTheme.colorScheme.onSurfaceVariant)
        }
        DropdownMenu(expanded = expanded, onDismissRequest = { expanded = false }) {
            (listOf<String?>(null) + options).forEachIndexed { n, name ->
                val index = if (n == 0) null else n - 1
                DropdownMenuItem(
                    text = { Text(name ?: "None") },
                    onClick = { expanded = false; onSelect(index) },
                    trailingIcon = { if (index == selected) Icon(Icons.Filled.Check, null) },
                )
            }
        }
    }
}

// A form button: a full-width row with its title in the accent colour.
@Composable
fun ButtonRow(title: String, last: Boolean = false, onClick: () -> Unit) {
    Text(title, fontSize = 17.sp, color = MaterialTheme.colorScheme.primary,
         modifier = Modifier.fillMaxWidth().clickable(onClick = onClick)
             .padding(horizontal = 16.dp, vertical = 12.dp))
    if (!last) HorizontalDivider(Modifier.padding(start = 16.dp))
}

// Hairline dividers in the separator colour. Material 3's Divider is
// horizontal only in this Compose version.
@Composable
fun HorizontalDivider(modifier: Modifier = Modifier) {
    Box(modifier.fillMaxWidth().height(0.5.dp).background(MaterialTheme.colorScheme.outlineVariant))
}

private val PanelGap = 16.dp

@Composable
fun Panel(modifier: Modifier, content: @Composable () -> Unit) {
    Surface(modifier.fillMaxWidth(), shape = RoundedCornerShape(10.dp),
            color = MaterialTheme.colorScheme.surface, content = content)
}
