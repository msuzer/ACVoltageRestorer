#include <Arduino.h>
#include "config.h"
#include "drivers/pwm_driver.h"
#include "sensing/zcd_sense.h"
#include "control/converter_fsm.h"
#include "safety/safety_interlock.h"

// FreeRTOS Task Handle for Core 1 Real-Time Loop
TaskHandle_t RealTimeTaskHandle = NULL;

// --- Core 0 Heartbeat Task (1 Hz Blink) ---
void heartbeatTask(void *pvParameters) {
    pinMode(PIN_USER_LED, OUTPUT);
    while (true) {
        digitalWrite(PIN_USER_LED, HIGH);
        vTaskDelay(pdMS_TO_TICKS(500)); // ON for 500ms
        digitalWrite(PIN_USER_LED, LOW);
        vTaskDelay(pdMS_TO_TICKS(500)); // OFF for 500ms
    }
}

// --- Core 1 Real-Time Task (1 kHz Loop) ---
void realTimeCore1Task(void *pvParameters) {
    const TickType_t xFrequency = pdMS_TO_TICKS(1); // 1ms tick interval
    TickType_t xLastWakeTime = xTaskGetTickCount(); 
    
    // Track previous health state for edge detection (starts assuming healthy)
    static bool lastHealthState = true; 

    while (true) {
        // Evaluate current system health from the wired-AND interlock chain
        bool currentHealth = checkSystemHealth();

        // Detect state change (Edge Trigger)
        if (currentHealth != lastHealthState) {
            lastHealthState = currentHealth;
            
            if (!currentHealth) {
                Serial.println("[WARNING] System Fault Detected! +15V Interlock Chain Broken. PWM Disabled.");
            } else {
                Serial.println("[INFO] System Health Restored. +15V Interlock Chain OK.");
            }
        }

        // If healthy, run converter state machine; otherwise, safety_interlock handles PWM shutdown
        if (currentHealth) {
            updateConverterFSM();
        }
        
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Initializing Direct AC-AC Buck Converter...");

    // Initialize individual modules
    initSafetyInterlock();
    initZCD();
    initPWMDriver();
    initConverterFSM();

    // 1. Create Heartbeat Task on Core 0 (Low Priority)
    xTaskCreatePinnedToCore(
        heartbeatTask,       /* Task function */
        "HeartbeatTask",     /* Name */
        2048,                /* Stack size */
        NULL,                /* Parameters */
        1,                   /* Priority (Low) */
        NULL,                /* Task handle */
        0                    /* Core 0 */
    );

    // Create High-Priority Real-Time Task pinned strictly to Core 1
    xTaskCreatePinnedToCore(
        realTimeCore1Task,   /* Task function */
        "RealTimeTask",      /* Name */
        4096,                /* Stack size */
        NULL,                /* Parameters */
        3,                   /* Priority (High) */
        &RealTimeTaskHandle, /* Task handle */
        1                    /* Core 1 */
    );

    Serial.println("System Initialization Complete. Starting Core 1 FSM.");
}

void loop() {
    // Core 0 handles low-priority telemetry, serial printing, monitoring
    vTaskDelay(pdMS_TO_TICKS(500));
    // Example telemetry print:
    // Serial.printf("System Healthy. Free Heap: %d\n", ESP.getFreeHeap());
}