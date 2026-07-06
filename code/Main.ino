#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// =====================================================
// WIFI
// =====================================================

const char* ssid = "Harshhp";
const char* password = "HARSH2006#";

// =====================================================
// MQTT
// =====================================================

const char* mqtt_server = "192.168.0.106";
WiFiClient espClient;
PubSubClient client(espClient);
#define ONE_WIRE_BUS 4

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);

// =====================================================
// ACS712
// =====================================================

#define ACS_PIN 34

// =====================================================
// LCD
// =====================================================

LiquidCrystal_I2C lcd(0x27, 16, 2);

// =====================================================
// SHARED DATA
// =====================================================

String systemStatus = "BOOTING";

struct Telemetry {
  float temperature;
  float current;
  float power;
};

Telemetry telemetryData;

SemaphoreHandle_t telemetryMutex;

// =====================================================
// WIFI
// =====================================================

void connectWiFi() {

  WiFi.begin(ssid, password);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected");

  systemStatus = "WIFI OK";

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("WiFi Connected");

  delay(1000);
}

// =====================================================
// MQTT
// =====================================================

void reconnectMQTT() {

  while (!client.connected()) {

    Serial.print("Connecting MQTT...");

    if (client.connect("SolSenseESP32")) {

      Serial.println(" Connected");

      systemStatus = "MQTT OK";

    } else {

      Serial.print(" Failed rc=");
      Serial.println(client.state());

      systemStatus = "MQTT FAIL";

      delay(2000);
    }
  }
}

// =====================================================
// ACS712
// =====================================================

float readCurrent() {

  long sum = 0;

  for (int i = 0; i < 500; i++) {

    sum += analogRead(ACS_PIN);

    delay(1);
  }

  float adc = sum / 500.0;

  float voltage = adc * (3.3 / 4095.0);

  float current = (voltage - 2.5) / 0.185;

  if (current < 0)
    current = -current;

  return current;
}

// =====================================================
// SENSOR TASK
// =====================================================

void SensorTask(void *pvParameters)
{
  while(true)
  {
    ds18b20.requestTemperatures();

    float temperature =
      ds18b20.getTempCByIndex(0);

    if(temperature == DEVICE_DISCONNECTED_C)
      temperature = -999;

    float current = readCurrent();

    float power = current * 12.0;

    xSemaphoreTake(
      telemetryMutex,
      portMAX_DELAY
    );

    telemetryData.temperature = temperature;
    telemetryData.current = current;
    telemetryData.power = power;

    xSemaphoreGive(
      telemetryMutex
    );

    vTaskDelay(
      pdMS_TO_TICKS(1000)
    );
  }
}

// =====================================================
// MQTT TASK
// =====================================================

void MQTTTask(void *pvParameters)
{
  while(true)
  {
    if(WiFi.status() != WL_CONNECTED)
    {
      systemStatus = "WIFI LOST";
      connectWiFi();
    }

    if(!client.connected())
    {
      reconnectMQTT();
    }

    client.loop();

    Telemetry data;

    xSemaphoreTake(
      telemetryMutex,
      portMAX_DELAY
    );

    data = telemetryData;

    xSemaphoreGive(
      telemetryMutex
    );

    String payload = "{";
    payload += "\"temperature\":";
    payload += String(data.temperature,2);
    payload += ",";
    payload += "\"current\":";
    payload += String(data.current,2);
    payload += ",";
    payload += "\"power\":";
    payload += String(data.power,2);
    payload += "}";

    bool sent = client.publish(
      "solsense/telemetry",
      payload.c_str()
    );

    if(sent)
      systemStatus = "MQTT OK";
    else
      systemStatus = "PUB FAIL";

    Serial.println("====================");
    Serial.println(payload);

    vTaskDelay(
      pdMS_TO_TICKS(5000)
    );
  }
}

// =====================================================
// LCD TASK
// =====================================================

void LCDTask(void *pvParameters)
{
  while(true)
  {
    Telemetry data;

    xSemaphoreTake(
      telemetryMutex,
      portMAX_DELAY
    );

    data = telemetryData;

    xSemaphoreGive(
      telemetryMutex
    );

    // PAGE 1

    lcd.clear();

    lcd.setCursor(0,0);
    lcd.print("T:");
    lcd.print(data.temperature,1);
    lcd.print("C");

    lcd.setCursor(10,0);
    lcd.print("I:");
    lcd.print(data.current,1);

    lcd.setCursor(0,1);
    lcd.print("P:");
    lcd.print(data.power,1);
    lcd.print("W");

    vTaskDelay(
      pdMS_TO_TICKS(2500)
    );

    // PAGE 2

    lcd.clear();

    lcd.setCursor(0,0);
    lcd.print("WiFi:");
    lcd.print(WiFi.RSSI());

    lcd.setCursor(0,1);
    lcd.print(systemStatus);

    vTaskDelay(
      pdMS_TO_TICKS(2500)
    );
  }
}

// =====================================================
// SETUP
// =====================================================

void setup()
{
  Serial.begin(115200);

  Wire.begin(18,19);

  ds18b20.begin();

  lcd.init();
  lcd.backlight();

  lcd.setCursor(0,0);
  lcd.print("SolSense AI");

  lcd.setCursor(0,1);
  lcd.print("Starting...");

  delay(2000);

  connectWiFi();

  client.setServer(
    mqtt_server,
    1883
  );

  reconnectMQTT();

  telemetryMutex =
    xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(
    SensorTask,
    "SensorTask",
    4096,
    NULL,
    2,
    NULL,
    1
  );

  xTaskCreatePinnedToCore(
    MQTTTask,
    "MQTTTask",
    6144,
    NULL,
    2,
    NULL,
    0
  );

  xTaskCreatePinnedToCore(
    LCDTask,
    "LCDTask",
    4096,
    NULL,
    1,
    NULL,
    0
  );
}

// =====================================================
// LOOP
// =====================================================

void loop()
{
  vTaskDelete(NULL);
}
