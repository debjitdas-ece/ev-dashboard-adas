# EV ADAS Dashboard — STM32F103C8T6 (Blue Pill)

A real-time electric vehicle dashboard with ADAS safety features, built on the STM32F103C8T6 "Blue Pill" and simulated end-to-end in PICSimLab. Sensor data flows from the MCU over UART into a Python (matplotlib) dashboard that mirrors what a real EV instrument cluster would show — speed, battery, motor health, and collision alerts, all updating live.

This was built during my Emertxe Embedded Systems Internship. The firmware runs a physics-based vehicle model (not just static sensor echoes), a full forward-collision/blind-spot ADAS engine off three ultrasonic sensors, a fault manager with bit-field fault tracking, and a UART command shell for injecting test values without touching a single potentiometer.

## What it does

- **EV Controller** — reads accelerator/brake pedal position from ADC, computes speed through a lumped inertia model (torque, drag, mass), tracks SOC via energy integration, estimates range, and scales torque across ECO / NORMAL / SPORT drive modes.
- **ADAS Engine** — three HC-SR04 ultrasonic sensors (front, left, right) feed a Time-To-Collision calculation, forward collision warning, blind-spot detection, and low-speed parking assist.
- **Fault Manager** — an 8-bit fault register (motor overheat, low SOC, collision-critical, sensor timeout, comm timeout) drives a 4-level alarm priority system and cuts motor PWM the instant a fault fires.
- **UART Telemetry** — two structured ASCII frames stream at 115200 baud, 10 Hz: an EV metrics line and an ADAS alert line.
- **UART Shell** — commands like `speed 80`, `soc 45`, `fault inject motor`, and `fault clear` let you test alarm thresholds and state transitions deterministically.
- **Python Dashboard** — a five-panel matplotlib UI: speedometer arc, SOC bar, ADAS bird's-eye view, rolling speed history, and an EV metrics panel.

## Hardware / pin map

| Pin | Peripheral | Signal |
|---|---|---|
| PA0 | ADC1 CH0 | Accelerator pedal (potentiometer) |
| PA1 | ADC1 CH1 | Brake pedal (potentiometer) |
| PA2 | ADC1 CH2 | Battery SOC (potentiometer) |
| PA3 | ADC1 CH3 | Motor temperature (potentiometer) |
| PA8 | TIM1 CH1 | Motor PWM, 20 kHz |
| PA9 / PA10 | USART1 TX/RX | Telemetry stream + command shell, 115200 bps |
| PC13 / PB1 | GPIO TRIG/ECHO | HC-SR04 Front |
| PB2 / PB3 | GPIO TRIG/ECHO | HC-SR04 Left |
| PB4 / PB5 | GPIO TRIG/ECHO | HC-SR04 Right |
| PB8–PB11 | GPIO | Collision, left BSD, right BSD, fault LEDs |
| PB0 | TIM3 CH1 | Buzzer tone (PWM) |

## Vehicle state machine

`PARKED → READY → DRIVING → REGEN → FAULT`, with entry/exit gated by accelerator and brake pedal thresholds, and any FAULT condition forcing an immediate PWM cut. Recovery from FAULT only happens through the `fault clear` shell command — no silent auto-recovery.

## Repo layout

```
ev-dashboard-adas/
├── firmware/
│   └── ev_dash/              STM32CubeIDE project (Core/, Drivers/, ev_dash.ioc, linker script)
├── dashboard/
│   └── dashboard.py          Python matplotlib dashboard (renamed from ev.py)
├── docs/
│   ├── EV_ADAS_requirement_and_design_document.pdf
│   └── EV_Control_Module.pdf
└── README.md
```

## Running it

**Firmware (PICSimLab simulation):**
1. Open `firmware/ev_dash/ev_dash.ioc` in STM32CubeIDE — CubeMX config regenerates the same pinout described above.
2. Build the project, then load the resulting `.elf`/`.bin` into PICSimLab configured as a Blue Pill (STM32F103C8T6) board.
3. Wire four virtual potentiometers to PA0–PA3 and three virtual ultrasonic sensors to the TRIG/ECHO pins listed above.

**Python dashboard:**
```bash
pip install pyserial matplotlib numpy
python dashboard/dashboard.py --port COM3      # real/simulated UART
python dashboard/dashboard.py --demo           # no hardware, synthetic data
```

## Telemetry format

Two lines, every 100 ms, 115200 baud:

```
SPD:72.5 SOC:79.3 TRQ:75 TMP:27.1 RNG:2600 ACC:50 BRK:0 MODE:1
F:40 L:400 R:400 TTC:2.1s COL:1 BSD:10 ALM:2 FLT:04
```

`FLT` is the fault byte in hex, `ALM` is alarm priority (0=none, 1=advisory, 2=warning, 3=critical), `COL` is collision level (0–2).

## Notes

Built and tested entirely in simulation (PICSimLab) — no physical hardware was used. The `docs/` folder has the full requirements and design document if you want the reasoning behind the thresholds, the alarm hysteresis logic, or the SOC/thermal model equations.
