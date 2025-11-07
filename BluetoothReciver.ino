#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEClient.h>
#include <BLERemoteService.h>
#include <BLERemoteCharacteristic.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ================== LCD SETUP ==================
LiquidCrystal_I2C lcd(0x27, 16, 2); // Adjust 0x27 if your LCD has different I2C address

// ================== BLE UUIDs (must match Node A) ==================
static BLEUUID serviceUUID("ABCD");
static BLEUUID temp1UUID("2A6E");
static BLEUUID temp2UUID("2A6F");
static BLEUUID ldr1UUID("2A76");
static BLEUUID ldr2UUID("2A77");
static BLEUUID pir1UUID("AB01");
static BLEUUID pir2UUID("AB02");
static BLEUUID fanUUID("2A59");

BLEAdvertisedDevice* advDevice;
bool deviceFound = false;

// ================== BLE Scan Callback ==================
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.haveServiceUUID() &&
        advertisedDevice.getServiceUUID().equals(serviceUUID)) {
      advDevice = new BLEAdvertisedDevice(advertisedDevice);
      deviceFound = true;
      advertisedDevice.getScan()->stop();
      Serial.print("✅ Found peripheral: ");
      Serial.println(advertisedDevice.getName().c_str());
    }
  }
};

// ================== SETUP ==================
void setup() {
  Serial.begin(115200);
  Serial.println("🔎 Node B — BLE Central Scanning...");

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Receiver Ready");
  delay(1500);
  lcd.clear();

  BLEDevice::init("");
}

// ================== MAIN LOOP ==================
void loop() {
  // ---- SCANNING PHASE ----
  if (!deviceFound) {
    BLEScan* scan = BLEDevice::getScan();
    scan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    scan->setActiveScan(true);
    scan->start(5, false); // scan for 5 seconds
    delay(1000);
    return;
  }

  // ---- CONNECTION PHASE ----
  BLEClient* client = BLEDevice::createClient();
  Serial.print("🔗 Connecting to ");
  Serial.println(advDevice->getName().c_str());

  if (!client->connect(advDevice)) {
    Serial.println("❌ Connection failed, rescanning...");
    deviceFound = false;
    delete advDevice;
    delay(2000);
    return;
  }
  Serial.println("✅ Connected successfully!");

  BLERemoteService* service = client->getService(serviceUUID);
  if (service == nullptr) {
    Serial.println("❌ Service not found on peripheral");
    client->disconnect();
    deviceFound = false;
    delete advDevice;
    delay(2000);
    return;
  }

  // Get characteristics
  BLERemoteCharacteristic* temp1C = service->getCharacteristic(temp1UUID);
  BLERemoteCharacteristic* temp2C = service->getCharacteristic(temp2UUID);
  BLERemoteCharacteristic* ldr1C  = service->getCharacteristic(ldr1UUID);
  BLERemoteCharacteristic* ldr2C  = service->getCharacteristic(ldr2UUID);
  BLERemoteCharacteristic* pir1C  = service->getCharacteristic(pir1UUID);
  BLERemoteCharacteristic* pir2C  = service->getCharacteristic(pir2UUID);
  BLERemoteCharacteristic* fanC   = service->getCharacteristic(fanUUID);

  if (!(temp1C && temp2C && ldr1C && ldr2C && pir1C && pir2C && fanC)) {
    Serial.println("❌ Missing characteristics!");
    client->disconnect();
    deviceFound = false;
    delete advDevice;
    delay(2000);
    return;
  }

  // ---- DATA READING LOOP ----
  while (client->isConnected()) {
    float temp1 = temp1C->readFloat();
    float temp2 = temp2C->readFloat();
    int ldr1 = ldr1C->readUInt16();
    int ldr2 = ldr2C->readUInt16();
    bool pir1 = (pir1C->readValue().c_str()[0] == 1);
    bool pir2 = (pir2C->readValue().c_str()[0] == 1);
    String fanVal = fanC->readValue();  // Use Arduino String

    // ---- SERIAL MONITOR OUTPUT ----
    Serial.println("----------------------------------------");
    Serial.print("🌡 Temp1: "); Serial.print(temp1, 1); Serial.print("°C | Temp2: "); Serial.println(temp2, 1);
    Serial.print("💡 LDR1: "); Serial.print(ldr1); Serial.print(" | LDR2: "); Serial.println(ldr2);
    Serial.print("🏃 PIR1: "); Serial.print(pir1 ? "MOTION" : "NO");
    Serial.print(" | PIR2: "); Serial.println(pir2 ? "MOTION" : "NO");
    Serial.print("💨 Fan: "); Serial.println(fanVal);

    // ---- DISPLAY ON LCD ----
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.print(temp1, 1);
    lcd.print("C F:");
    lcd.print(fanVal);

    lcd.setCursor(0, 1);
    lcd.print("L:");
    lcd.print(ldr1);
    lcd.print(" P:");
    lcd.print(pir1 ? "1" : "0");

    delay(3000);
  }

  Serial.println("🔁 Disconnected, restarting scan...");
  deviceFound = false;
  delete advDevice;
  delay(2000);
}
