package com.rtmidi.yellowlab.midireader

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.viewModels
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.IntrinsicSize
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.layout.wrapContentSize
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.CutCornerShape
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults.TextButtonContentPadding
import androidx.compose.material3.Divider
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.runtime.getValue
import androidx.compose.runtime.setValue
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import com.rtmidi.yellowlab.midireader.ui.theme.MidiReaderTheme
import androidx.lifecycle.viewmodel.compose.viewModel

class MainActivity : ComponentActivity() {
    private val viewModel by viewModels<MidiViewModel>()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setContent {
            MidiReaderTheme {
                Surface(
                    modifier = Modifier.fillMaxSize(),
                    color = MaterialTheme.colorScheme.background
                ) {
                    Column() {
                        midiPortSelection(viewModel.ports)
                        showEvents(events = viewModel.events)
                    }
                }
            }
        }
    }
}

@Composable
fun midiPortSelection(ports : List<String>) {
    val viewModel = viewModel<MidiViewModel>()

    Row(verticalAlignment = Alignment.CenterVertically,
        modifier = Modifier.height(IntrinsicSize.Max)) {
        Text(text = "Midi: ")
        midiPortsDropDown(ports, Modifier.weight(1f))
        Button(
            shape = CutCornerShape(10),
            contentPadding = TextButtonContentPadding,
            onClick = {
                viewModel.enumerateMidiPorts()
            }) {
            Text(text = "Refresh")
        }
    }
}

@Composable
fun midiPortsDropDown(ports: List<String>, modifier: Modifier = Modifier) {
    val viewModel = viewModel<MidiViewModel>()
    var expanded by remember { mutableStateOf(false) }
    var selectedIndex by remember { mutableStateOf(0) }

    Box(modifier = modifier.wrapContentSize(Alignment.TopStart)) {
        Text(ports[selectedIndex],modifier = Modifier
            .fillMaxWidth()
            .clickable(onClick = { expanded = true }))
        DropdownMenu(
            expanded = expanded,
            onDismissRequest = { expanded = false }
        ) {
            ports.forEachIndexed { index, s ->
                DropdownMenuItem(text = { Text(text = s) }, onClick = {
                    selectedIndex = index
                    expanded = false
                    viewModel.selectPort(selectedIndex)
                })
            }
        }
    }
}

@Composable
fun showEvents(events: List<MidiEvent>) {
    LazyColumn(modifier = Modifier
        .fillMaxHeight()
        .padding(5.dp)) {
        items(events) {
            Row(modifier = Modifier.height(IntrinsicSize.Max)) {
                Text(text = it.time.toString(), modifier = Modifier.weight(0.6F), overflow = TextOverflow.Clip)
                Divider(modifier = Modifier.fillMaxHeight().width(1.dp))
                Text(text = it.data, modifier = Modifier.weight(0.4F))
            }
        }
    }
}