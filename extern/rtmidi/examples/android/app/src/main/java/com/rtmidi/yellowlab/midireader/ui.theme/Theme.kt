package com.rtmidi.yellowlab.midireader.ui.theme

import android.app.Activity
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.darkColorScheme
import androidx.compose.material3.lightColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.runtime.SideEffect
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.toArgb
import androidx.compose.ui.platform.LocalView
import androidx.core.view.WindowCompat

// background is the grouped form background, surface a form cell.
private val LightColorScheme = lightColorScheme(
    primary = SystemBlue,
    background = GroupedBackground,
    surface = Color.White,
    onSurfaceVariant = SecondaryLabel,
    outlineVariant = Separator,
)

private val DarkColorScheme = darkColorScheme(
    primary = SystemBlueDark,
    background = GroupedBackgroundDark,
    surface = CellBackgroundDark,
    onSurfaceVariant = SecondaryLabel,
    outlineVariant = SeparatorDark,
)

@Composable
fun MidiReaderTheme(
    darkTheme: Boolean = isSystemInDarkTheme(),
    content: @Composable () -> Unit
) {
    val colorScheme = if (darkTheme) DarkColorScheme else LightColorScheme
    val view = LocalView.current
    if (!view.isInEditMode) {
        SideEffect {
            val window = (view.context as Activity).window
            window.statusBarColor = colorScheme.surface.toArgb()
            // Dark icons on a light bar.
            WindowCompat.getInsetsController(window, view).isAppearanceLightStatusBars = !darkTheme
        }
    }

    MaterialTheme(
        colorScheme = colorScheme,
        typography = Typography,
        content = content
    )
}
