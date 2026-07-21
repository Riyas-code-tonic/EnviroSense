#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <ESP_Mail_Client.h>
#include "S:\Arduino\libraries\Adafruit_GFX_Library\Fonts\FreeMonoBold9pt7b.h"

// ================================================================
// PERSISTENT RTC VARIABLES (Survive Deep Sleep)
// ================================================================
RTC_DATA_ATTR bool wasAboveLimit = false;
RTC_DATA_ATTR int minutesAboveLimit = 0;  // Counts consecutive high-temp cycles

// Email Alert Variable
const float TEMP_LIMIT = 35.0;
const int TriggerAlertinMins = 5; //After 5 minutes of continuous high temperature, send another email alert

//-------------------- I2C --------------------
#define SDA_PIN 21
#define SCL_PIN 22
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

//-------------------- Deep Sleep --------------------
#define uS_TO_S_FACTOR 1000000ULL
#define TIME_TO_SLEEP 60

//-------------------- WiFi --------------------
#define WLAN_SSID "Raj_4g"
#define WLAN_PASS "....."

//-------------------- Adafruit IO --------------------
#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883
#define AIO_USERNAME "Rajnarayan"
#define AIO_KEY ".........."

//===========================
// Gmail Credentials
//===========================
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

#define AUTHOR_EMAIL "abcd@gmail.com"
#define AUTHOR_PASSWORD "......"

#define RECIPIENT_EMAIL1 "2024.rajnarayan.hazra@ves.ac.in"
// #define RECIPIENT_EMAIL2 "2024.riya.pailwan@ves.ac.in"
// #define RECIPIENT_EMAIL3 "asawari.dudwadkar@ves.ac.in"

//===========================

//-------------------- Objects --------------------

Adafruit_BME680 bme;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

WiFiClient client;

SMTPSession smtp;
Session_Config config;
SMTP_Message message;

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

//------------------------------------------------
// Callback
//------------------------------------------------
void smtpCallback(SMTP_Status status) {
  Serial.println(status.info());

  if (status.success()) {
    Serial.println("Email sent successfully!");
  } else {
    Serial.println("Failed to send email.");
  }
}

//------------------------------------------------
// Send Email
//------------------------------------------------
void sendEmail(String ledState) {
  message.sender.name = "EnviroSense";
  message.sender.email = AUTHOR_EMAIL;

  message.subject = "The threshold has been exceeded";

  message.addRecipient("Receiver", RECIPIENT_EMAIL1);
  // message.addRecipient("Receiver", RECIPIENT_EMAIL2);
  // message.addRecipient("Receiver", RECIPIENT_EMAIL3);

  String body;

  body += "Hello,\n\n";
  body += "The temperature has 🚨exceeded the recommended threshold.\n\n";
  body += "Current Temperature: ";
  body += ledState;
  body += " °C 🌡️\n\n";
  body += "This is an automated email.\n";
  body += "\nRegards,\nEnviroSense🌳";

  message.text.content = body.c_str();
  message.text.charSet = "utf-8";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  smtp.callback(smtpCallback);

  if (!smtp.connect(&config)) {
    Serial.println("SMTP Connection Failed");
    return;
  }

  if (!MailClient.sendMail(&smtp, &message)) {
    Serial.println("Error Sending Email");
    Serial.println(smtp.errorReason());
  }

  message.clearRecipients();
}

//---------------------------------------
//Deep Sleep
//---------------------------------------

void goToSleep() {

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(20, 32);
  display.println("Deep Sleep Mode");
  Serial.println("Going to sleep...");
  display.display();
  delay(2000);
  display.clearDisplay();
  display.display();
  Serial.flush();

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}

//---------------------------------------
//Wifi Connect
//---------------------------------------
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


//---------------------------------------
//MQTT Connect
//---------------------------------------
void connectMQTT() {

  while (!mqtt.connected()) {

    Serial.print("Connecting to Adafruit IO... ");

    int8_t ret = mqtt.connect();

    if (ret == 0) {
      Serial.println("Connected!");
    } else {
      Serial.println(mqtt.connectErrorString(ret));
      mqtt.disconnect();
      delay(5000);
    }
  }
}

