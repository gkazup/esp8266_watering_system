#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ThingerWifi.h>
#include <MCP3008.h>

// thinger.io config
#define USER_ID "gkazup"
#define DEVICE_ID "watering_system"
#define DEVICE_CREDENTIAL "A(+NlE<:=D#9&K0[u3u@tyK_i"
#define ALARM_ENDPOINT "alarm_endpoint"

// wifi config
#define WIFI_SSID "your_wifi_ssid"
#define WIFI_WPA2_PASSWORD "your_wifi_password"

// SDI pin setup
#define CLOCK_PIN 14
#define MISO_PIN 12
#define MOSI_PIN 13
#define CS_PIN 15

// motor pin setup
#define SENSOR_PIN D1
#define LED_PIN D2


MCP3008 adc(CLOCK_PIN, MOSI_PIN, MISO_PIN, CS_PIN); // configure MCP3008 constructor
ThingerWifi thing(USER_ID, DEVICE_ID, DEVICE_CREDENTIAL); //configure Thinger constructor


void setup() {
  // add wifi connection
  thing.add_wifi(WIFI_SSID, WIFI_WPA2_PASSWORD);

  digitalWrite(LED_PIN, LOW); // turn off led alarm

}

void loop() {
  thing.handle();
}
