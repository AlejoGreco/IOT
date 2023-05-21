/****************************************
 * Include Libraries
 ****************************************/
#include "UbidotsESPMQTT.h"
#include "DHT.h"

/****************************************
 * Define Constants
 ****************************************/
#define TOKEN "BBFF-XaaB7DRD1mSrFoLq9b3dJzDRBony9o"     // Your Ubidots TOKEN
#define WIFINAME "Telecentro-a298"  // Your SSID
#define WIFIPASS "NWZLZWNZCTZ5"  // Your Wifi Pass

// Pins
#define ON_BOARD_LED 2

#define DHTPIN 32
#define DHTTYPE DHT11

// Global variables
Ubidots client(TOKEN);
DHT dht(DHTPIN, DHTTYPE);
unsigned long prevTime;

/****************************************
 * Auxiliar Functions
 ****************************************/

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if((char) payload[0] == '1')
  {
    digitalWrite(ON_BOARD_LED, HIGH);
  }
  else
  {
    digitalWrite(ON_BOARD_LED, LOW);
  }
}

/****************************************
 * Main Functions
 ****************************************/

void setup() {
  Serial.begin(115200);
  
  pinMode(ON_BOARD_LED, OUTPUT);
  dht.begin();

  prevTime = 0;
  
  //client.ubidotsSetBroker("industrial.api.ubidots.com");
  client.setDebug(true);  // Pass a true or false bool value to activate debug messages
  client.wifiConnection(WIFINAME, WIFIPASS);
  client.begin(callback);
  client.ubidotsSubscribe("hibernate001", "light");  // Insert the dataSource and Variable's Labels
}

void loop() {
  // put your main code here, to run repeatedly:
  if (!client.connected()) {
    client.reconnect();
    client.ubidotsSubscribe("hibernate001", "light");  // Insert the dataSource and Variable's Labels
  }

  client.loop();

  if((millis() - prevTime) > 20000)
  {
    float temperature = dht.readTemperature();
    float humedity = dht.readHumidity();

    client.add("humedity", humedity);
    client.add("temperature", temperature);
    client.ubidotsPublish("hibernate001");

    Serial.print("Temperatura: ");
    Serial.println(temperature);
    Serial.print("Humedad: ");
    Serial.println(humedity);

    prevTime = millis();
  }

  delay(100);
}