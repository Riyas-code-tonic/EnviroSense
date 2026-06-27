#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include "FS.h"
#include <LittleFS.h>

// --- Configuration Pins & Constants ---
#define SDA_PIN 6
#define SCL_PIN 7
#define SEALEVELPRESSURE_HPA (1013.25)

// --- OLED Configuration ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1 
#define SCREEN_ADDRESS 0x3C 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- BME680 Configuration ---
Adafruit_BME680 bme;

// -------- Log File Settings --------
const char* dataFile = "/env_data.csv";
#define MAX_FILE_SIZE 50000        // 50 KB

// --- Deep Sleep & LittleFS Settings ---
#define uS_TO_S_FACTOR 1000000ULL 
#define TIME_TO_SLEEP  60          // Sleep for 60 seconds

void manageLogFile() {
  if (!LittleFS.exists(dataFile))
    return;
  File file = LittleFS.open(dataFile, FILE_READ);
  if (!file) {
    Serial.println("Unable to open log file.");
    return;
  }

  size_t fileSize = file.size();
  file.close();
  Serial.print("Current Log Size: ");
  Serial.print(fileSize);
  Serial.println(" bytes");

  if (fileSize >= MAX_FILE_SIZE) {
    Serial.println("Storage limit reached.");
    Serial.println("Deleting old log file...");

    LittleFS.remove(dataFile);

    File newFile = LittleFS.open(dataFile, FILE_WRITE);
    if (newFile) {
      newFile.println("Temperature,Humidity,Pressure,Gas");
      newFile.close();
      Serial.println("New log file created.");
    }
  }
}

void setup() {
  Serial.begin(115200); 
  delay(500);       // Give the computer half second to recognize the native USB port

  Wire.begin(SDA_PIN, SCL_PIN);

  // 2. Initialize LittleFS
  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS Mount Failed");
    return;
  }

  manageLogFile();

// Create CSV file if it doesn't exist
  if (!LittleFS.exists(dataFile)) {
      File file = LittleFS.open(dataFile, FILE_WRITE);

      if (file) {
          file.println("Temperature,Humidity,Pressure,Gas");
          file.close();
      }
  }

  // 3. Initialize the OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 20);
    display.println("Initializing...");
    display.display();
  }

  // 4. Initialize BME680 sensor
  if (!bme.begin(0x77)) {     
    Serial.println("Could not find a valid BME680 sensor!");
    display.clearDisplay();
    display.setCursor(0, 20);
    display.println("BME680 Error!");
    display.display();
    // Do not use an infinite while(1) loop here, otherwise it never goes to sleep to retry!
  } else {
    // BME680 Oversampling settings
    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme.setGasHeater(320, 150);
  }

  // 5. Fetch data from sensor
  if (!bme.performReading()) {
    Serial.println("Failed to perform reading :(");
  } else {
    // --- Print to OLED Display ---
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // Line 1: Title
    display.setCursor(20, 0);
    display.println("ENVIROSENSE");
    display.drawFastHLine(0, 10, 128, SSD1306_WHITE); 

    // Line 2: Temperature
    display.setCursor(0, 16);
    display.print("Temp: ");
    display.print(bme.temperature, 1); 
    display.print(" C");

    // Line 3: Humidity
    display.setCursor(0, 28);
    display.print("Humid: ");
    display.print(bme.humidity, 1);
    display.print(" %");

    // Line 4: Pressure
    display.setCursor(0, 40);
    display.print("Pres: ");
    display.print(bme.pressure / 100.0, 1);
    display.print(" hPa");

    // Line 5: Gas Resistance 
    display.setCursor(0, 52);
    display.print("Gas: ");
    display.print(bme.gas_resistance / 1000.0, 1);
    display.print(" KOhms");

    display.display();

    // Create a comma-separated string for the CSV file: Temp,Humidity,Pressure,Gas
    String logData =
    String(bme.temperature,2) + "," +
    String(bme.humidity,2) + "," +
    String(bme.pressure/100.0,2) + "," +
    String(bme.gas_resistance/1000.0,2) + "\n";
                     
    File file = LittleFS.open(dataFile, FILE_APPEND);
    if (file) {
      file.print(logData);
      file.close();
      Serial.println("Logged successfully: " + logData);
    } else {
      Serial.println("Failed to open file for appending");
    }
  }

  // 6. Go to Deep Sleep
  delay(10000); 
  
  Serial.println("Going to sleep now...");
  display.clearDisplay(); // Turn off OLED to save power
  display.display();
  
  Serial.flush(); 
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}

void loop() {
  // Empty. Deep sleep architecture relies purely on setup()
}
