/**
   ESP32
*/
#include <WiFi.h>
#include <PubSubClient.h>
#include "DHTesp.h"
#include <ESP32Servo.h>

// WiFi
const char* ssid ="Wokwi-GUEST";
const char* password = "";

// MQTT
const char* mqtt_server = "broker.emqx.io";

const char* SUBTOPIC_LED = "ESP32/LED";
const char* SUBTOPIC_DOOR = "ESP32/DOOR";
const char* SUBTOPIC_TEMP = "ESP32/Temperature";
const char* SUBTOPIC_HUMIDITY = "ESP32/Humidity";
const char* SUBTOPIC_PIR = "ESP32/PIR";

WiFiClient espClient;
PubSubClient client(espClient);
//pir
#define pirPin 25
int statusPir = LOW;
int gerakanPir;
// LED
const int LED_PIN = 13;

// DHT
const int DHT_PIN = 15;
DHTesp dhtSensor;

// Servo
Servo servo;  // create servo object to control a servo

int SERVO_PIN = 2;  // analog pin used to connect the potentiometer

void setup_wifi() {
  delay(10);
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    String clientId = "mqttx_6535c7a8";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("Connected");
      client.subscribe(SUBTOPIC_LED);
      client.subscribe(SUBTOPIC_DOOR);
    } else {
      delay(5000);
    }
  }
}

void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Receive Topic: ");
  Serial.println(topic);

  Serial.print("Payload: ");
  Serial.println((char *)payload);
  if (!strcmp(topic, SUBTOPIC_LED)) {
    if (!strncmp((char *)payload, "on" , length)) {
      digitalWrite(LED_PIN, HIGH);
    } else if (!strncmp((char *)payload, "off" , length)) {
      digitalWrite(LED_PIN, LOW);
    }
  } else if (!strcmp(topic, SUBTOPIC_DOOR)) {
    if (!strncmp((char *)payload, "on" , length)) {
      Serial.println("SERVO TERBUKA");
      servo.write(90);
    } else if (!strncmp((char *)payload, "off" , length)) {
      Serial.println("SERVO TERTUTUP");
      servo.write(0);
    }
  }
}

void setup() {
  Serial.begin(115200);
  randomSeed(micros());

  pinMode(LED_PIN, OUTPUT);
  pinMode(pirPin, INPUT);

  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);

  servo.attach(SERVO_PIN, 500, 2400);  // attaches the servo on pin 13 to the servo object
  
  servo.write(0);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}
float old_temperature = 0;
float old_humidity = 0;

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
    gerakanPir = digitalRead(pirPin); //membaca sinyal dari pir ketika ada gerakan
  if(gerakanPir==HIGH){ //mengecek kondisi pir, jika mendeteksi gerakan maka skrip di bawah if akan dieksekusi secara berulang-ulang saat pir tetap mendeteksi gerakan
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    delay(500);
     //menyalakan led ketika terdapat gerakan
    if(statusPir==LOW){ //mengecek status dari pir apakah low atau high, jika low skrip di bawah if akan dieksekusi sekali ketika pir mendeteksi gerakan di awal
      client.publish(SUBTOPIC_PIR, ("Ada Object")); //menampilkan apk mqttx
      servo.write(90);
      Serial.println("Ada Object!"); //menampilkan peringatan ke serial monitor bahwa gerakan terdeteksi
      statusPir=HIGH; //mengubah status pir dari low ke high
    }
  }
  else { //jika pir tidak mendeteksi gerakan skrip akan dieksekusi
    digitalWrite(LED_PIN, LOW); //mematikan led karena tidak ada gerakan
    if(statusPir==HIGH){ //mengecek status pir, jika high skrip di dalam if akan dieksekusi
      client.publish(SUBTOPIC_PIR, ("Tidak Ada Object")); //menampilkan mqttx
      servo.write(0);
      Serial.println("Tidak Ada Object!"); //menampilkan peringatan bahwa gerakan tidak terdeteksi
      statusPir=LOW; //mengubah status dari pir dari high ke low
    }
  }
  TempAndHumidity  data = dhtSensor.getTempAndHumidity();
  float temperature = data.temperature;
  float humidity = data.humidity;

  if (old_temperature != temperature) {
    Serial.println("Temp: " + String(temperature, 2) + "°C");
    client.publish(SUBTOPIC_TEMP, String(temperature, 2).c_str());
    free;
  }
  old_temperature = temperature;

  if (old_humidity != humidity) {
    Serial.println("Humidity: " + String(humidity, 1) + "%");
    client.publish(SUBTOPIC_HUMIDITY, String(humidity, 1).c_str());
    free;
  }
  old_humidity = humidity;
  
  // Serial.println("---");
  delay(1000);
}