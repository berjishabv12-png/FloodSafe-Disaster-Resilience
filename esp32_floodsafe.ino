#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Replace these values. Do not upload Wi-Fi credentials to a public repository.
const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* SERVER_URL = "http://192.168.1.10:5000/api/update"; // Replace with computer IPv4
const char* ROAD_ID = "ROAD_1";

const int TRIG_PIN = 5;
const int ECHO_PIN = 18;
// Measure sensor face to the empty container bottom/reference height.
const float SENSOR_HEIGHT_CM = 30.0;

float readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000UL);
  if (duration == 0) return -1.0;
  return (duration * 0.0343f) / 2.0f;
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  unsigned long started = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - started < 20000UL) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("ESP32 IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Wi-Fi failed; will retry.");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);
  connectWiFi();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  float distance = readDistanceCm();
  if (distance < 0) {
    Serial.println("No echo: check sensor alignment and range.");
    delay(3000);
    return;
  }

  float depth = SENSOR_HEIGHT_CM - distance;
  if (depth < 0) depth = 0;
  Serial.printf("Distance: %.2f cm | Estimated depth: %.2f cm\n", distance, depth);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.setTimeout(4000);
    http.begin(SERVER_URL);
    http.addHeader("Content-Type", "application/json");
    StaticJsonDocument<160> doc;
    doc["road_id"] = ROAD_ID;
    doc["distance_cm"] = distance;
    String body;
    serializeJson(doc, body);
    int code = http.POST(body);
    Serial.printf("HTTP response: %d\n", code);
    if (code > 0) Serial.println(http.getString());
    else Serial.println("Server unreachable: check IP, Wi-Fi and firewall.");
    http.end();
  }
  delay(3000);
}
