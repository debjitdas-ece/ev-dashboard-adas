# Pin Assignment — STM32F103C8T6 Blue Pill

| Pin | Component | Direction | Function |
|---|---|---|---|
| PA0 | ADC1 CH0 | Input | Accelerator pedal (potentiometer) |
| PA1 | ADC1 CH1 | Input | Brake pedal (potentiometer) |
| PA2 | ADC1 CH2 | Input | Battery SOC (potentiometer) |
| PA3 | ADC1 CH3 | Input | Motor temperature (NTC potentiometer) |
| PA8 | TIM1 CH1 | PWM Output | Motor drive — 20 kHz |
| PA9 | USART1 TX | Output | Telemetry DMA stream |
| PA10 | USART1 RX | Input | UART command shell (IRQ) |
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
| PB0 | TIM3 CH1 | PWM Output | Buzzer tone |
