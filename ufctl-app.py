#!/usr/bin/env python3
import subprocess
import evdev

def find_magic_keyboard():
    devices = [evdev.InputDevice(path) for path in evdev.list_devices()]
    for device in devices:
        if "Magic Keyboard" in device.name:
            return device

def main():
    magic_keyboard = find_magic_keyboard()
    if magic_keyboard is None:
        print("ERROR: Apple Magic Keyboard not found.")
        return

    for event in magic_keyboard.read_loop():
        if event.type == evdev.ecodes.EV_KEY:
            if event.code == evdev.ecodes.KEY_BRIGHTNESSDOWN:
                subprocess.run(["ufctl", "-"])
            elif event.code == evdev.ecodes.KEY_BRIGHTNESSUP:
                subprocess.run(["ufctl", "+"])

if __name__ == "__main__":
    main()