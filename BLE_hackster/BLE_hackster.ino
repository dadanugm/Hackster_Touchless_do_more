/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updates by chegewara
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <WiFi.h>
#include <Arduino.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

//#define SERVICE_UUID        "34221389-d113-480a-b34f-a7aa3c5a7513"
//#define CHARACTERISTIC_UUID "672c6e72-b0b1-43f9-87ec-4be40f8af113"
#define SERVICE_UUID        "d218b592-3b8c-471f-bedb-ee5943dcb1b9"
#define CHARACTERISTIC_UUID "e0df84b6-0257-4feb-abda-6b87f54b7f06"


int scanTime = 2; //In seconds
BLEScan* pBLEScan;
void set_as_broadcaster(void);
void set_as_observer(void);
void rrsi_calculation(void);
void warning_alarm(void);
void open_door(void);

////// RSSI Calculation constants /////
float A = 55; 
float NLog = 0.72;
float Dist = 0.0; // distance
int8_t RSSI_Val = 0;
bool interrupt = false;
const char * ssid = "hackster";
const char * password = "hackster123";

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks 
{
    void onResult(BLEAdvertisedDevice advertisedDevice) 
    {
      Serial.println(advertisedDevice.toString().c_str());
      Serial.print("RSSI: ");
      RSSI_Val = advertisedDevice.getRSSI();
      Serial.println(RSSI_Val);
      Serial.print("TX Power: ");
      Serial.println(advertisedDevice.getTXPower());
      // doing detection
      rrsi_calculation();
    }
};

void IRAM_ATTR isr() 
{
    interrupt = true;
}

void setup() 
{
  pinMode(4, OUTPUT);
  pinMode(0, INPUT_PULLUP);
  digitalWrite(4, LOW);
  attachInterrupt(0, isr, FALLING);
  Serial.begin(115200);
  Serial.println("Starting BLE work!");
}

void loop() 
{
  // put your main code here, to run repeatedly:
  set_as_broadcaster();
  set_as_observer();
  if (interrupt){open_door();} // open the door
}

void set_as_broadcaster(void)
{
  BLEDevice::init("ble broadcaster");
  //BLEDevice::setPower(ESP_PWR_LVL_P4);
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setValue("This is Broadcaster");
  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("BLE Broadcaster RUN");
  delay(10000);
  BLEDevice::deinit(0);
}

void set_as_observer(void)
{
    BLEDevice::init("");
    //BLEDevice::setPower(ESP_PWR_LVL_P4);
    pBLEScan = BLEDevice::getScan(); //create new scan
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);  // less or equal setInterval value
    for (int i=0;i<10;i++)
    {
      pBLEScan->start(scanTime, false);
      pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
      delay(1000);
    }
    BLEDevice::deinit(0);
}

void rrsi_calculation(void)
{
  // formula. RSSI = (-1)*(10*Nlog10*Dist+A)
  // for distance then, Dist = RSSI-A/(10*Nlog10)
  Dist = abs(abs((float)RSSI_Val)-A)/7.2;
  Serial.println(Dist);
  if (Dist<3.0)
  {
    // doing alarm
    digitalWrite(4, HIGH);   
    delay(300);                        
    digitalWrite(4, LOW);    
    Serial.println("your distance is : ");
    Serial.println(Dist);
    Serial.println("Move to safe distance!");
  }
}

void open_door(void)
{
  // when interrupt happens, -> open the door
  // steps: 1. wake the wifi up,connect to ssid 
  // 2. connect to udp server
  // when everythings ok, then open the door
  // to avoid crashing, deinit ble first
  interrupt = false;
  BLEDevice::deinit(0);
  btStop();
  delay(1000);
  Serial.println("Hoodooor");
  WiFi.begin(ssid, password);
  delay(1000);
  while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.println("WiFi Failed to connect");
    Serial.println("Check yout SSID and Password");
    delay(2000);
  }
  delay(5000);
  // turn off wifi
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);

  delay(1000);
  btStart();
  set_as_observer();
}
