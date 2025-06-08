/*
 * Project: Fuzzy Logic Irrigation System
 * 
 * Description:
 * This Arduino project implements an automated irrigation system based on fuzzy logic.
 * It monitors environmental conditions (temperature, humidity, and soil moisture)
 * using a DHT22 sensor and an analog soil moisture sensor.
 * 
 * The system uses a set of predefined fuzzy rules to determine the appropriate
 * watering level (pump power). Sensor readings are fuzzified, processed through
 * the rule base, and then defuzzified to get a crisp output value for pump control.
 * 
 * Current sensor data and the calculated pump power are displayed on an
 * Adafruit ST7735 TFT screen, managed by the FuzzyDisplay class.
 * 
 * The main loop operates non-blockingly, periodically reading sensors,
 * executing the fuzzy logic, and updating the display.
 * 
 * Components:
 * - DHT22 Sensor (Temperature & Humidity)
 * - Analog Soil Moisture Sensor
 * - Adafruit ST7735 TFT Display
 * - Water Pump (controlled by the output of the fuzzy logic)
 * 
 * Libraries:
 * - DHT.h (for DHT sensor)
 * - Fuzzy.h (for fuzzy logic operations - specific library assumed)
 * - Adafruit_ST7735.h (for TFT display)
 * - SPI.h (dependency for Adafruit_ST7735)
 * 
 * Pins:
 * - DHTPIN: 13 (DHT22 data pin)
 * - SOIL_MOISTURE_PIN: 27 (Analog input for soil moisture)
 * - TFT_CS: 5 (TFT Chip Select)
 * - TFT_RST: 4 (TFT Reset)
 * - TFT_DC: 22 (TFT Data/Command)
 * 
 * Author: CE320 - Fuzzy Logic Team
 * Date: May 29, 2025 
 */

#include <DHT.h>
#include <Fuzzy.h>

#include "FuzzyDisplay.h" 

// --- Sensor and General Defines ---
#define DHTPIN 13
#define DHTTYPE DHT22
#define SOIL_MOISTURE_PIN 27

// --- TFT Pin Defines ---
#define TFT_CS    5
#define TFT_RST   4  
#define TFT_DC    22

// --- Object Instantiations ---
DHT dht(DHTPIN, DHTTYPE);
Fuzzy* fuzzy = new Fuzzy();
FuzzyDisplay myDisplay(TFT_CS, TFT_DC, TFT_RST);

// --- Fuzzy Logic Definitions ---
FuzzySet* lowTemp = new FuzzySet(-5, -5, 10, 20);     
FuzzySet* mediumTemp = new FuzzySet(10, 20, 20, 30);   
FuzzySet* highTemp = new FuzzySet(20, 30, 45, 45);     

FuzzySet* lowHumidity = new FuzzySet(0, 0, 30, 50);     
FuzzySet* mediumHumidity = new FuzzySet(30, 50, 50, 70); 
FuzzySet* highHumidity = new FuzzySet(50, 70, 100, 100); 

FuzzySet* drySoil = new FuzzySet(0, 0, 20, 35);     
FuzzySet* moistSoil = new FuzzySet(20, 35, 40, 55);  
FuzzySet* wetSoil = new FuzzySet(40, 55, 100, 100);  

FuzzySet* noWater = new FuzzySet(0, 0, 0, 15);       
FuzzySet* lowWater = new FuzzySet(0, 15, 15, 40);  
FuzzySet* moderateWater = new FuzzySet(15, 40, 40, 60); 
FuzzySet* fullWater = new FuzzySet(40, 60, 100, 100);  

FuzzyInput* temperatureInput = new FuzzyInput(1);
FuzzyInput* humidityInput = new FuzzyInput(2);
FuzzyInput* soilMoistureInput = new FuzzyInput(3);
FuzzyOutput* pumpPowerOutput = new FuzzyOutput(1);


// --- Timing Variables for Non-Blocking Operation ---
unsigned long lastDhtReadTime = 0;
const unsigned long dhtReadInterval = 2000; // Read DHT every 2 seconds (DHT22 recommended)

unsigned long lastSoilReadTime = 0; // Stores the last time soil moisture was read
const unsigned long soilReadInterval = 500; // Defines the interval for reading soil moisture (in milliseconds)

unsigned long lastLogicDisplayTime = 0; // Stores the last time fuzzy logic was processed and display updated
const unsigned long logicDisplayInterval = 1000; // Defines the interval for logic processing and display updates (in milliseconds)

// --- Global Variables to Store Latest Sensor Data ---
float currentTemperature = NAN; // Stores the latest temperature reading. NAN indicates no valid reading yet.
float currentHumidity = NAN;    // Stores the latest humidity reading. NAN indicates no valid reading yet.
float currentSoilMoisture = NAN; // Stores the latest soil moisture reading. NAN indicates no valid reading yet.
float currentPumpPower = 0;     // Stores the calculated pump power. Initialized to 0.


