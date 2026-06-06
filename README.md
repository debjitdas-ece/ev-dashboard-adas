# Real-Time EV Dashboard + ADAS Warning System

> **Emertxe Automotive Embedded Internship · Jayalaxmi Institute · Jun–Aug 2026**
> STM32F103C8T6 (Blue Pill) · STM32CubeIDE · PICSimLab · Embedded C

---

## Overview

A real-time Electric Vehicle instrument cluster combined with a multi-zone ADAS collision and blind-spot warning system, built entirely in Embedded C on the STM32F103C8T6 (Blue Pill) using STM32 HAL (CubeIDE). Hardware simulation runs in PICSimLab; a Python dashboard (matplotlib + pyserial) renders live gauges at 10 Hz over UART.

The system implements:
- Physics-based EV speed, SOC, torque, and range modelling at 10 Hz
- 3-zone ADAS using HC-SR04 ultrasonic sensors (front collision + left/right blind spot)
- Deterministic 5-state vehicle state machine with fault isolation
- Binary telemetry streaming over UART DMA at 115200 bps
- Priority-tiered alarm engine (P0–P3) with LED, buzzer, and dashboard indication
- UART command shell for live test injection and fault recovery

---

## Hardware Platform

| Component | Model / Detail | Qty |
|---|---|---|
| Microcontroller | STM32F103C8T6 (Blue Pill) — 72 MHz, 64 KB Flash, 20 KB SRAM | 1 |
| Simulator | PICSimLab (open-source ARM simulator) | — |
| IDE | STM32CubeIDE (HAL framework, CubeMX config) | — |
| Ultrasonic Sensors | HC-SR04 — 2–400 cm range, ±3 mm accuracy, 5V supply | 3 |
| Buzzer | Passive PWM buzzer — driven by TIM4, variable frequency tones | 1 |
| Status LEDs | Red / Yellow GPIO-driven — 220 Ω series resistor | 4 |
| Potentiometers | PICSimLab components — simulate accel, brake, SOC, motor temp | 4 |
| Programmer | ST-Link V2 (SWD) | 1 |

---

## Pin Assignment — STM32F103C8T6 Blue Pill

| Pin | Component | Direction | Function |
|---|---|---|---|
| PA0 | ADC1 CH0 | Input | Accelerator pedal position (0–100%) via potentiometer |
| PA1 | ADC1 CH1 | Input | Brake pedal pressure (0–100%) via potentiometer |
| PA2 | ADC1 CH2 | Input | Battery SOC simulation via potentiometer |
| PA3 | ADC1 CH3 | Input | Motor temperature (NTC) via potentiometer |
| PA8 | TIM1 CH1 | PWM Output | Motor drive signal — 20 kHz |
| PA9 | USART1 TX | Output | Telemetry packet stream (DMA) |
| PA10 | USART1 RX | Input | UART command shell (IRQ ring buffer) |
| PC13 | GPIO | Output | HC-SR04 Front — TRIG |
| PB1 | GPIO | Input | HC-SR04 Front — ECHO |
| PB2 | GPIO | Output | HC-SR04 Left — TRIG |
| PB3 | GPIO | Input | HC-SR04 Left — ECHO |
| PB4 | GPIO | Output | HC-SR04 Right — TRIG |
| PB5 | GPIO | Input | HC-SR04 Right — ECHO |
| PB8 | GPIO | Output | Red LED — Collision warning |
| PB9 | GPIO | Output | Yellow LED — Left blind spot |
| PB10 | GPIO | Output | Yellow LED — Right blind spot |
| PB11 | GPIO | Output | Red LED — Fault indicator |
| PB0 (TIM3) | TIM3 CH1–3 | PWM Output | Buzzer tone generation |

---

## System Architecture

```
ADC Inputs (PA0–PA3) ──► EV Controller ──► Speed / SOC / Torque / Power / Range
         100 Hz (TIM1 IRQ)                         │
                                                    ▼
TIM3 IRQ @ 100 ms ──► HC-SR04 ReadAll() ──► ADAS Engine ──► Alarm (P0–P3)
  3 sensors sequentially                   TTC + blind spot      │
                                                    │             ▼
                                            Fault Manager    LED + Buzzer
                                                    │
                                                    ▼
                                     UART TelemetryManager (DMA)
                                       Packets 0x01 + 0x02 @ 10 Hz
                                                    │
                                                    ▼
                                         Python Dashboard
                                     matplotlib + pyserial @ 10 Hz
```

---

## Vehicle State Machine

