# Direct AC–AC Buck Converter (Prelude to Voltage Restorer)

A high-performance, single-phase direct AC–AC buck converter designed as the foundational testbed for a Dynamic Voltage Restorer (DVR). This project implements advanced voltage-polarity commutation on an ESP32 microcontroller using hardware-enforced dead-time and a modular firmware architecture.

---

## 🚀 Key Engineering Features
* **Topology:** Soeiro's Standard L-Type AC-AC Buck Converter topology.
* **Commutation Strategy:** Hagemeyer Voltage-Polarity Commutation (utilizing input voltage sign to eliminate complex high-bandwidth current-direction sensors and suppress inductive spikes via natural bypass paths).
* **Control Platform:** ESP32 running a hybrid architecture (Arduino framework with FreeRTOS dual-core task separation and native ESP-IDF MCPWM hardware acceleration).
* **Switching Frequency:** $20\text{ kHz}$ with hardware-enforced $800\text{ ns}$ complementary dead-time.
* **Hardware Safety Interlock:** Series wired-AND $+15\text{ V}$ driver health fault-chain with immediate hardware shutdown capability.
* **Anti-Distortion ZCD:** Optocoupled phase sensing backed by an $8.5\text{ ms}$ software noise-lockout filter to immunize the converter against grid harmonics and vacuum cleaner/appliance noise spikes.

---

## 🛠️ Hardware Inventory & Pin Mapping
* **Power Switches:** $4 \times$ `FGW60N65WE` $650\text{ V}$, $60\text{ A}$ Trench Field-Stop IGBTs configured in common-emitter pairs ($S_1/S_3$ and $S_2/S_4$).
* **Power Source:** Yıldırım Elektronik 1500 VA single-phase variable transformer (bench testing initiated at the $55\text{ V}$ tap).
* **Gate Drivers:** Dual TLP350-based isolated driver boards driven by `M26LS31` differential line drivers ($5\text{ V}$, $100\text{ }\Omega$ loop resistors).
* **Filter Passives:** Alcon $4.0\text{ }\mu\text{F} / 1000\text{ VDC}$ film capacitors, Alcon KP-6 $0.1\text{ }\mu\text{F} / 1200\text{ VDC}$ pulse snubber capacitors, and a gapped output filter inductor ($L_o$).
* **Load:** $220\text{ V} / 100\text{ W}$ incandescent bulb (self-healing thermal test load).

---

## 📂 Project Structure
```text
ac_ac_buck_converter/
├── platformio.ini
├── include/
│   └── config.h              # Global constants, pin mappings, frequency settings
├── src/
│   ├── main.cpp              # FreeRTOS Core 0 heartbeat & Core 1 real-time task manager
│   ├── drivers/
│   │   ├── pwm_driver.h      # MCPWM hardware configuration & dead-time
│   │   └── pwm_driver.cpp
│   ├── sensing/
│   │   ├── zcd_sense.h       # Zero-crossing detection & 8.5ms noise-lockout state machine
│   │   ├── zcd_sense.cpp
│   │   ├── adc_sensor.h      # Trimpot setpoint & analog sensor handling
│   │   └── adc_sensor.cpp
│   ├── control/
│   │   ├── converter_fsm.h   # Hagemeyer half-cycle & commutation control FSM
│   │   └── converter_fsm.cpp
│   └── safety/
│       ├── safety_interlock.h # Wired-AND +15V fault monitoring & emergency shutdown
│       └── safety_interlock.cpp
└── README.md