//-------Connect Email------------------------
void connectEmail() {
  // SMTP Configuration
  config.server.host_name = SMTP_HOST;
  config.server.port = SMTP_PORT;

  config.login.email = AUTHOR_EMAIL;
  config.login.password = AUTHOR_PASSWORD;

  config.login.user_domain = "";
}
//-------Logo Splash Screen------------
void logo() {

  static const unsigned char PROGMEM image_tree_bits[] = { 0x00, 0x33, 0x30, 0x00, 0x00, 0x33, 0x30, 0x00, 0x0c,
                                                           0xcc, 0xf3, 0x30, 0x0c, 0xcc, 0xf3, 0x30, 0x33, 0x3f, 0x0c, 0x3c, 0x33, 0x3f, 0x0c, 0x3c, 0x0f, 0x03, 0xcf, 0xc0, 0x0f,
                                                           0x03, 0xcf, 0xc0, 0x33, 0xf3, 0xfc, 0xcc, 0x33, 0xf3, 0xfc, 0xcc, 0xc0, 0x3f, 0xc3, 0x33, 0xc0, 0x3f, 0xc3, 0x33, 0x3c,
                                                           0xcf, 0x0f, 0x3c, 0x3c, 0xcf, 0x0f, 0x3c, 0xcf, 0xcf, 0x3c, 0xc0, 0xcf, 0xcf, 0x3c, 0xc0, 0x30, 0xff, 0xff, 0xf3, 0x30,
                                                           0xff, 0xff, 0xf3, 0x0f, 0x3f, 0xf0, 0x3c, 0x0f, 0x3f, 0xf0, 0x3c, 0x33, 0x03, 0xcc, 0xc3, 0x33, 0x03, 0xcc, 0xc3, 0x0c,
                                                           0xc3, 0xc0, 0x0c, 0x0c, 0xc3, 0xc0, 0x0c, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x00,
                                                           0x03, 0xc0, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00 };

  display.clearDisplay();
  // tree
  display.drawBitmap(45, 5, image_tree_bits, 32, 32, 1);
  // string 2
  display.setTextColor(1);
  display.setTextWrap(false);
  display.setFont(&FreeMonoBold9pt7b);
  display.setCursor(4, 52);
  display.print("Envirosense");
  display.display();
}
void checkAlert(float temperature) {
  //----------Email Alert Function-------------
  String strTemp = String(temperature, 2);
  // 2. THRESHOLD EVALUATION WITH 5-MINUTE RE-ALERT
  if (temperature > TEMP_LIMIT) {

    if (!wasAboveLimit) {
      // Initial spike above 30°C -> Send immediate email
      Serial.println("🚨 RISING EDGE: Temp crossed above 30°C! Sending alert...");
      sendEmail(strTemp);

      wasAboveLimit = true;
      minutesAboveLimit = 1;  // Start counter at 1 minute
    } else {
      // Temperature is STILL high on subsequent 1-minute wakeups
      minutesAboveLimit++;
      Serial.print("⚠️ Temp still high. Minute count: ");
      Serial.println(minutesAboveLimit);

      if (minutesAboveLimit >= TriggerAlertinMins) {
        // 5 minutes have passed -> Re-trigger email alert
        Serial.println("🚨 5-MINUTE REMINDER: Temp still above 30°C! Sending alert...");
        sendEmail(strTemp);

        minutesAboveLimit = 0;  // Reset counter for the next 5-minute cycle
      } else {
        // Waiting for the 5-minute mark to pass
        Serial.println("ℹ️ Temp still high — skipping duplicate email alert.");
      }
    }
  } else {
    // Temperature is NORMAL (<= 30°C)
    if (wasAboveLimit) {
      Serial.println("✅ RESET: Temp dropped below 30°C.");
    } else {
      Serial.println("✅ Temp normal.");
    }

    // Reset RTC flags
    wasAboveLimit = false;
    minutesAboveLimit = 0;
  }
}

void setup() {

  Serial.begin(115200);
  delay(500);

  Wire.begin(SDA_PIN, SCL_PIN);

  // OLED
  if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    display.clearDisplay();
    logo();
    delay(2000);

    // Reset to the regular font for the main sensor display
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setFont();
  }

  // BME680
  if (!bme.begin()) {
    Serial.println("BME680 not found!");
    goToSleep();
  }

  //BME Sensor reading & Display-------------------------------------------
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

  display.setCursor(32, 0);
  display.println("ENVIROSENSE");
  display.drawFastHLine(0, 10, 128, SSD1306_WHITE);

  display.setCursor(0, 16);
  display.printf("Temp : %.1f C", temperature);

  display.setCursor(0, 28);
  display.printf("Hum  : %.1f %%", humidity);

  display.setCursor(0, 40);
  display.printf("Pres : %.1f hPa", pressure);

  display.setCursor(0, 52);
  display.printf("Gas  : %.1f KOhm", gas);
  display.display();


  //-----------------------------------------------------------------------------

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
  connectEmail();
  checkAlert(temperature);
  delay(3000);
  goToSleep();
}

void loop() {
}