```
       Power-on
          │
          ▼
       PARKED ──── Accel > 2% ────► READY ──── Accel > 5% ────► DRIVING
          ▲                                                          │  │
          │                                                   Brake > 10%  FAULT
          │                                                          │      │
          └──── fault clear (UART) ◄──── FAULT ◄───────────────────┘      │
                                            ▲        Overheat / Low SOC /  │
                                            └────────────── Collision ◄────┘
                                         REGEN
                                   (Brake > 10% while DRIVING)
```

| State | Entry Condition | Key Actions |
|---|---|---|
| PARKED | Power-on / reset | All outputs off, UART heartbeat |
| READY | Accel pedal > 2% | Motor armed, sensors active |
| DRIVING | Accel pedal > 5% | Motor PWM, ADAS active, stream data |
| REGEN | Brake > 10% while DRIVING | Negative torque PWM, SOC charging |
| FAULT | Overheat / Low SOC / Collision CRITICAL | Cut PWM, open contactors, P1 alarm |

---

## ADAS Sensor Zones

| Sensor | TRIG | ECHO | Zone | Warning Threshold | Critical Threshold |
|---|---|---|---|---|---|
| SENSOR_FRONT | PC13 | PB1 | Forward (0–4 m) | < 50 cm or TTC < 3 s | < 20 cm or TTC < 1.5 s → FAULT |
| SENSOR_LEFT | PB2 | PB3 | Left blind spot | < 30 cm (speed > 20 km/h) | — ADVISORY |
| SENSOR_RIGHT | PB4 | PB5 | Right blind spot | < 30 cm (speed > 20 km/h) | — ADVISORY |

**Sensor polling:** All 3 HC-SR04s triggered sequentially every 100 ms via TIM3 IRQ. Worst-case ReadAll() time: 3 × 30 ms = 90 ms < 100 ms budget.

**Alarm hysteresis:** De-assertion requires 3 consecutive clear readings (300 ms) to prevent rapid toggling on noisy data.

---

## Alarm Priority Levels

| Level | Condition | Trigger | LED | Buzzer |
|---|---|---|---|---|
| P1 CRITICAL | Front < 20 cm or TTC < 1.5 s | Immediate FAULT transition + PWM cut | Red rapid flash | Rapid beep + FAULT banner |
| P2 WARNING | Front < 50 cm or TTC < 3 s | ADAS WARNING state | Red steady | Double beep + amber highlight |
| P3 ADVISORY | Blind spot < 30 cm or speed > 120 km/h | ADAS advisory | Yellow steady | Single beep + yellow indicator |
| P0 NONE | All clear | Normal | Off | Silent |

---

## Firmware Modules

| Module | Files | Responsibility |
|---|---|---|
| EV Controller | `ev_control.c/.h` | Speed model (physics inertia), SOC, torque, power, range, ECO/NORMAL/SPORT drive modes |
| ADAS Engine | `adas.c/.h` | TTC calculation, collision/blind-spot/parking logic, alarm priority |
| Ultrasonic Driver | `ultrasonic.c/.h` | HC-SR04 trigger-echo GPIO polling, distance conversion, clamping to [2, 400] cm |
| Fault Manager | `fault.c/.h` | Fault flag evaluation (FAULT_OT, FAULT_SOC, FAULT_COL, FAULT_SEN), state transitions, contactor control |
| UART Shell | `uart_shell.c/.h` | Ring buffer RX (128 bytes, IRQ), command parser, binary packet TX via DMA |
| Buzzer Driver | `buzzer.c/.h` | TIM4 PWM tone generation, alarm priority mapping |
| Main Scheduler | `main.c` | ISR flag polling, watchdog (IWDG) refresh, module init, 10 ms main loop |

**Main loop budget:** < 5 ms execution time. IWDG watchdog reset if loop stalls > 1 s.

---

## UART Telemetry Protocol — 115200 bps, 8N1

| Packet | Type | Rate | Payload |
|---|---|---|---|
| `0x01` EV Metrics | Binary frame | 10 Hz | speed (km/h), SOC (%), torque (Nm), power (kW), range (km), motor_temp (°C), drive_mode, uptime_ms |
| `0x02` ADAS Alerts | Binary frame | 10 Hz | front_cm, left_cm, right_cm, collision_lvl, blindspot_l, blindspot_r, ttc_sec |
| `0x03` Vehicle State | Binary frame | On change | state, drive_mode, fault_flags |
| `0x04` ACK / Status | Binary frame | On request | uptime_ms + version string |

---

## UART Shell Commands

