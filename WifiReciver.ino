#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// =========================
// Wi-Fi Credentials
// =========================
const char* ssid = "Jonah";
const char* password = "12345678";

// =========================
// Web Server
// =========================
WebServer server(80);  // HTTP server on port 80

// =========================
// LCD
// =========================
LiquidCrystal_I2C lcd(0x27, 16, 2);

// =========================
// Variables to store sensor data
// =========================
float temp1 = 0, temp2 = 0;
int ldr1 = 0, ldr2 = 0;
int pir1 = 0, pir2 = 0;
int fanState = 0; // 0 = OFF, 1 = ON

// =========================
// Handle HTTP GET Request
// Example URL: 
// http://RECEIVER_IP/update?temp1=23&temp2=24&ldr1=500&ldr2=450&pir1=1&pir2=0&fan=1
// =========================
void handleUpdate() {
  if (server.hasArg("temp1")) temp1 = server.arg("temp1").toFloat();
  if (server.hasArg("temp2")) temp2 = server.arg("temp2").toFloat();
  if (server.hasArg("ldr1"))  ldr1 = server.arg("ldr1").toInt();
  if (server.hasArg("ldr2"))  ldr2 = server.arg("ldr2").toInt();
  if (server.hasArg("pir1"))  pir1 = server.arg("pir1").toInt();
  if (server.hasArg("pir2"))  pir2 = server.arg("pir2").toInt();
  if (server.hasArg("fan"))   fanState = server.arg("fan").toInt();

  // Print to Serial Monitor
  Serial.println("=== Sensor Data Received ===");
  Serial.printf("Temp1: %.1f | Temp2: %.1f\n", temp1, temp2);
  Serial.printf("LDR1: %d | LDR2: %d\n", ldr1, ldr2);
  Serial.printf("PIR1: %d | PIR2: %d\n", pir1, pir2);
  Serial.printf("Fan: %s\n", fanState ? "ON" : "OFF");

  // Display on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T1:"); lcd.print(temp1,1);
  lcd.print(" T2:"); lcd.print(temp2,1);
  lcd.setCursor(0, 1);
  lcd.print("L1:"); lcd.print(ldr1);
  lcd.print(" L2:"); lcd.print(ldr2);
  delay(2000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PIR1:"); lcd.print(pir1);
  lcd.setCursor(0, 1);
  lcd.print("PIR2:"); lcd.print(pir2);
  delay(2000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Fan: ");
  lcd.print(fanState ? "ON " : "OFF");
  lcd.setCursor(0, 1);
  lcd.print("Smart Classroom");
  delay(2000);

  // Send acknowledgement back to sender
  server.send(200, "text/plain", "ACK");
}

void setup() {
  Serial.begin(115200);

  // Initialize LCD
  Wire.begin(21, 22); // SDA=21, SCL=22 for ESP32
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Smart Classroom");
  lcd.setCursor(0,1);
  lcd.print("Receiver Init...");
  delay(1500);
  lcd.clear();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Wi-Fi Connected!");
  Serial.print("📡 Receiver IP: ");
  Serial.println(WiFi.localIP());

  // Start Web Server
  server.on("/update", handleUpdate);
  server.begin();
  Serial.println("🌐 HTTP Server started, waiting for data...");
}

void loop() {
  server.handleClient(); // Handle incoming HTTP requests
}
