# Opta Material Transport Firmware

Embedded C++ firmware for an Arduino Opta PLC that controls an industrial material-transport machine. The system implements a finite-state machine for forward, stop, pause, and reverse operation, integrates physical pushbuttons, relay outputs, and LED indicators with debouncing and safety interlocks, and exposes a Wi‑Fi Access Point plus a lightweight web UI for real-time timer configuration.

## Highlights
- Finite-state machine for motion sequencing (forward, pause, reverse)
- Pushbutton inputs with debounce logic and interlocks
- Relay outputs and multi-LED status indicators
- On-device Wi‑Fi AP with web page to tune motion timers

## Hardware
- Arduino Opta PLC (Finder/Arduino Pro)
- Industrial actuators driven by Opta relays
- Physical pushbuttons and status LEDs

> Additional board and wiring documents will be added in this repository.

## Wi‑Fi Configuration (Defaults)
- SSID: `Opta_WiFi_AP`
- Password: `12345678`
- Static IP: `192.168.1.144`

## Web Interface
- Connect to the Wi‑Fi AP
- Open a browser to `http://192.168.1.144/`
- Update `Timer1` and `Timer2` values (milliseconds)

## I/O Mapping (from firmware)
- `A0`: Button 1 (forward)
- `A1`: Button 2 (reverse)
- `PE_4`: Wi‑Fi toggle button (press and hold ~1s)
- `D0`, `D1`: Relay outputs
- `LED_D0`..`LED_D3`, `LED_USER`: Status LEDs

## Build & Upload
Use Arduino IDE or Arduino CLI with the Opta board package installed, then build and upload `opta_firmware.ino`.

## Notes
This project focuses on reliable control logic and safe operation. Adapt timers and I/O assignments to your specific machine and safety requirements.
