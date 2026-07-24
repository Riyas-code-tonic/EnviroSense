#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>

// --- Configuration Pins & Constants ---
#define SDA_PIN 6
#define SCL_PIN 7
#define SEALEVELPRESSURE_HPA (1013.25)

// --- OLED Configuration ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1 
#define SCREEN_ADDRESS 0x3C // Change to 0x3D if your OLED doesn't respond
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- BME680 Configuration ---
Adafruit_BME680 bme;

void setup() {
  Serial.begin(115200);

  // 1. Initialize I2C on GPIO6 and GPIO7 for BOTH devices
  Wire.begin(SDA_PIN, SCL_PIN);

  // 2. Initialize the OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); 
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 20);
  display.println("Initializing...");
  display.display();

  // 3. Initialize BME680 sensor
  // Note: If it fails, try changing to bme.begin(0x77) depending on your module address
  if (!bme.begin(0x77)) {     
    Serial.println("Could not find a valid BME680 sensor!");
    display.clearDisplay();
    display.setCursor(0, 20);
    display.println("BME680 Error!");
    display.display();
    while (1);
  }

  // BME680 Oversampling settings
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);

  // Gas sensor heater
  bme.setGasHeater(320, 150);

  Serial.println("System Initialized!");
}

void loop() {
  // Fetch data from sensor
  if (!bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }

  // --- Print to Serial Monitor (Optional) ---
  Serial.print("Temperature = "); Serial.print(bme.temperature); Serial.println(" *C");
  Serial.print("Humidity = ");    Serial.print(bme.humidity);    Serial.println(" %");

  // --- Print to OLED Display ---
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Line 1: Title
  display.setCursor(20, 0);
  display.println("ENVIROSENSE");
  display.drawFastHLine(0, 10, 128, SSD1306_WHITE); // Draw a separation line

  // Line 2: Temperature
  display.setCursor(0, 16);
  display.print("Temp: ");
  display.print(bme.temperature, 1); // 1 decimal place
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

  // Line 5: Gas Resistance (Air Quality metric)
  display.setCursor(0, 52);
  display.print("Gas: ");
  display.print(bme.gas_resistance / 1000.0, 1);
  display.print(" KOhms");

  // Refresh display with new data
  display.display();

  // Slow down readings slightly for display stability
  delay(1000); 
}
