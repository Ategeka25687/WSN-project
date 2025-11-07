#include <HardwareSerial.h>

// ============================
// LoRa UART Configuration
// ============================
HardwareSerial LoRaSerial(2);  // Use UART2

#define LORA_RXD 28 // ESP32 RX (connect to LoRa TX)
#define LORA_TXD 25  // ESP32 TX (connect to LoRa RX)

// ============================
// Sensor & Actuator Pins
// ============================
#define LM35_1_PIN 34
#define LM35_2_PIN 35
#define PIR_1_PIN 4
#define PIR_2_PIN 18
#define LDR_1_PIN 36
#define LDR_2_PIN 39
#define FAN_PIN 27     // Fan control pin

// ============================
// Setup
// ============================
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("🌡️ Starting LoRa UART Test with LM35 + PIR + LDR + Fan control...");

  // Initialize UART2 for LoRa
  LoRaSerial.begin(9600, SERIAL_8N1, LORA_RXD, LORA_TXD);
  delay(500);
  Serial.println("✅ LoRa UART initialized.");

  // Configure pins
  pinMode(LM35_1_PIN, INPUT);
  pinMode(LM35_2_PIN, INPUT);
  pinMode(PIR_1_PIN, INPUT);
  pinMode(PIR_2_PIN, INPUT);
  pinMode(LDR_1_PIN, INPUT);
  pinMode(LDR_2_PIN, INPUT);
  pinMode(FAN_PIN, OUTPUT);

  digitalWrite(FAN_PIN, LOW);  // Start with fan OFF
  Serial.println("✅ All sensor pins and fan initialized.\n");
}

// ============================
// Function to Read LM35 Temperature
// ============================
float readTemperature(uint8_t pin) {
  int adcValue = analogRead(pin);
  float voltage = (adcValue * 3.3) / 4095.0;  // Convert ADC to voltage
  float temperature = voltage * 100.0;        // 10mV per °C → multiply by 100
  return temperature;
}

// ============================
// Main Loop
// ============================
void loop() {
  Serial.println("📡 Reading sensors...");

  // Read LM35 sensors
  float temp1 = readTemperature(LM35_1_PIN);
  float temp2 = readTemperature(LM35_2_PIN);

  // Read PIR sensors
  int pir1State = digitalRead(PIR_1_PIN);
  int pir2State = digitalRead(PIR_2_PIN);

  // Read LDR sensors
  int ldr1Value = analogRead(LDR_1_PIN);
  int ldr2Value = analogRead(LDR_2_PIN);

  // Control fan based on temperature
  String fanStatus;
  if (temp1 > 13.0) {
    digitalWrite(FAN_PIN, HIGH);
    fanStatus = "ON";
  } else {
    digitalWrite(FAN_PIN, LOW);
    fanStatus = "OFF";
  }

  // Prepare LoRa message
  String message = "Temp1:" + String(temp1, 1) + "C, Temp2:" + String(temp2, 1) +
                   "C | PIR1:" + String(pir1State) + ", PIR2:" + String(pir2State) +
                   " | LDR1:" + String(ldr1Value) + ", LDR2:" + String(ldr2Value) +
                   " | Fan:" + fanStatus;

  // ============================
  // Send message via LoRa
  // ============================
  Serial.println("🚀 Sending data to receiver via LoRa...");
  LoRaSerial.println(message);
  delay(50);  // small delay for transmission stability
  Serial.println("✅ Message sent to receiver!");
  Serial.println("-----------------------------------------------");

  // ============================
  // Debug info on Serial Monitor
  // ============================
  Serial.print("Temp1: "); Serial.print(temp1, 1);
  Serial.print("°C | Temp2: "); Serial.print(temp2, 1);
  Serial.print("°C | PIR1: "); Serial.print(pir1State);
  Serial.print(" | PIR2: "); Serial.print(pir2State);
  Serial.print(" | LDR1: "); Serial.print(ldr1Value);
  Serial.print(" | LDR2: "); Serial.print(ldr2Value);
  Serial.print(" | Fan: "); Serial.println(fanStatus);
  Serial.println("-----------------------------------------------\n");

  delay(2000);  // Send every 2 seconds
}
