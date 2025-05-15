/*********************************************************************************************************
* LILYGO T-Display-S3 DS18B20 Temperature Sensor Project
*
* Description:
*   This code reads temperature data from a DS18B20 sensor and displays it on the built-in
*    screen of the LilyGO T-Display-S3 using the TFT_eSPI library.
*   The code uses state machine logic with millis to avoid using delays, which is code blocking.
*   If the sensor reading is invalid, it displays "Sensor Not Connected" on the screen.
*   The values are only updated if there is a change in the sensor readings.
*
* How It Works:
*   1. Sensor Reading: The code reads temperature data from the DS18B20 sensor as a float value
*      at regular 2 second intervals.
*   2. Display: The sensor data is updated on the screen only when there is a difference in readings.
*   3. State Machine: A state machine is used to manage the timing of sensor readings and display updates.
*   4. Sensor Check: If the sensor readings are invalid, it displays "Sensor Not Connected".
*
* Pin Connections:
*   - DS18B20 Data Pin -> GPIO1
*   - LCD Backlight    -> GPIO15
*   - Ground           -> GND
*   - Voltage          -> 3.3V
*
* Notes:
*   - The OneWire and DallasTemperature libraries are used to read data from the DS18B20 sensor.
*   - The TFT_eSPI library is configured to work with the LilyGO T-Display-S3, providing an easy way to
*      display information on the built-in screen.
*   - DS18B20 pinout: [-] = GND | [S] = Signal PIN | [MIDDLE PIN] = Supply Voltage PIN.
*
* DS18B20 Specifications:
*   - Communicates over one-wire bus communication
*   - Power supply range: 3.0V to 5.5V
*   - Resolution: 9-bit to 12-bit
*   - Operating temperature range: -55ºC to +125ºC
*   - Accuracy +/-0.5 ºC (between the range -10ºC to 85ºC)

**********************************************************************************************************/

/*************************************************************
******************* INCLUDES & DEFINITIONS *******************
**************************************************************/

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// TFT_eSPI
TFT_eSPI tft = TFT_eSPI();

// OneWire & DS18B20 Sensor
#define ONE_WIRE_BUS 1               // GPIO where the DS18B20 is connected to
OneWire oneWire(ONE_WIRE_BUS);       // setup a oneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire); // pass our oneWire reference to Dallas Temperature sensor

// State Machine States
enum class State {
  READ_SENSOR,    // state for reading sensor data
  WAIT,           // state for waiting for sensor to process data
  UPDATE_DISPLAY, // state for updating the display
};

// Global variables
State currentState = State::READ_SENSOR; // initial state
unsigned long previousMillis = 0;        // for non-blocking timing
int sensorDelayInterval = 750;           // allow time for sensor to process data (750ms)
int sensorReadInterval = 2000;           // read sensor every 2 seconds
float temperatureC = 0.0;                // variable to store temperature reading in Celcius
float temperatureF = 0.0;                // variable to store temperature reading in Fahrenheit
float previousTemperatureC = 0.0;        // store previous Celcius value
bool valueChanged = false;               // flag to indicate when values have changed
bool sensorConnected = true;             // flag to track sensor connection
bool firstRun = true;                    // flag for initial screen setup

// Text positions for dynamic updates
#define CELSIUS_Y 85
#define FAHRENHEIT_Y 133
#define ERROR_Y 85


/*************************************************************
*********************** HELPER FUNCTIONS *********************
**************************************************************/

// Function to draw the static screen elements
void drawStaticScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  
  tft.println("---------------------------");
  tft.println(" DS18B20 Sensor Module");
  tft.println("---------------------------");
  
  if (sensorConnected) {
    tft.println("\nTemp in Celcius:");
    tft.println("\n\nTemp in Fahrenheit:");
  }
}

// Function to update temperature values on screen
void updateTemperatureValues() {
  if (sensorConnected) {
    // Update Celsius value
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(0, CELSIUS_Y);
    tft.print(String(temperatureC) + " C    ");
    
    // Update Fahrenheit value
    tft.setCursor(0, FAHRENHEIT_Y);
    tft.print(String(temperatureF) + " F    ");
  }
}

// Function to show sensor error message
void showSensorError() {
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(0, ERROR_Y);
  tft.println("\n!! Sensor Not Connected !!");
}


/*************************************************************
*********************** MAIN FUNCTIONS ***********************
**************************************************************/

// SETUP
void setup() {
  // Initialize the TFT display
  tft.init();
  tft.setRotation(0);                     // adjust rotation (0 & 2 portrait | 1 & 3 landscape)
  tft.fillScreen(TFT_BLACK);              // clear screen
  tft.setTextFont(2);                     // set the font
  tft.setTextColor(TFT_WHITE, TFT_BLACK); // set text colour

  tft.println("Initialising...\n");

  delay(1000);

  // Initialize the DS18B20 sensor
  sensors.begin();
  sensors.setResolution(12); // set resolution to 12 bits for higher precision

  // Draw the initial static screen
  drawStaticScreen();
  firstRun = false;
}

// MAIN LOOP
void loop() {
  unsigned long currentMillis = millis(); // get the current millis time

  // State Machine Logic
  switch (currentState) {
    case State::READ_SENSOR:
      // Request temperature reading from the DS18B20 sensor
      sensors.requestTemperatures();

      // Move to the WAIT state to allow the sensor to process data
      currentState = State::WAIT;
      previousMillis = currentMillis; // record the time when we entered the WAIT state
      break;

    case State::WAIT:
      // Wait for the sensor to process the data (non-blocking)
      if (currentMillis - previousMillis >= sensorDelayInterval) {
        // After waiting, read the temperature
        temperatureC = sensors.getTempCByIndex(0);                    // get temperature in Celsius
        temperatureF = DallasTemperature::toFahrenheit(temperatureC); // convert to Fahrenheit

        // Check if the reading is valid
        bool currentConnectionState = (temperatureC != DEVICE_DISCONNECTED_C);
        
        // Check if connection status changed
        if (currentConnectionState != sensorConnected) {
          sensorConnected = currentConnectionState;
          drawStaticScreen(); // Redraw the entire screen if connection status changed
          if (!sensorConnected) {
            showSensorError();
          }
        }
        
        // Check if the readings have changed (using a threshold to avoid floating-point precision issues)
        if (sensorConnected && abs(temperatureC - previousTemperatureC) >= 0.1) {
          valueChanged = true;
          previousTemperatureC = temperatureC;
        }

        // Move to the UPDATE_DISPLAY state
        currentState = State::UPDATE_DISPLAY;
      }
      break;

    case State::UPDATE_DISPLAY:
      // Update the display with the new sensor data if changed
      if (valueChanged && sensorConnected) {
        updateTemperatureValues();
        valueChanged = false;
      }

      // Move back to the READ_SENSOR state after the sensorReadInterval
      if (currentMillis - previousMillis >= sensorReadInterval) {
        currentState = State::READ_SENSOR;
      }
      break;

    default:
      // Default case (should not happen)
      currentState = State::READ_SENSOR;
      break;
  }
}
