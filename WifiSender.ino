#include <WiFi.h>
#include <HTTPClient.h>

// =========================
// Wi-Fi credentials
// =========================
const char* ssid = "Jonah";
const char* password = "12345678";

// =========================
// Receiver IP
// =========================
const char* receiverIP = "192.168.137.240"; // Receiver ESP32 IP

// =========================
// Sensor and Actuator Pins
// =========================
#define TEMP1_PIN 34   // LM35 temperature sensor 1
#define TEMP2_PIN 35   // LM35 temperature sensor 2
#define LDR1_PIN 32
#define LDR2_PIN 33
#define PIR1_PIN 25
#define PIR2_PIN 26
#define FAN_PIN 27     // Fan (relay or transistor control pin)

// =========================
// Global variable
// =========================
int fanState = 0;  // 0 = OFF, 1 = ON

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Wi-Fi connected!");
  Serial.print("📡 Sender IP: ");
  Serial.println(WiFi.localIP());

  // Initialize pins
  pinMode(PIR1_PIN, INPUT);
  pinMode(PIR2_PIN, INPUT);
  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, LOW);  // Start with fan OFF
}

void loop() {
  // =========================
  // Read sensor values
  // =========================
  float temp1 = analogRead(TEMP1_PIN) * (3.3 / 4095.0) * 100.0; // LM35
  float temp2 = analogRead(TEMP2_PIN) * (3.3 / 4095.0) * 100.0;
  int ldr1 = analogRead(LDR1_PIN);
  int ldr2 = analogRead(LDR2_PIN);
  int pir1 = digitalRead(PIR1_PIN);
  int pir2 = digitalRead(PIR2_PIN);

  // =========================
  // Fan control logic
  // =========================
  if (temp1 > 20 || temp2 > 20) {
    digitalWrite(FAN_PIN, HIGH);  // Turn fan ON
    fanState = 1;
  } else {
    digitalWrite(FAN_PIN, LOW);   // Turn fan OFF
    fanState = 0;
  }

  // =========================
  // Send data over Wi-Fi
  // =========================
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Build the URL
    String url = String("http://") + receiverIP + "/update?" +
                 "temp1=" + temp1 +
                 "&temp2=" + temp2 +
                 "&ldr1=" + ldr1 +
                 "&ldr2=" + ldr2 +
                 "&pir1=" + pir1 +
                 "&pir2=" + pir2 +
                 "&fan=" + fanState;

    Serial.println("\n🌐 Sending data to receiver:");
    Serial.println(url);

    http.begin(url);
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("✅ Receiver response: " + response);
    } else {
      Serial.println("❌ Error sending data: " + String(httpResponseCode));
    }
    http.end();
  }

  delay(5000); // Send every 5 seconds
}
