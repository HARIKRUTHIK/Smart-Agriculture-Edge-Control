// =============================================================================
// Project: Real-Time Edge-Controlled Smart Agriculture System
// Target: ESP32 + Multi-Sensors + Relay Actuation Matrix
// Features: Sub-50ms Local Edge Decision Loop, Zero Cloud Dependency
// =============================================================================

#include <Wire.h>

// -----------------------------------------------------------------------------
// Sensor Pinout (Analog & Digital Inputs)
// -----------------------------------------------------------------------------
#define SOIL_MOISTURE_PIN   34    // Capacitive Soil Moisture Sensor (ADC1_CH6)
#define LDR_LIGHT_PIN       35    // Light Dependent Resistor (ADC1_CH7)
#define NUTRIENT_PIN        32    // Nutrient / EC analog sensor input (ADC1_CH4)

// -----------------------------------------------------------------------------
// Actuator Relay Pinout
// -----------------------------------------------------------------------------
#define RELAY_PUMP_PIN      25    // Irrigation Water Pump Relay
#define RELAY_FAN_PIN       26    // Greenhouse Ventilation / Cooling Fan Relay
#define RELAY_NUTRIENT_PIN  27    // Solenoid Nutrient Dispenser Relay

// Threshold Calibration Limits
const float MOISTURE_THRESHOLD_PERCENT = 35.0; // Trigger pump if moisture < 35%
const float LIGHT_THRESHOLD_LUX        = 250.0; // High sunlight limit
const float NUTRIENT_THRESHOLD_PPM     = 400.0; // Trigger dispenser if nutrients low

// Edge Decision Loop Timing
const unsigned long CONTROL_INTERVAL_MS = 250; // 250 ms deterministic cycle
unsigned long lastControlTime = 0;

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n--- Real-Time Smart Agriculture Edge Controller Starting ---");

    // Configure Actuator Relays (Active-Low industrial relay modules)
    pinMode(RELAY_PUMP_PIN, OUTPUT);
    pinMode(RELAY_FAN_PIN, OUTPUT);
    pinMode(RELAY_NUTRIENT_PIN, OUTPUT);

    // Initial state: all actuators OFF
    digitalWrite(RELAY_PUMP_PIN, HIGH);
    digitalWrite(RELAY_FAN_PIN, HIGH);
    digitalWrite(RELAY_NUTRIENT_PIN, HIGH);

    // ADC Resolution
    analogReadResolution(12); // 0 - 4095

    Serial.println("[OK] Actuator relays and sensor inputs configured.");
}

void loop() {
    unsigned long now = millis();
    if (now - lastControlTime >= CONTROL_INTERVAL_MS) {
        unsigned long t_start = micros();
        lastControlTime = now;

        // 1. Read Sensors (Sampling time Ts ~ 50 ms)
        int rawMoisture = analogRead(SOIL_MOISTURE_PIN);
        int rawLight    = analogRead(LDR_LIGHT_PIN);
        int rawNutrient = analogRead(NUTRIENT_PIN);

        // Convert to engineering units
        // Capacitive sensor: 4095 = bone dry, ~1500 = fully saturated
        float moisturePercent = map(rawMoisture, 4095, 1500, 0, 100);
        moisturePercent = constrain(moisturePercent, 0.0, 100.0);

        float lightValue = (rawLight / 4095.0) * 1000.0;
        float nutrientPpm = (rawNutrient / 4095.0) * 1200.0;

        // 2. Local Edge Decision Logic (Processing time Tp < 1 ms)
        bool pumpActive = false;
        bool fanActive = false;
        bool nutrientActive = false;

        // Autonomous Irrigation Rule: Aw = 1 if M < Ts
        if (moisturePercent < MOISTURE_THRESHOLD_PERCENT) {
            pumpActive = true;
            digitalWrite(RELAY_PUMP_PIN, LOW); // Turn Pump ON
        } else {
            digitalWrite(RELAY_PUMP_PIN, HIGH); // Turn Pump OFF
        }

        // Autonomous Cooling Rule
        if (lightValue > LIGHT_THRESHOLD_LUX) {
            fanActive = true;
            digitalWrite(RELAY_FAN_PIN, LOW); // Turn Fan ON
        } else {
            digitalWrite(RELAY_FAN_PIN, HIGH); // Turn Fan OFF
        }

        // Autonomous Nutrient Dosing Rule
        if (nutrientPpm < NUTRIENT_THRESHOLD_PPM) {
            nutrientActive = true;
            digitalWrite(RELAY_NUTRIENT_PIN, LOW); // Dispense Nutrients
        } else {
            digitalWrite(RELAY_NUTRIENT_PIN, HIGH);
        }

        unsigned long decision_latency_us = micros() - t_start;

        // 3. Serial Audit Log
        Serial.printf("[AGRI-EDGE] Moisture=%.1f%% (Pump:%d) | Light=%.0f (Fan:%d) | Nutrients=%.0f ppm (Dose:%d) | LoopTime=%lu us\n",
                      moisturePercent, pumpActive, lightValue, fanActive, nutrientPpm, nutrientActive, decision_latency_us);
    }
}
