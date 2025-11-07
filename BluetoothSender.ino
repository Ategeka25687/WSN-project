#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// ===============================
// UUIDs must match Central code
// ===============================
#define SERVICE_UUID        "ABCD"
#define TEMP1_UUID          "2A6E"
#define TEMP2_UUID          "2A6F"
#define LDR1_UUID           "2A76"
#define LDR2_UUID           "2A77"
#define PIR1_UUID           "AB01"
#define PIR2_UUID           "AB02"
#define FAN_UUID            "2A59"

// ===============================
// Sensor Pins
// ===============================
#define TEMP1_PIN 34
#define TEMP2_PIN 35
#define LDR1_PIN 32
#define LDR2_PIN 33
#define PIR1_PIN 25
#define PIR2_PIN 26
#define FAN_PIN 27

// ===============================
// BLE Characteristic Pointers
// ===============================
BLECharacteristic *temp1C, *temp2C, *ldr1C, *ldr2C, *pir1C, *pir2C, *fanC;

int fanState = 0;

// ===============================
// Setup BLE
// ===============================
void setup() {
  Serial.begin(115200);
  Serial.println("🚀 Node A — BLE Peripheral (Sensor Node)");

  // Create BLE device
  BLEDevice::init("SmartClassNode");

  // Create BLE server
  BLEServer *pServer = BLEDevice::createServer();

  // Create service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create characteristics
  temp1C = pService->createCharacteristic(TEMP1_UUID, BLECharacteristic::PROPERTY_READ);
  temp2C = pService->createCharacteristic(TEMP2_UUID, BLECharacteristic::PROPERTY_READ);
  ldr1C  = pService->createCharacteristic(LDR1_UUID,  BLECharacteristic::PROPERTY_READ);
  ldr2C  = pService->createCharacteristic(LDR2_UUID,  BLECharacteristic::PROPERTY_READ);
  pir1C  = pService->createCharacteristic(PIR1_UUID,  BLECharacteristic::PROPERTY_READ);
  pir2C  = pService->createCharacteristic(PIR2_UUID,  BLECharacteristic::PROPERTY_READ);
  fanC   = pService->createCharacteristic(FAN_UUID,   BLECharacteristic::PROPERTY_READ);

  // Start service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  BLEDevice::startAdvertising();

  Serial.println("📡 Advertising SmartClass service...");
  
  pinMode(PIR1_PIN, INPUT);
  pinMode(PIR2_PIN, INPUT);
  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, LOW);
}

// ===============================
// Loop — Update and publish data
// ===============================
void loop() {
  // Read sensors
  float temp1 = analogRead(TEMP1_PIN) * (3.3 / 4095.0) * 100.0;
  float temp2 = analogRead(TEMP2_PIN) * (3.3 / 4095.0) * 100.0;
  int ldr1 = analogRead(LDR1_PIN);
  int ldr2 = analogRead(LDR2_PIN);
  int pir1 = digitalRead(PIR1_PIN);
  int pir2 = digitalRead(PIR2_PIN);

  // Fan control logic
  if (temp1 > 20 || temp2 > 20) {
    digitalWrite(FAN_PIN, HIGH);
    fanState = 1;
  } else {
    digitalWrite(FAN_PIN, LOW);
    fanState = 0;
  }

  // Update BLE characteristics
  temp1C->setValue(temp1);
  temp2C->setValue(temp2);
  ldr1C->setValue((uint16_t)ldr1);
  ldr2C->setValue((uint16_t)ldr2);
  pir1C->setValue((uint8_t)pir1);
  pir2C->setValue((uint8_t)pir2);
  fanC->setValue(String(fanState).c_str());

  Serial.println("📤 Sending BLE data...");
  Serial.print("T1: "); Serial.print(temp1);
  Serial.print(" | T2: "); Serial.print(temp2);
  Serial.print(" | Fan: "); Serial.println(fanState);

  delay(3000);
}
