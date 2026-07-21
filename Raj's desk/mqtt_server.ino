#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>

//-------------------- I2C --------------------
#define SDA_PIN 21
#define SCL_PIN 22
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

//-------------------- Deep Sleep --------------------
#define uS_TO_S_FACTOR 1000000ULL
#define TIME_TO_SLEEP 30

//-------------------- WiFi --------------------
#define WLAN_SSID "Username"
#define WLAN_PASS "Password123"

//-------------------- Adafruit IO --------------------
#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883
#define AIO_USERNAME "...................."
#define AIO_KEY "........................"

//-------------------- Objects --------------------
Adafruit_BME680 bme;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

WiFiClient client;

Adafruit_MQTT_Client mqtt(
  &client,
  AIO_SERVER,
  AIO_SERVERPORT,
  AIO_USERNAME,
  AIO_KEY);

// Feeds
Adafruit_MQTT_Publish tempFeed(&mqtt, AIO_USERNAME "/feeds/temperature");
Adafruit_MQTT_Publish humFeed(&mqtt, AIO_USERNAME "/feeds/humidity");
Adafruit_MQTT_Publish pressFeed(&mqtt, AIO_USERNAME "/feeds/pressure");
Adafruit_MQTT_Publish gasFeed(&mqtt, AIO_USERNAME "/feeds/gas");

void goToSleep() {
  Serial.println("Going to sleep...");
  Serial.flush();

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}

void connectWiFi() {

  if (WiFi.status() == WL_CONNECTED)
    return;

  Serial.print("Connecting WiFi");

  WiFi.begin(WLAN_SSID, WLAN_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");
}

void connectMQTT() {

  while (!mqtt.connected()) {

    Serial.print("Connecting to Adafruit IO... ");

    int8_t ret = mqtt.connect();

    if (ret == 0) {
      Serial.println("Connected!");
    }
    else {
      Serial.println(mqtt.connectErrorString(ret));
      mqtt.disconnect();
      delay(5000);
    }
  }
}

void setup() {

  Serial.begin(115200);
  delay(500);

  Wire.begin(SDA_PIN, SCL_PIN);

  // OLED
  if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    display.clearDisplay();
    // display.display();
    // delay(2000);
  }

  // BME680
  if (!bme.begin()) {
    Serial.println("BME680 not found!");
    goToSleep();
  }

  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150);

  if (!bme.performReading()) {
    Serial.println("Sensor read failed");
    goToSleep();
  }

  float temperature = bme.temperature;
  float humidity = bme.humidity;
  float pressure = bme.pressure / 100.0;
  float gas = bme.gas_resistance / 1000.0;

  // OLED Display
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(32,0);
  display.println("ENVIROSENSE");
  display.drawFastHLine(0,10,128,SSD1306_WHITE);

  display.setCursor(0,16);
  display.printf("Temp : %.1f C",temperature);

  display.setCursor(0,28);
  display.printf("Hum  : %.1f %%",humidity);

  display.setCursor(0,40);
  display.printf("Pres : %.1f hPa",pressure);

  display.setCursor(0,52);
  display.printf("Gas  : %.1f KOhm",gas);

  display.display();

  // WiFi & MQTT
  connectWiFi();
  connectMQTT();

  mqtt.processPackets(100);
  mqtt.ping();

  // Publish to Adafruit IO
  tempFeed.publish(temperature);
  humFeed.publish(humidity);
  pressFeed.publish(pressure);
  gasFeed.publish(gas);

  Serial.println("Published to Adafruit IO");

  delay(3000);

  goToSleep();
}

void loop() {
}