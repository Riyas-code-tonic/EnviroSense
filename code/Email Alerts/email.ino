#include <WiFi.h>
#include <ESP_Mail_Client.h>

//===========================
// WiFi Credentials
//===========================
#define WIFI_SSID       "username"  //Your WiFi SSID
#define WIFI_PASSWORD   "password123"

//===========================
// Gmail Credentials
//===========================
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

#define AUTHOR_EMAIL    ".....@gmail.com"   //Sender mail ID
#define AUTHOR_PASSWORD ".........."  // Sender app passcode npt email one

#define RECIPIENT_EMAIL1 "abcd@gmail.com"  //Receiver mail ID

//===========================
// LED Pin
//===========================
#define LED_PIN 2

SMTPSession smtp;
Session_Config config;
SMTP_Message message;

//------------------------------------------------
// Callback
//------------------------------------------------
void smtpCallback(SMTP_Status status)
{
  Serial.println(status.info());

  if (status.success())
  {
    Serial.println("Email sent successfully!");
  }
  else
  {
    Serial.println("Failed to send email.");
  }
}

//------------------------------------------------
// Send Email
//------------------------------------------------
void sendEmail(String ledState)
{
  message.sender.name = "EnviroSense";
  message.sender.email = AUTHOR_EMAIL;

  message.subject = "ESP32 LED Status";

  message.addRecipient("Receiver", RECIPIENT_EMAIL1);

  String body;

  body += "Hello,\n\n";
  body += "Led Status Update \n\n";
  body += "Current State: ";
  body += ledState;
  body += "\n\n";
  body += "This is an automated email.\n";
  body += "\nRegards,\nESP32";

  message.text.content = body.c_str();
  message.text.charSet = "utf-8";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  smtp.callback(smtpCallback);

  if (!smtp.connect(&config))
  {
    Serial.println("SMTP Connection Failed");
    return;
  }

  if (!MailClient.sendMail(&smtp, &message))
  {
    Serial.println("Error Sending Email");
    Serial.println(smtp.errorReason());
  }

  message.clearRecipients();
}

//------------------------------------------------
// Setup
//------------------------------------------------
void setup()
{
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.println();
  Serial.println("Connecting to WiFi...");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // SMTP Configuration
  config.server.host_name = SMTP_HOST;
  config.server.port = SMTP_PORT;

  config.login.email = AUTHOR_EMAIL;
  config.login.password = AUTHOR_PASSWORD;

  config.login.user_domain = "";

  Serial.println("------------------------------------");
  Serial.println("Type ON or OFF in Serial Monitor");
  Serial.println("------------------------------------");
}

//------------------------------------------------
// Loop
//------------------------------------------------
void loop()
{
  if (Serial.available())
  {
    String command = Serial.readStringUntil('\n');
    command.trim();
    command.toUpperCase();

    if (command == "ON")
    {
      digitalWrite(LED_PIN, HIGH);
      Serial.println("LED ON");
      sendEmail("ON");
    }
    else if (command == "OFF")
    {
      digitalWrite(LED_PIN, LOW);
      Serial.println("LED OFF");
      sendEmail("OFF");
    }
    else
    {
      Serial.println("Invalid Command");
      Serial.println("Type ON or OFF");
    }
  }
}