// --- Fuzzy Rule Setup Functions ---
// This function defines the fuzzy rules that govern the irrigation system's behavior.
// Each rule maps combinations of input conditions (temperature, humidity, soil moisture)
// to an output action (pump power).
void setupFuzzyRules() {
  // Rule 1: Nếu Temp = Low, Humid = Low, Soil = Dry -> Tưới ít
  addRule(lowTemp, lowHumidity, drySoil, lowWater);

  // Rule 2: Nếu Temp = Low, Humid = Medium hoặc High -> Không tưới
  addRule(lowTemp, mediumHumidity, NULL, noWater); // Rule for Medium Humidity
  addRule(lowTemp, highHumidity, NULL, noWater);  // Rule for High Humidity

  // Rule 3: Nếu Temp = Low, Soil = Moist -> Không tưới
  addRule(lowTemp, NULL, moistSoil, noWater);

  // Rule 4: Nếu Soil = Wet -> Không tưới
  addRule(NULL, NULL, wetSoil, noWater); // This rule only considers soil moisture

  // Rule 5: Nếu Temp = Medium, Humid = Low, Soil = Dry -> Tưới đầy đủ
  addRule(mediumTemp, lowHumidity, drySoil, fullWater);

  // Rule 6: Nếu Temp = Medium, Humid = Low, Soil = Moist -> Tưới vừa
  addRule(mediumTemp, lowHumidity, moistSoil, moderateWater);

  // Rule 7: Nếu Temp = Medium, Humid = Medium, Soil = Dry -> Tưới vừa
  addRule(mediumTemp, mediumHumidity, drySoil, moderateWater);

  // Rule 8: Nếu Temp = Medium, Humid = Medium, Soil = Moist -> Tưới ít
  addRule(mediumTemp, mediumHumidity, moistSoil, lowWater);

  // Rule 9: Nếu Temp = Medium, Humid = High, Soil = Dry -> Tưới ít
  addRule(mediumTemp, highHumidity, drySoil, lowWater);

  // Rule 10: Nếu Temp = Medium, Humid = High, Soil = Moist -> Không tưới
  addRule(mediumTemp, highHumidity, moistSoil, noWater);

  // Rule 11: Nếu Temp = High, Soil = Dry -> Tưới đầy đủ
  addRule(highTemp, NULL, drySoil, fullWater);

  // Rule 12: Nếu Temp = High, Humid = High, Soil = Moist -> Tưới ít
  addRule(highTemp, highHumidity, moistSoil, lowWater);

  // Rule 13: Nếu Temp = High, Humid = Low hoặc Medium, Soil = Moist -> Tưới vừa
  addRule(highTemp, lowHumidity, moistSoil, moderateWater);
  addRule(highTemp, mediumHumidity, moistSoil, moderateWater);
}

void addRule(FuzzySet* tempSet, FuzzySet* humidSet, FuzzySet* soilSet, FuzzySet* outputSet) {
  FuzzyRuleAntecedent* antecedent = new FuzzyRuleAntecedent();
  int inputCount = (tempSet != NULL) + (humidSet != NULL) + (soilSet != NULL);
  
  if (inputCount == 0) {
    Serial.println("Error: Rule needs at least one input set");
    return;
  }
  
  if (inputCount == 1) {
    if (tempSet != NULL) antecedent->joinSingle(tempSet);
    else if (humidSet != NULL) antecedent->joinSingle(humidSet);
    else if (soilSet != NULL) antecedent->joinSingle(soilSet);
  } 
  else if (inputCount == 2) {
    if (tempSet != NULL && humidSet != NULL) {
      antecedent->joinWithAND(tempSet, humidSet);
    } else if (tempSet != NULL && soilSet != NULL) {
      antecedent->joinWithAND(tempSet, soilSet);
    } else if (humidSet != NULL && soilSet != NULL) {
      antecedent->joinWithAND(humidSet, soilSet);
    }
  } 
  else if (inputCount == 3) {
    FuzzyRuleAntecedent* tempHumid = new FuzzyRuleAntecedent(); // Intermediate antecedent
    tempHumid->joinWithAND(tempSet, humidSet);
    antecedent->joinWithAND(tempHumid, soilSet); // Then AND with the third
  }

  FuzzyRuleConsequent* consequent = new FuzzyRuleConsequent();
  consequent->addOutput(outputSet);

  static int ruleNum = 1; // Ensures unique rule numbers if library requires
  FuzzyRule* rule = new FuzzyRule(ruleNum++, antecedent, consequent);
  fuzzy->addFuzzyRule(rule);
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  delay(1000); // DHT sensor can take a moment to stabilize after begin

  // --- Fuzzy Logic Setup ---
  temperatureInput->addFuzzySet(lowTemp);
  temperatureInput->addFuzzySet(mediumTemp);
  temperatureInput->addFuzzySet(highTemp);
  fuzzy->addFuzzyInput(temperatureInput);

  humidityInput->addFuzzySet(lowHumidity);
  humidityInput->addFuzzySet(mediumHumidity);
  humidityInput->addFuzzySet(highHumidity);
  fuzzy->addFuzzyInput(humidityInput);

  soilMoistureInput->addFuzzySet(drySoil);
  soilMoistureInput->addFuzzySet(moistSoil);
  soilMoistureInput->addFuzzySet(wetSoil);
  fuzzy->addFuzzyInput(soilMoistureInput);

  pumpPowerOutput->addFuzzySet(noWater);
  pumpPowerOutput->addFuzzySet(lowWater);
  pumpPowerOutput->addFuzzySet(moderateWater);
  pumpPowerOutput->addFuzzySet(fullWater);
  fuzzy->addFuzzyOutput(pumpPowerOutput);
  
  setupFuzzyRules();

  // --- Display Setup ---
  myDisplay.begin();      
  myDisplay.drawLayout(); 
  
  // Initialize last read times to ensure first read happens quickly if desired,
  // or use 0 to adhere strictly to the first interval.
  // Setting them to 'millis() - interval' would trigger an immediate first read.
  // For simplicity, we'll let the first interval pass.
  lastDhtReadTime = millis(); 
  lastSoilReadTime = millis();
  lastLogicDisplayTime = millis();
}

