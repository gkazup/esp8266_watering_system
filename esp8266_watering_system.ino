#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ThingerWifi.h>
#include <MCP3008.h>

// thinger.io config
#define USER_ID "username"
#define DEVICE_ID "watering_system"
#define DEVICE_CREDENTIAL "device cred"

// wifi config
#define WIFI_SSID "your_wifi_ssid"
#define WIFI_WPA2_PASSWORD "your_wifi_password"

// SDI pin setup
#define CLOCK_PIN 14
#define MISO_PIN 12
#define MOSI_PIN 13
#define CS_PIN 15

// motor pin setup
#define PUMP0_PIN D1
#define PUMP1_PIN D2
#define PUMP2_PIN D3
#define PUMP3_PIN D4


MCP3008 adc(CLOCK_PIN, MOSI_PIN, MISO_PIN, CS_PIN); // configure MCP3008 constructor
ThingerWifi thing(USER_ID, DEVICE_ID, DEVICE_CREDENTIAL); //configure Thinger constructor


void setup() {
  // setup pump states
  pinMode(PUMP0_PIN, OUTPUT);
    digitalWrite(PUMP0_PIN, LOW);
  pinMode(PUMP1_PIN, OUTPUT);
    digitalWrite(PUMP1_PIN, LOW);
  pinMode(PUMP2_PIN, OUTPUT);
    digitalWrite(PUMP2_PIN, LOW);
  pinMode(PUMP3_PIN, OUTPUT);
    digitalWrite(PUMP3_PIN, LOW);
  
  // add wifi connection
  thing.add_wifi(WIFI_SSID, WIFI_WPA2_PASSWORD);

  thing["pump0"] << [](pson& in){
    if(in.is_empty()){
        in = (bool) digitalRead(PUMP0_PIN);
    }
    else{
        digitalWrite(PUMP0_PIN, in ? HIGH : LOW);
    }
  }
  thing["pump1"] << [](pson& in){
    if(in.is_empty()){
        in = (bool) digitalRead(PUMP1_PIN);
    }
    else{
        digitalWrite(PUMP1_PIN, in ? HIGH : LOW);
    }
  }
  thing["pump2"] << [](pson& in){
    if(in.is_empty()){
        in = (bool) digitalRead(PUMP2_PIN);
    }
    else{
        digitalWrite(PUMP2_PIN, in ? HIGH : LOW);
    }
  }
  thing["pump3"] << [](pson& in){
    if(in.is_empty()){
        in = (bool) digitalRead(PUMP3_PIN);
    }
    else{
        digitalWrite(PUMP3_PIN, in ? HIGH : LOW);
    }
  }

}

void loop() {
  thing.handle();
}
