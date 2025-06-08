// FuzzyDisplay.h
#ifndef FuzzyDisplay_h // Include guard to prevent multiple inclusions
#define FuzzyDisplay_h

#include <Adafruit_ST7735.h> // Include the Adafruit ST7735 library for TFT display control

// Defines a class to manage the TFT display for the fuzzy irrigation system.
class FuzzyDisplay {
  public:
    // Constructor: Initializes the FuzzyDisplay object with the necessary pins for the TFT display.
    // csPin: Chip Select pin for the TFT.
    // dcPin: Data/Command pin for the TFT.
    // rstPin: Reset pin for the TFT.
    FuzzyDisplay(int8_t csPin, int8_t dcPin, int8_t rstPin);

    // Initializes the TFT display. Call this in the Arduino setup() function.
    // rotation: Sets the screen rotation (0-3). Default is 3.
    void begin(uint8_t rotation = 3); 

    // Draws the static layout of the user interface on the TFT screen.
    // This includes titles, labels, and lines that don't change.
    void drawLayout();

    // Updates the dynamic values (sensor readings and pump power) on the TFT screen.
    // temp: Current temperature value.
    // humid: Current humidity value.
    // soil: Current soil moisture value.
    // pump: Current calculated pump power value.
    void updateValues(float temp, float humid, float soil, float pump);

  private:
    Adafruit_ST7735 tft; // An instance of the Adafruit_ST7735 class to interact with the display.

    // Member variables to store the previous sensor and pump values.
    // These are used to optimize display updates by only redrawing values that have changed.
    float prevTemp;    // Stores the previously displayed temperature.
    float prevHumid;   // Stores the previously displayed humidity.
    float prevSoil;    // Stores the previously displayed soil moisture.
    float prevPump;    // Stores the previously displayed pump power.
};

#endif // End of include guard