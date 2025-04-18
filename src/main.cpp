#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>

// Replace these with your WiFi credentials:
const char* ssid = "MERCUSYS";
const char* password = "17122001";

// Create a web server on port 80
WebServer server(80);

// Global variables for tire data
float pressureFL, temperatureFL;
float pressureFR, temperatureFR;
float pressureRL, temperatureRL;
float pressureRR, temperatureRR;

// Timer variables for updating data every 5 seconds
unsigned long lastDataUpdate = 0;
const unsigned long dataUpdateInterval = 5000; // 5 seconds


// /data endpoint returns tire data in JSON format
void handleData() {
  StaticJsonDocument<256> doc;
  doc["ip"] = WiFi.localIP().toString();

  JsonObject tireData = doc.createNestedObject("tireData");

  JsonObject FL = tireData.createNestedObject("FL");
  FL["pressure"] = pressureFL;
  FL["temperature"] = temperatureFL;

  JsonObject FR = tireData.createNestedObject("FR");
  FR["pressure"] = pressureFR;
  FR["temperature"] = temperatureFR;

  JsonObject RL = tireData.createNestedObject("RL");
  RL["pressure"] = pressureRL;
  RL["temperature"] = temperatureRL;

  JsonObject RR = tireData.createNestedObject("RR");
  RR["pressure"] = pressureRR;
  RR["temperature"] = temperatureRR;

  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
}

// /command endpoint to receive commands from the React Native app
void handleCommand() {
  if (server.hasArg("plain") == false) {
    server.send(400, "text/plain", "No command received");
    return;
  }
  String command = server.arg("plain");
  Serial.print("Received command: ");
  Serial.println(command);
  server.send(200, "text/plain", "Command received");
}

// Update the tire data with random values
void updateRandomData() {
  // For example: pressure between 30.0 and 35.0 PSI, temperature between 20.0 and 30.0 °C
  pressureFL = random(300, 350) / 10.0;
  temperatureFL = random(200, 300) / 10.0;

  pressureFR = random(300, 350) / 10.0;
  temperatureFR = random(200, 300) / 10.0;

  pressureRL = random(300, 350) / 10.0;
  temperatureRL = random(200, 300) / 10.0;

  pressureRR = random(300, 350) / 10.0;
  temperatureRR = random(200, 300) / 10.0;
  
  Serial.println("Updated tire data:");
  Serial.print("FL: Pressure = "); Serial.print(pressureFL);
  Serial.print(" PSI, Temp = "); Serial.print(temperatureFL); Serial.println(" °C");

  Serial.print("FR: Pressure = "); Serial.print(pressureFR);
  Serial.print(" PSI, Temp = "); Serial.print(temperatureFR); Serial.println(" °C");

  Serial.print("RL: Pressure = "); Serial.print(pressureRL);
  Serial.print(" PSI, Temp = "); Serial.print(temperatureRL); Serial.println(" °C");

  Serial.print("RR: Pressure = "); Serial.print(pressureRR);
  Serial.print(" PSI, Temp = "); Serial.print(temperatureRR); Serial.println(" °C");
}

void setup() {
  Serial.begin(74880);
  delay(2000);
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected! IP: ");
  Serial.println(WiFi.localIP());

  // Optionally start mDNS so you can reach it via esp32.local
  // if (MDNS.begin("esp32")) {
  //   Serial.println("mDNS responder started: esp32.local");
  // } else {
  //   Serial.println("Error setting up mDNS responder!");
  // }

  // Register HTTP endpoints
  // You can comment out handleRoot if you prefer not to use it

  server.on("/data", handleData);
  server.on("/command", HTTP_POST, handleCommand);
  
  server.begin();
  Serial.println("HTTP server started");

  // Seed random number generator and initialize tire data immediately
  randomSeed(analogRead(0));
  updateRandomData();
  lastDataUpdate = millis();
}

void loop() {
  server.handleClient();
  
  // Update tire data every 5 seconds
  if (millis() - lastDataUpdate >= dataUpdateInterval) {
    lastDataUpdate = millis();
    updateRandomData();
  }
}
