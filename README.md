# CE320 - Fuzzy Logic Irrigation System

## Project Overview

This project implements an automated irrigation system for plants using fuzzy logic. It monitors environmental conditions—temperature, humidity, and soil moisture—and intelligently decides the appropriate amount of water to supply via a pump. The system provides real-time feedback on sensor readings and pump status through an ST7735 TFT display.

## Features

*   **Sensor Monitoring**: Continuously reads data from:
    *   DHT22 sensor (Temperature and Humidity)
    *   Analog Soil Moisture sensor
*   **Fuzzy Logic Control**: Employs a fuzzy logic engine with predefined rules to determine the optimal pump power based on sensor inputs.
*   **TFT Display**: Shows current temperature, humidity, soil moisture levels, and the calculated pump power on an Adafruit ST7735 screen.
*   **Non-Blocking Operation**: Uses `millis()` for timing to ensure responsive sensor reading and display updates without halting the main program flow.
*   **Optimized Display Updates**: The display only redraws values that have changed, reducing flicker and improving performance.

## Hardware Requirements

*   ESP32 Development Board (or similar Arduino-compatible board with sufficient pins and ADC resolution)
*   DHT22 Temperature and Humidity Sensor
*   Analog Soil Moisture Sensor
*   Adafruit ST7735 TFT Display (1.8" or similar)
*   Water Pump (and appropriate driver/relay if needed, controlled by a digital pin based on fuzzy output)
*   Breadboard and Jumper Wires

## Software & Libraries

*   **Arduino IDE** or **PlatformIO**
*   **Required Libraries**:
    *   `DHT sensor library` (by Adafruit)
    *   `Fuzzy.h` (A fuzzy logic library compatible with the classes used, e.g., "Fuzzy" by Arduino or other)
    *   `Adafruit ST7735 and ST7789 Library` (by Adafruit)
    *   `Adafruit GFX Library` (by Adafruit - dependency for ST7735)
    *   `SPI.h` (Standard Arduino library)

    *(Refer to `libraries_required.txt` for more details on installation.)*

## Setup and Installation

1.  **Connect Hardware**:
    *   DHT22 Data Pin to GPIO 13 (configurable via `DHTPIN`)
    *   Soil Moisture Sensor Analog Out to GPIO 27 (configurable via `SOIL_MOISTURE_PIN`)
    *   TFT Display:
        *   CS to GPIO 5 (`TFT_CS`)
        *   RST to GPIO 4 (`TFT_RST`)
        *   DC to GPIO 22 (`TFT_DC`)
        *   SDA/MOSI to ESP32's MOSI pin (usually GPIO 23)
        *   SCK/SCLK to ESP32's SCLK pin (usually GPIO 18)
        *   LED/VCC/GND as per display module requirements.
    *   Connect the water pump control mechanism to a suitable output pin (this part is not explicitly detailed in the provided code but is the ultimate output of the system).
2.  **Install Libraries**: Open the Arduino IDE, go to `Sketch > Include Library > Manage Libraries...` and install the libraries listed above. For PlatformIO, add them to your `platformio.ini`.
3.  **Configure Pins**: Verify pin definitions at the top of `FuzzyLogic.ino` match your wiring.
4.  **Upload Code**: Select your board and port, then upload `FuzzyLogic.ino` to your microcontroller.
5.  **Serial Monitor**: Open the Serial Monitor at 115200 baud to view debug messages and sensor readings.

## How It Works

1.  **Initialization (`setup()`):**
    *   Serial communication, DHT sensor, and TFT display are initialized.
    *   Fuzzy logic inputs, outputs, and sets are defined.
    *   Fuzzy rules are established in `setupFuzzyRules()`.
    *   The static layout of the TFT display is drawn.
2.  **Main Loop (`loop()`):**
    *   Periodically reads temperature and humidity from the DHT22 sensor.
    *   Periodically reads the analog value from the soil moisture sensor and converts it to a percentage.
    *   If all sensor readings are valid:
        *   The current sensor values are fed into the fuzzy logic system (`fuzzy->setInput()`).
        *   The inputs are fuzzified (`fuzzy->fuzzify()`).
        *   The fuzzy rules are evaluated, and the result is defuzzified to get a crisp pump power value (`fuzzy->defuzzify()`).
    *   The latest sensor readings and the calculated pump power are updated on the TFT display.
    *   Debug information is printed to the Serial Monitor.

## Customization

*   **Fuzzy Sets and Rules**: Modify the `FuzzySet` definitions and the rules in `setupFuzzyRules()` in `FuzzyLogic.ino` to fine-tune the irrigation behavior for different plants or environments.
*   **Sensor Pins**: Change the `#define` statements for sensor and TFT pins if your wiring differs.
*   **Display Layout**: Adjust the `drawLayout()` and `updateValues()` methods in `FuzzyDisplay.cpp` to change the appearance of the TFT display.
*   **Timing Intervals**: Modify `dhtReadInterval`, `soilReadInterval`, and `logicDisplayInterval` to change how frequently tasks are performed.

---
