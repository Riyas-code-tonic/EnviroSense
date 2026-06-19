#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>

#define SDA_PIN 6
#define SCL_PIN 7
#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME680 bme;

void setup() {
  Serial.begin(115200);

  // Initialize I2C on GPIO6 and GPIO7
  Wire.begin(SDA_PIN, SCL_PIN);

  // Initialize BME680
  if (!bme.begin()) {     // If your sensor uses address 0x77, use bme.begin(0x77)
    Serial.println("Could not find a valid BME680 sensor!");
    while (1);
  }

  // Oversampling settings
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);

  // Gas sensor heater
  bme.setGasHeater(320, 150);

  Serial.println("BME680 initialized!");
}

void loop() {
  if (!bme.performReading()) {
    Serial.println("Failed to perform reading");
    return;
  }

  Serial.print("Temperature = ");
  Serial.print(bme.temperature);
  Serial.println(" °C");

  Serial.print("Pressure = ");
  Serial.print(bme.pressure / 100.0);
  Serial.println(" hPa");

  Serial.print("Humidity = ");
  Serial.print(bme.humidity);
  Serial.println(" %");

  Serial.print("Gas Resistance = ");
  Serial.print(bme.gas_resistance / 1000.0);
  Serial.println(" KOhms");

  Serial.print("Approx. Altitude = ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");

  Serial.println();
  delay(100);
}