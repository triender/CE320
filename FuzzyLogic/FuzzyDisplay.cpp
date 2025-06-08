// FuzzyDisplay.cpp
#include "FuzzyDisplay.h" // Include the header file we just defined
#include <Adafruit_GFX.h>    // Adafruit_ST7735 requires Adafruit_GFX

// Constructor implementation
FuzzyDisplay::FuzzyDisplay(int8_t csPin, int8_t dcPin, int8_t rstPin) :
  tft(csPin, dcPin, rstPin), // Initialize the tft object
  prevTemp(-999.9),          // Initialize previous values to unlikely states to force first draw
  prevHumid(-999.9),
  prevSoil(-999.9),
  prevPump(-999.9) {
}

// begin method implementation
void FuzzyDisplay::begin(uint8_t rotation) {
  tft.initR(INITR_GREENTAB); // Initialize TFT with Green Tab configuration
  tft.setRotation(rotation); // Set the display rotation
  tft.fillScreen(ST77XX_BLACK); // Clear the screen to black
}

// drawLayout method implementation
void FuzzyDisplay::drawLayout() {
  tft.setTextSize(1); // Set default text size
  
  // Print the main title
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK); // White text on black background
  tft.setCursor(10, 5);
  tft.println("Fuzzy Irrigation System");
  
  // Draw a horizontal line separator
  tft.drawFastHLine(0, 20, tft.width(), ST77XX_WHITE); // Line across the screen width
  
  // Print sensor labels
  tft.setTextColor(ST77XX_CYAN, ST77XX_BLACK); // Cyan text for labels
  tft.setCursor(10, 30);
  tft.print("Temp:");
  tft.setCursor(65, 30);
  tft.print("Humid:");
  tft.setCursor(123, 30);
  tft.print("Soil:");

  // Print pump power label
  tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK); // Green text for pump label
  tft.setCursor(10, 60);
  tft.print("Pump Power Output:");
}

// updateValues method implementation
void FuzzyDisplay::updateValues(float temp, float humid, float soil, float pump) {
  bool pumpChanged = false; // Flag to track if pump value or bar needs redraw

  // Update Temperature if changed or if it's the first time (prevTemp is NAN or initial value)
  // A small threshold (0.05) is used to avoid flickering from minor fluctuations.
  if (isnan(temp) || isnan(prevTemp) || abs(temp - prevTemp) > 0.05) {
    tft.fillRect(10, 40, 45, 11, ST77XX_BLACK); // Clear previous temperature value area
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    tft.setCursor(10, 40);
    if (isnan(temp)) { // If temperature is Not a Number, display "---"
      tft.print("---");
    } else {
      tft.print(temp, 1); // Print temperature with 1 decimal place
      // Custom logic to position the degree symbol 'C' correctly based on number of digits
      int t = 36; // Base X position for degree symbol
      if (temp < 0) { // Adjust for negative sign
        t += 5; 
        if (abs(temp) < 10 && temp != 0) t += 5; // Adjust for single digit negative numbers
      } 
      else if (abs(temp) < 10 && temp != 0) { // Adjust for single digit positive numbers
        t -=5; 
      }
      else if (temp == 0) { // Adjust for zero
        t-=5;
      }

      // Draw a small degree symbol (°) using pixels
      tft.drawPixel(t, 40, ST77XX_WHITE);
      tft.drawPixel(t-1, 40, ST77XX_WHITE);
      tft.drawPixel(t, 41, ST77XX_WHITE);
      tft.drawPixel(t-1, 41, ST77XX_WHITE);
      tft.setCursor(t + 2, 40); // Position cursor for 'C'
      tft.print("C");
    }
    prevTemp = temp; // Store current temperature as previous for next comparison
  }

  // Update Humidity if changed or if it's the first time
  if (isnan(humid) || isnan(prevHumid) || abs(humid - prevHumid) > 0.05) {
    tft.fillRect(65, 40, 40, 11, ST77XX_BLACK); // Clear previous humidity value area
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    tft.setCursor(65, 40);
     if (isnan(humid)) { // If humidity is Not a Number, display "---"
      tft.print("---");
    } else {
      tft.print(humid, 1); // Print humidity with 1 decimal place
      int valEndX = tft.getCursorX(); // Get X position after printing the number
      tft.setCursor(valEndX + 2, 40); // Position cursor for '%' symbol
      tft.print("%");
    }
    prevHumid = humid; // Store current humidity as previous
  }

  // Update Soil Moisture if changed or if it's the first time
  if (isnan(soil) || isnan(prevSoil) || abs(soil - prevSoil) > 0.05) {
    tft.fillRect(123, 40, 40, 11, ST77XX_BLACK); // Clear previous soil moisture value area
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    tft.setCursor(123, 40);
    if (isnan(soil)) { // If soil moisture is Not a Number, display "---"
      tft.print("---");
    } else {
      tft.print(soil, 1); // Print soil moisture with 1 decimal place
      int valEndX = tft.getCursorX(); // Get X position after printing the number
      tft.setCursor(valEndX + 2, 40); // Position cursor for '%' symbol
      tft.print("%");
    }
    prevSoil = soil; // Store current soil moisture as previous
  }

  // Update Pump Power text if changed or if it's the first time. Using a larger threshold for pump.
  if (isnan(pump) || isnan(prevPump) || abs(pump - prevPump) > 0.5) { 
    pumpChanged = true; // Indicate that the pump value (and thus bar) needs updating
    tft.fillRect(55, 74, 70, 16, ST77XX_BLACK); // Clear previous pump power value area
    tft.setTextSize(2); // Use larger text for pump power
    if (isnan(pump)) { // If pump power is Not a Number, display "--" (due to larger text size)
        tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
        tft.setCursor(55,74);
        tft.print("--"); 
    } else {
        // Change text color based on pump power level
        if (pump < 20) {
        tft.setTextColor(ST77XX_BLUE, ST77XX_BLACK);
        } else if (pump < 50) {
        tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
        } else {
        tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
        }
        tft.setCursor(55, 74);
        // Print pump power. Show 0 decimal places if it's a whole number, 1 otherwise.
        // Constrain pump value to 0-100 range for display.
        tft.print(constrain(pump, 0, 100.0), (pump == (int)pump && pump >=0 && pump <=100) ? 0 : 1 ); 
    }
    prevPump = pump; // Store current pump power as previous
  }
  
  // Update Pump Power bar graph if pump value changed or if it's the initial draw
  if (pumpChanged || prevPump == -999.9) { // -999.9 is the initial prevPump value
    tft.fillRect(10, 100, 140, 15, ST77XX_BLACK); // Clear previous bar area
    
    int barWidth = 0;
    if(!isnan(pump)){ // Calculate bar width only if pump value is valid
        // Map constrained pump value (0-100) to bar width (0-140 pixels)
        barWidth = map(constrain(pump, 0, 100), 0, 100, 0, 140);
    }
    
    tft.fillRect(10, 100, barWidth, 15, ST77XX_GREEN); // Draw the new bar
    tft.drawRect(10, 100, 140, 15, ST77XX_WHITE);     // Draw a border around the bar area
  }
}