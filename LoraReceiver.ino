#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HardwareSerial.h>

// ============================
// LCD & LoRa Configuration
// ============================
LiquidCrystal_I2C lcd(0x27, 20, 4);   // Adjust I2C address if needed (0x27 or 0x3F)
HardwareSerial LoRaSerial(2);         // Use UART2

#define LORA_RXD 28  // ESP32 RX (connect to LoRa TX)
#define LORA_TXD 25   // ESP32 TX (connect to LoRa RX)

String receivedData = "";
unsigned long lastReceiveTime = 0;

// ============================
// Setup
// ============================
void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("🌡️ LoRa Receiver");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");

  // Initialize UART2 for LoRa
  LoRaSerial.begin(9600, SERIAL_8N1, LORA_RXD, LORA_TXD);
  delay(1000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("📡 Ready to receive");
  Serial.println("✅ Receiver ready! Waiting for data...");
}

// ============================
// Main Loop
// ============================
void loop() {
  // Check if LoRa has incoming data
  if (LoRaSerial.available()) {
    receivedData = LoRaSerial.readStringUntil('\n'); // Read full message
    receivedData.trim();

    if (receivedData.length() > 0) {
      lastReceiveTime = millis();

      // Print to Serial Monitor
      Serial.println("✅ Data received!");
      Serial.println(receivedData);
      Serial.println("----------------------------------");

      // Show confirmation on LCD
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("✅ Data Received!");
      delay(800);

      // Now display parsed information on LCD
      lcd.clear();

      // Split message into parts
      // Example: "Temp1:23.5C, Temp2:24.1C | PIR1:1, PIR2:0 | LDR1:2048, LDR2:1880 | Fan:ON"
      lcd.setCursor(0, 0);
      lcd.print(receivedData.substring(0, 20)); // line 1

      if (receivedData.length() > 20) {
        lcd.setCursor(0, 1);
        lcd.print(receivedData.substring(20, 40)); // line 2
      }

      if (receivedData.length() > 40) {
        lcd.setCursor(0, 2);
        lcd.print(receivedData.substring(40, 60)); // line 3
      }

      if (receivedData.length() > 60) {
        lcd.setCursor(0, 3);
        lcd.print(receivedData.substring(60, 80)); // line 4
      }
    }
  }

  // If no data received for 5 seconds, show waiting message
  if (millis() - lastReceiveTime > 5000) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" GATEWAY");
    lcd.setCursor(0, 1);
    lcd.print("LOADING....");

    Serial.println("Waiting for LoRa data...");
    delay(2000);
  }
}