```
mode <eco/normal/sport>     — Switch drive mode; resets torque scale
speed set <kmh>             — Inject speed into EV model (testing)
soc set <pct>               — Override SOC value (testing)
obstacle <cm>               — Inject front distance (test alarm)
blindspot <on/off>          — Simulate side vehicle present
fault inject <motor/soc>    — Trigger motor overheat or SOC fault
fault clear                 — Clear faults, return to PARKED
status                      — Print full system state over UART
stream <on/off>             — Enable/disable telemetry packet stream
reset                       — Software reset via IWDG
```

---

## Python Dashboard

A matplotlib + pyserial GUI renders a dark instrument cluster refreshed at 10 Hz.

| Panel | Content |
|---|---|
| Speedometer (top-left) | Arc gauge, speed in km/h, colour-coded green → amber → red |
| Battery SOC (top-right) | Horizontal bar, % label, estimated range km |
| ADAS Bird-Eye (bottom-left) | Top-down view: ego vehicle, obstacle arcs at measured distances, TTC label, blind-spot indicators |
| Metrics Panel (bottom-right) | Power kW, motor temp °C, torque Nm, drive mode badge |

**Colour coding:** `#00ff88` Normal · `#ffaa00` Warning · `#ff3333` Critical / Fault · `#4488ff` Ego vehicle

---

## Fault Conditions

| Fault | Trigger | Action |
|---|---|---|
| FAULT_OT (overheat) | Motor temp > 90 °C | Cut PWM, enter FAULT state |
| FAULT_SOC (low battery) | SOC < 2% | Cut PWM, enter FAULT state |
| FAULT_COL (collision) | Front < 20 cm or TTC < 1.5 s | Cut PWM, enter FAULT state |
| FAULT_SEN (sensor timeout) | Any sensor miss | Degrade gracefully, log warning |
| FAULT_COM (UART timeout) | No RX > 5 s | Warning log, retain last known values |
| Watchdog timeout | Main loop stall > 1 s | IWDG hardware reset |

Recovery: `fault clear` via UART shell → transitions FAULT → PARKED.

---

## Key Performance Targets

| Metric | Target |
|---|---|
| Main loop execution time | < 5 ms |
| HC-SR04 ReadAll() worst case | < 100 ms (3 × 30 ms) |
| UART packet latency | < 10 ms |
| Fault state entry latency | ≤ 1 loop cycle (≤ 10 ms) |
| P1 CRITICAL alarm from threshold breach | < 100 ms |
| Dashboard gauge refresh after sensor change | < 150 ms |
| ADC resolution | 12-bit (4096 steps) |
| UART baud rate | 115200 bps |
| Motor PWM frequency | ~20 kHz |

---

## Repo Structure

```
ev-dashboard-adas/
├── src/
│   ├── main.c              ← Main scheduler, ISR flags, watchdog
│   ├── ev_control.c        ← Speed model, SOC, torque, drive modes
│   ├── adas.c              ← TTC, collision/blind-spot/parking logic
│   ├── ultrasonic.c        ← HC-SR04 GPIO polling, distance conversion
│   ├── fault.c             ← Fault flags, state transitions
│   ├── uart_shell.c        ← Ring buffer RX, command parser, DMA TX
│   └── buzzer.c            ← TIM4 PWM tone generation
├── include/
│   ├── ev_control.h
│   ├── adas.h
│   ├── ultrasonic.h
│   ├── fault.h
│   ├── uart_shell.h
│   └── buzzer.h
├── docs/
│   ├── EV_ADAS_Requirements_Design_v1.0.pdf
│   └── pin_assignment.md
├── tests/
│   └── fault_injection_test.md
└── README.md
```

---

## Tools & Environment

| Tool | Purpose |
|---|---|
| STM32CubeIDE | Firmware development, HAL config (CubeMX), SWD flashing |
| PICSimLab | ARM simulation — potentiometers, signal generator, oscilloscope, UART terminal |
| ST-Link V2 | SWD programming and debug interface |
| Python 3 + matplotlib + pyserial | Live telemetry dashboard |

---

## About

Built during the Emertxe Automotive Embedded Internship program (Jayalaxmi Institute, Jun–Aug 2026) as a production-style embedded C project covering real-time scheduling, HAL driver development, UART DMA, state machine design, and sensor fusion — targeting automotive embedded and ADAS engineering roles.

**Debjit Das** · B.Tech ECE @ JGEC
📧 0001debjitdas@gmail.com
🔗 [LinkedIn](https://linkedin.com/in/debjit-das-00892b3a8) · [GitHub](https://github.com/debjitdas-ece)
