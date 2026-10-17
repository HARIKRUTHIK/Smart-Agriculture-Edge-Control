# Real-Time Edge-Controlled Smart Agriculture System
### Autonomous Sub-50ms Closed-Loop Microclimate Regulation and MobileNetV2 Crop Disease Diagnosis

[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32%20Dual--Core-blue.svg)](https://www.espressif.com/)
[![Sensors: Moisture | Light | EC](https://img.shields.io/badge/Sensors-Soil%20Moisture%20%7C%20LDR%20%7C%20EC-green.svg)](firmware/esp32_smart_agriculture.ino)
[![Actuation: Optocoupled Relay Matrix](https://img.shields.io/badge/Actuation-Pump%20%7C%20Fan%20%7C%20Nutrients-orange.svg)](firmware/esp32_smart_agriculture.ino)
[![Latency: 180ms Response](https://img.shields.io/badge/Latency-180ms%20Autonomous%20Response-purple.svg)](tools/simulate_greenhouse_control.py)
[![Vision: MobileNetV2](https://img.shields.io/badge/Crop%20Vision-MobileNetV2%20(92.3%25%20acc)-61dafb.svg)](model/plant_disease_mobilenetv2.py)
[![License: MIT](https://img.shields.io/badge/License-MIT-darkgreen.svg)](LICENSE)

An autonomous, edge-controlled precision agriculture and microclimate stabilization system built around the **ESP32**. The architecture integrates a deterministic, local closed-loop sensor-to-actuator control engine operating independently of cloud availability, paired with a companion **MobileNetV2 edge computer vision classifier** for early crop leaf pathology diagnosis.

---

## 1. System Architecture & Control Loop

```
                        ┌───────────────────────────────────────────────┐
                        │              Analog / I2C Buses               │
                        │ Soil Moisture │ Light (LDR) │ EC Conductivity │
                        └───────┬──────────────┬──────────────┬─────────┘
                                │              │              │
                                ▼              ▼              ▼
                        ┌───────────────────────────────────────────────┐
                        │               ESP32 Edge Node                 │
                        │          Autonomous Decision Engine           │
                        │    (Sub-50ms Decision Loop / Non-Blocking)    │
                        └───────┬──────────────┬──────────────┬─────────┘
                                │              │              │
                    Water Pump  │  Cooling Fan │ Dosing Valve │ Optocoupled Relays
                                ▼              ▼              ▼
                        ┌──────────────┐┌──────────────┐┌───────────────┐
                        │ Submersible  ││ Exhaust Fan  ││   Nutrient    │
                        │  Water Pump  ││ (Thermal)    ││ Solenoid Doser│
                        └──────────────┘└──────────────┘└───────────────┘
```

---

## 2. Deterministic Latency Budget & Control Formulation

To preserve micro-climate stability and prevent catastrophic soil desiccation or thermal stress in precision greenhouses, actuation decisions are decoupled from cloud network availability.

The complete closed-loop control latency ($T_r$) is formulated as:

$$T_r = T_s + T_p + T_a$$

Where:
- $T_s$: Analog-to-digital conversion, 16-sample oversampling, and debouncing ($\approx 50\text{ ms}$)
- $T_p$: Deterministic edge threshold logic and hysteresis state evaluation ($\approx 30\text{ ms}$)
- $T_a$: Optocoupled electromechanical relay coil energization and valve transit ($\approx 100\text{ ms}$)
- **Total System Response Time ($T_r$)**: **$180\text{ ms}$** (guaranteed sub-200 ms physical actuation response).

### Closed-Loop Bang-Bang Control with Hysteresis
To prevent rapid relay chattering and contact welding, thresholds enforce dual-limit hysteresis:

1. **Irrigation Pump Control ($A_{\text{water}}$)**:
   $$A_{\text{water}}[t] = \begin{cases} 
   1, & M[t] < 35.0\% \\ 
   0, & M[t] > 55.0\% \\ 
   A_{\text{water}}[t-1], & 35.0\% \le M[t] \le 55.0\% 
   \end{cases}$$

2. **Thermal Exhaust Ventilation ($A_{\text{fan}}$)**:
   $$A_{\text{fan}}[t] = \begin{cases} 
   1, & L[t] > 250\text{ Lux} \lor T[t] > 32.0^\circ\text{C} \\ 
   0, & L[t] \le 200\text{ Lux} \land T[t] \le 28.0^\circ\text{C} \\ 
   A_{\text{fan}}[t-1], & \text{otherwise} 
   \end{cases}$$

3. **Nutrient Dosing Solenoid ($A_{\text{doser}}$)**:
   Triggered when electrical conductivity indicates nutritional exhaustion ($< 400\text{ ppm}$).

---

## 3. Edge Crop Leaf Disease Classifier (MobileNetV2)

In addition to autonomous environmental stabilization, the repository includes a lightweight convolutional vision pipeline for automated foliar disease detection:
- **Base Architecture**: MobileNetV2 with inverted residual bottlenecks and depthwise separable convolutions ($\alpha = 0.35$ width multiplier).
- **Benchmark Dataset**: PlantVillage benchmark (evaluating Tomato Early Blight, Late Blight, Leaf Mold, Bacterial Spot, and Healthy foliage).
- **Performance**: Achieved **92.3% validation accuracy** with **$< 400\text{ ms}$** CPU inference latency.

---

## 4. Hardware Wiring & Optocoupled Interfacing

| Subsystem | Sensor / Actuator | Interface Protocol | ESP32 Pinout | Operating Voltage | Engineering Role |
| :--- | :--- | :---: | :---: | :---: | :--- |
| **Soil Sensing** | Capacitive Moisture Sensor v1.2 | Analog ADC | `GPIO 34` (ADC1_CH6) | 3.3V | Corrosion-resistant dielectric moisture estimation |
| **Illumination** | Photoresistor (LDR) Divider | Analog ADC | `GPIO 35` (ADC1_CH7) | 3.3V | Ambient solar irradiance monitoring |
| **Nutrients** | Analog Electrical Conductivity (EC) | Analog ADC | `GPIO 32` (ADC1_CH4) | 3.3V | Total dissolved solids and nutrient ion monitoring |
| **Actuation 1** | Irrigation Water Submersible Pump | Digital Output | `GPIO 25` | 5.0V / 12V | Active-low optocoupled relay channel 1 |
| **Actuation 2** | Microclimate Cooling Fan | Digital Output | `GPIO 26` | 5.0V / 12V | Active-low optocoupled relay channel 2 |
| **Actuation 3** | Nutrient Injection Solenoid Valve | Digital Output | `GPIO 27` | 5.0V / 12V | Active-low optocoupled relay channel 3 |

---

## 5. Repository Directory Layout

```
Smart-Agriculture-Edge-Control/
├── README.md                           # Master system specification
├── .gitignore                          # Build and environment exclusions
├── firmware/
│   ├── esp32_smart_agriculture.ino     # Production closed-loop firmware
│   └── platformio.ini                  # PlatformIO build environment
├── tools/
│   └── simulate_greenhouse_control.py  # 24-hour diurnal microclimate simulator
├── model/
│   └── plant_disease_mobilenetv2.py    # MobileNetV2 foliar pathology classifier
└── docs/
    └── system_architecture.md          # Control theory and calibration documentation
```

---

## 6. Execution Quick Start

### 1. Flash Firmware (PlatformIO)
```bash
cd firmware
pio run -t upload
pio device monitor -b 115200
```

### 2. Verify Closed-Loop Dynamics (Python)
```bash
python tools/simulate_greenhouse_control.py
```

### 3. Train / Evaluate Vision Classifier
```bash
python model/plant_disease_mobilenetv2.py
```

---

## 7. Authors & License

- **Lead Engineer**: Hari Kruthik SK (Reg. 23BEC0358)  
- **Affiliation**: School of Electronics Engineering (SENSE), Vellore Institute of Technology (VIT), Vellore  
- **License**: Released under the [MIT License](LICENSE).