void loop() {
  unsigned long currentTime = millis(); // Get current time once per loop

  // --- Task 1: Read DHT Sensor (Temperature and Humidity) ---
  if (currentTime - lastDhtReadTime >= dhtReadInterval) {
    lastDhtReadTime = currentTime;
    float temp = dht.readTemperature();
    float humid = dht.readHumidity();

    if (!isnan(temp)) {
      currentTemperature = temp;
    }
    if (!isnan(humid)) {
      currentHumidity = humid;
    }
    // Serial.println("DHT Updated"); // For debugging
  }

  // --- Task 2: Read Soil Moisture Sensor ---
  if (currentTime - lastSoilReadTime >= soilReadInterval) {
    lastSoilReadTime = currentTime;
    int soilMoistureRaw = analogRead(SOIL_MOISTURE_PIN);
    // Convert raw analog reading to percentage.
    // Assumes higher raw value means drier soil.
    // 4095.0 is the max ADC value (12-bit for ESP32).
    float calculatedSoilMoisture = (1.0 - (soilMoistureRaw / 4095.0)) * 100.0; 
    
    // Clamp values based on raw readings
    if (soilMoistureRaw >= 4095) calculatedSoilMoisture = 0; 
    // For typical resistive sensors, raw value 0 is very wet (100%)
    // The formula (1.0 - (0 / 4095.0)) * 100.0 gives 100.
    
    // Ensure calculatedSoilMoisture stays within 0-100 range if needed after formula
    currentSoilMoisture = constrain(calculatedSoilMoisture, 0.0, 100.0);
    // Serial.println("Soil Updated"); // For debugging
  }

  // --- Task 3: Process Fuzzy Logic and Update Display ---
  if (currentTime - lastLogicDisplayTime >= logicDisplayInterval) {
    lastLogicDisplayTime = currentTime;

    // Check if all sensor data is valid before using
    if (!isnan(currentTemperature) && !isnan(currentHumidity) && !isnan(currentSoilMoisture)) {
      fuzzy->setInput(1, currentTemperature);
      fuzzy->setInput(2, currentHumidity);
      fuzzy->setInput(3, currentSoilMoisture);
      
      fuzzy->fuzzify();
      currentPumpPower = fuzzy->defuzzify(1);

      // Serial Printing for Debugging
      Serial.print("Temp: "); Serial.print(currentTemperature, 1); Serial.print("°C, ");
      Serial.print("Humid: "); Serial.print(currentHumidity, 1); Serial.print("%, ");
      Serial.print("Soil: "); Serial.print(currentSoilMoisture, 1); Serial.print("%, ");
      Serial.print("Pump: "); Serial.print(currentPumpPower, 1); Serial.println("%");

      // Update the display with the latest processed values
      myDisplay.updateValues(currentTemperature, currentHumidity, currentSoilMoisture, currentPumpPower);
    } else {
      // Handle cases where sensor data might still be NAN (e.g., initial readings)
      // The display library should already handle NANs by printing "---"
      myDisplay.updateValues(currentTemperature, currentHumidity, currentSoilMoisture, currentPumpPower);
      Serial.println("Waiting for all sensor data to be valid...");
    }
  }
  
  //non-blocking tasks
}