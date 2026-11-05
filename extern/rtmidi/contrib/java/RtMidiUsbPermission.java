package com.yellowlab.rtmidi;

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbInterface;
import android.hardware.usb.UsbManager;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;

import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.List;

/**
 * Optional.  Asks the user for USB access to the attached USB MIDI devices.
 *
 * MIDI input and output need no permission: they go through Android's MIDI
 * service.  USB access lets RtMidi read a device's own descriptors, which gives
 * it the device's port names ("SoftStep Control Surface" rather than
 * "SoftStep MIDI 1"), and on Android 12 and earlier the true number of input
 * ports, so that it can refuse to open a device that would hang the system
 * instead of hanging.  RtMidi only reads the descriptors and never asks for
 * access itself: the grant covers the whole device, so asking is the
 * application's decision.
 *
 * A grant made through {@link #request} lasts until the device is unplugged.
 * For a grant that lasts, declare an activity that handles
 * {@code android.hardware.usb.action.USB_DEVICE_ATTACHED} with a device filter
 * for USB MIDI (class 1, subclass 3); the system's dialog then offers to
 * remember the choice.  See examples/android.
 *
 * After a grant, enumerate ports again (getPortCount()) to see the names.
 */
public final class RtMidiUsbPermission {

    /** Called on the main thread once every device has been answered. */
    public interface Callback {
        void onComplete(int granted, int denied);
    }

    private RtMidiUsbPermission() {}

    /** True for a USB device with a MIDI Streaming interface. */
    public static boolean isMidiDevice(UsbDevice device) {
        for (int i = 0; i < device.getInterfaceCount(); i++) {
            UsbInterface iface = device.getInterface(i);
            if (iface.getInterfaceClass() == 1 && iface.getInterfaceSubclass() == 3)
                return true;
        }
        return false;
    }

    /** The attached USB MIDI devices this application has no access to yet. */
    public static List<UsbDevice> devicesWithoutPermission(Context context) {
        UsbManager usb = (UsbManager) context.getSystemService(Context.USB_SERVICE);
        List<UsbDevice> result = new ArrayList<>();
        if (usb == null) return result;
        for (UsbDevice device : usb.getDeviceList().values()) {
            if (isMidiDevice(device) && !usb.hasPermission(device)) result.add(device);
        }
        return result;
    }

    /**
     * Asks for access to every attached USB MIDI device that does not have it
     * yet, one system dialog at a time.  Devices that already have access are
     * not asked about and are not counted.
     */
    public static void request(Context context, Callback callback) {
        new Session(context.getApplicationContext(), devicesWithoutPermission(context),
                    callback).next();
    }

    private static final class Session extends BroadcastReceiver {
        private final Context context;
        private final UsbManager usb;
        private final ArrayDeque<UsbDevice> pending;
        private final Callback callback;
        private final String action;
        private final Handler main = new Handler(Looper.getMainLooper());
        private boolean registered = false;
        private int granted = 0;
        private int denied = 0;

        Session(Context context, List<UsbDevice> devices, Callback callback) {
            this.context = context;
            this.usb = (UsbManager) context.getSystemService(Context.USB_SERVICE);
            this.pending = new ArrayDeque<>(devices);
            this.callback = callback;
            this.action = context.getPackageName() + ".RTMIDI_USB_PERMISSION";
        }

        void next() {
            if (pending.isEmpty()) {
                finish();
                return;
            }
            if (!registered) {
                IntentFilter filter = new IntentFilter(action);
                // The answer comes back through our own PendingIntent, so no
                // other application needs to reach this receiver.
                if (Build.VERSION.SDK_INT >= 33)
                    context.registerReceiver(this, filter, Context.RECEIVER_NOT_EXPORTED);
                else
                    context.registerReceiver(this, filter);
                registered = true;
            }
            // The system adds the answer to this intent as extras, so from
            // Android 12 it must be mutable; naming the package makes it
            // explicit, which Android 14 requires of a mutable PendingIntent.
            int flags = Build.VERSION.SDK_INT >= 31 ? PendingIntent.FLAG_MUTABLE : 0;
            Intent intent = new Intent(action).setPackage(context.getPackageName());
            usb.requestPermission(pending.peekFirst(),
                                  PendingIntent.getBroadcast(context, 0, intent, flags));
        }

        @Override
        public void onReceive(Context c, Intent intent) {
            pending.pollFirst();
            if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) granted++;
            else denied++;
            next();
        }

        private void finish() {
            if (registered) {
                context.unregisterReceiver(this);
                registered = false;
            }
            if (callback != null) {
                // Always asynchronous, on the main thread, even with nothing to ask.
                final int g = granted, d = denied;
                main.post(() -> callback.onComplete(g, d));
            }
        }
    }
}
