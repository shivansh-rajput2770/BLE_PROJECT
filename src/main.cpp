/*
  Truck-07 · BLE basics (simple version)
  ESP32 = BLE server with 1 service and 2 characteristics:
    LED  -> phone writes "1" or "0"
    TEMP -> ESP32 sends temperature every 2 seconds (notify)
  Phone app: nRF Connect (Android + iPhone)
*/
#include <Arduino.h>
#include <BLEDevice.h>

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define LED_UUID     "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define TEMP_UUID    "beb5483e-36e1-4688-b7f5-ea07361b26a9"
#define LED_PIN      2

BLEServer *server;
BLECharacteristic *ledChar;
BLECharacteristic *tempChar;

String lastLed = "0";
bool wasConnected = false;
unsigned long lastSend = 0;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  // 1. Start BLE with a name
  BLEDevice::init("SHIVA BL-T09");
  server = BLEDevice::createServer();

  // 2. Make one service (folder) with two characteristics (files)
  BLEService *service = server->createService(SERVICE_UUID);

  ledChar = service->createCharacteristic(LED_UUID,
              BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  ledChar->setValue("0");

  tempChar = service->createCharacteristic(TEMP_UUID,
              BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  tempChar->setValue("0.0");

  service->start();

  // 3. Start advertising ("I'm here!")
  BLEDevice::getAdvertising()->addServiceUUID(SERVICE_UUID);
  BLEDevice::startAdvertising();

  Serial.println("BLE ready. Scan for \"Truck07-BLE\" in nRF Connect");
}

void loop() {
  // 4. Phone connected or disconnected?
  bool connected = server->getConnectedCount() > 0;
  if (connected && !wasConnected) {
    Serial.println("Phone connected");
  }
  if (!connected && wasConnected) {
    Serial.println("Phone disconnected -> advertising again");
    BLEDevice::startAdvertising();
  }
  wasConnected = connected;

  // 5. Did the phone write a new LED value?
  String led = String(ledChar->getValue().c_str());
  if (led != lastLed) {
    lastLed = led;
    Serial.println("Phone wrote LED = \"" + led + "\"");
    if (led == "1") digitalWrite(LED_PIN, HIGH);
    if (led == "0") digitalWrite(LED_PIN, LOW);
  }

  // 6. Every 2 seconds: send engine temperature (fake: 88.0 - 95.9)
  if (millis() - lastSend >= 2000) {
    lastSend = millis();
    String temp = String(random(880, 960) / 10.0, 1);
    tempChar->setValue(std::string(temp.c_str()));
    if (connected) {
      tempChar->notify();
      Serial.println("Notify TEMP = " + temp);
    }
  }

  delay(20);
}