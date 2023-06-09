/****************************************
 * Include Libraries
 ****************************************/
#include "UbidotsESPMQTT.h"
#include "DHT.h"
#include <stdio.h>
#include <string.h>

/****************************************
 * Define Constants
 ****************************************/
#define TOKEN "BBFF-XaaB7DRD1mSrFoLq9b3dJzDRBony9o"     // Your Ubidots TOKEN
#define WIFINAME "Telecentro-a298"  // Your SSID
#define WIFIPASS "NWZLZWNZCTZ5"  // Your Wifi Pass

// Topics define
#define DEVICE_LABEL "lowpower001"
#define TOP_TEMP "temperature"
#define TOP_HUM "humidity"
#define TOP_SLEEP "sleepTime"
#define TOP_LIGHT "light"

#define uS_TO_M_FACTOR 1000000 * 60       /* Conversion factor for micro seconds to minutes */
#define mS_TO_M_FACTOR 1000 * 60       /* Conversion factor for mili seconds to minutes */

// Pins
#define ON_BOARD_LED 2

#define DHTPIN 32
#define DHTTYPE DHT11

// Global variables
Ubidots client(TOKEN);
DHT dht(DHTPIN, DHTTYPE);
unsigned long prevTime;
unsigned long time_to_sleep; 
unsigned char meditions, currentMeditions;

RTC_DATA_ATTR int bootCount = 0;

/****************************************
 * Auxiliar Functions
 ****************************************/
/*
Method to print the reason by which ESP32
has been awaken from sleep
*/
void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if(strstr(topic, TOP_LIGHT) != NULL){
    if((char) payload[0] == '1')
    {
      digitalWrite(ON_BOARD_LED, HIGH);
    }
    else
    {
      digitalWrite(ON_BOARD_LED, LOW);
    }

    return;
  }

  if(strstr(topic, "sleeptime") != NULL){
    sscanf((char*)payload , "%d", &time_to_sleep);
    esp_sleep_enable_timer_wakeup(time_to_sleep * uS_TO_M_FACTOR);
    Serial.println("Setup ESP32 to sleep for every " + String(time_to_sleep) + " Minutes");
    return;
  }
  
}

/****************************************
 * Main Functions
 ****************************************/

void setup() {
  Serial.begin(115200);
  delay(100);
  
  pinMode(ON_BOARD_LED, OUTPUT);
  dht.begin();
  delay(100);

  prevTime = 0;
  time_to_sleep = 1; 
  meditions = 2;
  currentMeditions = 0;

  //Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  //Print the wakeup reason for ESP32
  print_wakeup_reason();
  
  //client.ubidotsSetBroker("industrial.api.ubidots.com");
  client.setDebug(true);  // Pass a true or false bool value to activate debug messages
  client.wifiConnection(WIFINAME, WIFIPASS);
  client.begin(callback);
  client.ubidotsSubscribe(DEVICE_LABEL, TOP_LIGHT);  // Insert the dataSource and Variable's Labels
  client.ubidotsSubscribe(DEVICE_LABEL, TOP_SLEEP);  // Insert the dataSource and Variable's Labels
}

void loop() {
  // put your main code here, to run repeatedly:
  if (!client.connected()) {
    client.reconnect();
    client.ubidotsSubscribe(DEVICE_LABEL, TOP_LIGHT);  // Insert the dataSource and Variable's Labels
    client.ubidotsSubscribe(DEVICE_LABEL, TOP_SLEEP);  // Insert the dataSource and Variable's Labels
  }

  client.loop();

  if((millis() - prevTime) > time_to_sleep * mS_TO_M_FACTOR / meditions )
  {
    float temperature = dht.readTemperature();
    float humedity = dht.readHumidity();

    client.add(TOP_HUM, humedity);
    client.add(TOP_TEMP, temperature);
    client.ubidotsPublish(DEVICE_LABEL);

    Serial.print("Temperatura: ");
    Serial.println(temperature);
    Serial.print("Humedad: ");
    Serial.println(humedity);

    currentMeditions++;
    prevTime = millis();
  }

  if(currentMeditions == meditions){
    Serial.println("Going to sleep now");
    Serial.flush(); 
    esp_deep_sleep_start();
    Serial.println("This will never be printed");
  }

  delay(10);
}