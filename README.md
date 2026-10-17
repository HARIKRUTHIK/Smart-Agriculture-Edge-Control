# Real-Time Edge-Controlled Smart Agriculture System

[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32-blue.svg)](https://www.espressif.com/)
[![Sensors: Moisture | Light | Nutrients](https://img.shields.io/badge/Sensors-Soil%20Moisture%20%7C%20LDR%20%7C%20EC-green.svg)]()
[![Actuation: Relay Matrix](https://img.shields.io/badge/Actuation-Pump%20%7C%20Fan%20%7C%20Nutrients-orange.svg)]()
[![Latency: Sub--50ms Decision](https://img.shields.io/badge/Latency-Sub--50ms%20Autonomous%20Loop-purple.svg)]()
[![Vision: MobileNetV2](https://img.shields.io/badge/Crop%20Vision-MobileNetV2%20(92.3%25%20acc)-61dafb.svg)]()

An autonomous edge-controlled precision agriculture system designed around the **ESP32**. The system couples a deterministic local closed-loop sensor-to-actuator control engine with a **MobileNetV2 edge computer vision classifier** for crop leaf disease diagnosis.

---

## 1. System Architecture

```
                    ┌────────────────────────────┐
                    │      Analog / I2C Buses    │
                    │ Moisture │ Light │ Nutrient│
                    └─────┬─────────┬────────┬───┘
                          │         │        │
                          ▼         ▼        ▼
                    ┌────────────────────────────┐
                    │      ESP32 Edge Node       │
                    │ Autonomous Decision Engine │
                    │   (<50 ms Decision Loop)   │
                    └──────┬────────┬────────┬───┘
                           │        │        │
                     Pump  │   Fan  │  Doser │ Relay Actuators
                           ▼        ▼        ▼
                    ┌────────────┬───────┬───────┐
                    │ Irrigation │Cooling│Nutri- │
                    │ Water Pump │  Fan  │  ent  │
                    └────────────┴───────┴───────┘
```

---

## 2. Deterministic Latency Budget

To maintain micro-climate stability inside precision greenhouses, actuation decisions are decoupled from cloud network availability. 

The complete control loop response time ($T_r$) is modeled as:
$$T_r = T_s + T_p + T_a$$
Where:
- $T_s$: Sensor ADC conversion and debouncing ($\approx 50\text{ ms}$)
- $T_p$: Local rule evaluation and edge threshold comparison ($\approx 30\text{ ms}$)
- $T_a$: Electromechanical relay coil energization and valve switching ($\approx 100\text{ ms}$)
- **Total Autonomous Response Time**: **$180\text{ ms}$** (guaranteed sub-200 ms closed-loop response).

### Autonomous Decision Matrix
1. **Irrigation Valve ($A_w$)**:
   $$A_w = \begin{cases} 1, & M < T_s \quad (\text{Moisture} < 35\%) \\ 0, & \text{otherwise} \end{cases}$$
2. **Cooling Ventilation Fan**: Activated when ambient solar light / heat index exceeds $250\text{ Lux}$.
3. **Nutrient Injection Dosing**: Solenoid opens when electrical conductivity indicates $<400\text{ ppm}$ nutrient depletion.

---

## 3. Edge Crop Leaf Disease Classifier (MobileNetV2)

In addition to environmental micro-climate regulation, the project features a companion edge vision model for early plant pathology diagnosis:
- **Architecture**: MobileNetV2 ($\alpha = 0.35$ compact width multiplier).
- **Dataset**: PlantVillage benchmark (tomato conditions including Early Blight, Late Blight, Leaf Mold, Bacterial Spot, and Healthy).
- **Performance**: Achieved **92.3% test accuracy** with **$<400\text{ ms}$** edge inference latency.

---

## 4. Hardware Wiring & Pin Mapping

| Peripheral | Protocol | ESP32 GPIO | Description |
| :--- | :---: | :---: | :--- |
| **Capacitive Soil Moisture** | Analog ADC | `GPIO 34` | Corrosion-resistant soil hydration sensor |
| **LDR Light Sensor** | Analog ADC | `GPIO 35` | Ambient solar irradiance |
| **Nutrient Sensor (EC)** | Analog ADC | `GPIO 32` | Fertilizer salt concentration |
| **Irrigation Pump Relay** | Digital Output | `GPIO 25` | 12V submersible water pump control |
| **Ventilation Fan Relay** | Digital Output | `GPIO 26` | High-flow greenhouse exhaust fan |
| **Nutrient Dosing Solenoid**| Digital Output | `GPIO 27` | Liquid nutrient dosing valve |

---

## 5. Repository File Structure

```text
Smart-Agriculture-Edge-Control/
├── README.md
├── .gitignore
│
├── firmware/
│   └── esp32_smart_agriculture.ino        # Autonomous edge closed-loop firmware
│
└── model/
    └── plant_disease_mobilenetv2.py       # MobileNetV2 plant leaf pathology architecture
```
