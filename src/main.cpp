#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid     = "MERCUSYS";
const char* password = "17122001";

WebServer server(80);

// Tire data globals (start with your defaults)
float pressureFL    = 32.5;
float temperatureFL = 22.0;
float pressureFR    = 32.5;
float temperatureFR = 22.0;
float pressureRL    = 32.5;
float temperatureRL = 22.0;
float pressureRR    = 32.5;
float temperatureRR = 22.0;

// /data returns the current tire data
void handleData() {
  StaticJsonDocument<256> doc;
  doc["ip"] = WiFi.localIP().toString();
  JsonObject tireData = doc.createNestedObject("tireData");
  auto addWheel = [&](const char* id, float p, float t){
    JsonObject w = tireData.createNestedObject(id);
    w["pressure"]    = p;
    w["temperature"] = t;
  };
  addWheel("FL", pressureFL, temperatureFL);
  addWheel("FR", pressureFR, temperatureFR);
  addWheel("RL", pressureRL, temperatureRL);
  addWheel("RR", pressureRR, temperatureRR);

  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

// bump the named wheel’s pressure by +1 or –1
void updateData(const String& wheel, const String& command) {
  int delta = (command == "INCREASE") ? 1 : -1;
  if      (wheel == "FL") pressureFL += delta;
  else if (wheel == "FR") pressureFR += delta;
  else if (wheel == "RL") pressureRL += delta;
  else if (wheel == "RR") pressureRR += delta;
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

void handleCommand() {
  StaticJsonDocument<200> doc;
  auto err = deserializeJson(doc, server.arg("plain"));
  if (err) {
    server.send(400, "application/json", "{\"error\":\"bad json\"}");
    return;
  }
  String wheel   = doc["wheel"].as<String>();
  String command = doc["command"].as<String>();

  updateData(wheel, command);

  // Optionally, return the updated tire data or a status object:
  StaticJsonDocument<128> resp;
  resp["status"] = "ok";
  resp["wheel"]  = wheel;
  resp["pressure"] = (wheel=="FL"?pressureFL:
                      wheel=="FR"?pressureFR:
                      wheel=="RL"?pressureRL:
                                  pressureRR);
  String out;
  serializeJson(resp, out);
  server.send(200, "application/json", out);
}

void setup() {
  Serial.begin(74880);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected, IP = " + WiFi.localIP().toString());

  server.on("/data", HTTP_GET, handleData);
  server.on("/command", HTTP_POST, handleCommand);
  server.begin();
}

void loop() {
  server.handleClient();
  // No more random updates here!
}
