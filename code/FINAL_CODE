#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>

#include <ArduinoJson.h>

#define SDA_PIN 21
#define SCL_PIN 22

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define SEALEVELPRESSURE_HPA 1013.25

const char* ssid = "HELLO";
const char* password = "12345678";

// Replace with your Render URL
const char* serverURL = "https://envirosense-backend.onrender.com/sensor";

Adafruit_BME680 bme;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void connectWiFi()
{
    if (WiFi.status() == WL_CONNECTED)
        return;

    Serial.println("Connecting to WiFi...");

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("WiFi Connected");
    Serial.print("IP Address : ");
    Serial.println(WiFi.localIP());
}

void setup()
{
    Serial.begin(115200);

    Wire.begin(SDA_PIN, SCL_PIN);

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println("OLED Failed");
        while (1);
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    if (!bme.begin())
    {
        Serial.println("BME680 Not Found");
        while (1);
    }

    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme.setGasHeater(320, 150);

    connectWiFi();

    Serial.println("-------------------------");
    Serial.println("EnviroSense Started");
    Serial.println("-------------------------");
}

void loop()
{
    connectWiFi();

    if (!bme.performReading())
    {
        Serial.println("Sensor Reading Failed");
        delay(2000);
        return;
    }

    float temperature = bme.temperature;
    float humidity = bme.humidity;
    float pressure = bme.pressure / 100.0;
    float gas = bme.gas_resistance / 1000.0;

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // Title
    display.setCursor(20, 0);
    display.println("ENVIROSENSE");

    // Horizontal line
    display.drawFastHLine(0, 10, 128, SSD1306_WHITE);

    // Temperature
    display.setCursor(0, 16);
    display.print("Temp: ");
    display.print(temperature, 1);
    display.print(" C");

    // Humidity
    display.setCursor(0, 28);
    display.print("Humid: ");
    display.print(humidity, 1);
    display.print(" %");

    // Pressure
    display.setCursor(0, 40);
    display.print("Pres: ");
    display.print(pressure, 1);
    display.print(" hPa");

    // Gas
    display.setCursor(0, 52);
    display.print("Gas: ");
    display.print(gas, 1);
    display.print(" KOhms");

    display.display();

    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http;

        http.begin(serverURL);

        http.addHeader("Content-Type", "application/json");

        StaticJsonDocument<256> doc;

        doc["temperature"] = temperature;
        doc["humidity"] = humidity;
        doc["pressure"] = pressure;
        doc["gas"] = gas;

        String json;

        serializeJson(doc, json);

        Serial.println("-------------------------");
        Serial.println("Sending Data...");
        Serial.println(json);

        int httpCode = http.POST(json);

        Serial.print("HTTP Response Code : ");
        Serial.println(httpCode);

        if (httpCode > 0)
        {
            Serial.println("Data Sent Successfully");
            Serial.println(http.getString());
        }
        else
        {
            Serial.println("Failed to Send Data");
        }

        http.end();
    }
    else
    {
        Serial.println("WiFi Disconnected");
    }

    delay(5000);
}
