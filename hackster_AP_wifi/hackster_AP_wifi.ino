#include <ESP8266WiFi.h>
#include <WiFiClient.h>

// SSID & PASS
const char *ssid = "hackster";
const char *password = "hackster123";
void client_scan(void);
// Register your device MAC here
char my_phone[18] = "E0:1F:88:66:4E:2E";
char my_device[18] = "BC:DD:C2:DF:8A:38";

void setup() 
{
  delay(1000);
  Serial.begin(115200);
  WiFi.softAP(ssid, password);
}

void loop() 
{
  // Do network scan
  client_scan();
  delay(2000);
}

void client_scan(void) 
{
  unsigned char number_client;
  struct station_info *stat_info;
  String result;
  char wifiClientMac[18];
  int i=1;
  int door = -1;
  
  number_client= wifi_softap_get_station_num();
  stat_info = wifi_softap_get_station_info();
  Serial.print(" Total Connected Clients are = ");
  Serial.println(number_client);
  while (stat_info != NULL) 
  {
    result += "Client ";
    result += String(i);
    result += " = ";
    result += IPAddress(stat_info->ip).toString();
    result += " - ";
    sprintf(wifiClientMac, "%02X:%02X:%02X:%02X:%02X:%02X", MAC2STR(stat_info->bssid));
    result += wifiClientMac;
    stat_info = STAILQ_NEXT(stat_info, next);
    i++;
    Serial.println(result);
    Serial.println();
    // do unlock by compare the MAC client 
    door = strcmp(wifiClientMac,my_phone)*strcmp(wifiClientMac,my_device);
    if (door == 0)
    {
      // Open the door
      Serial.println("the door is opened");
      // Then use GPIO to drive your Door Opener, drive a motor, ETC
    }
    else
    {
      // unidentified device
      Serial.println("Unregistered Device");
    }
    //Serial.printf("%i",door);
    wifi_softap_free_station_info();
  }
  delay(500);
}
