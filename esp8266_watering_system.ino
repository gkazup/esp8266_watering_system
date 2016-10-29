#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ThingerWifi.h>
#include <MCP3008.h>

// thinger.io config
#define USER_ID "username"
#define DEVICE_ID "device id"
#define DEVICE_CREDENTIAL "device cred"

// wifi config
#define WIFI_SSID "your_wifi_ssid"
#define WIFI_WPA2_PASSWORD "your_wifi_password"

// SDI pin setup to connect MCP3008
#define CLOCK_PIN 14
#define MISO_PIN 12
#define MOSI_PIN 13
#define CS_PIN 15

// motor pin setup
#define PUMP0_PIN D1
#define PUMP1_PIN D2
#define PUMP2_PIN D3
#define PUMP3_PIN D4

// measurement pin setup
#define MEASUREMENT_PIN D5


MCP3008 adc(CLOCK_PIN, MOSI_PIN, MISO_PIN, CS_PIN); // configure MCP3008 constructor
ThingerWifi thing(USER_ID, DEVICE_ID, DEVICE_CREDENTIAL); //configure Thinger constructor


int wateringLowThreshold = 500;     // the threshold to start the pump
int wateringHighThreshold = 900;    // the threshold to stop the pump

// keep a history of the sensor values to calculate average
// average value goes into [0] - this will be used to control the pumps
// actual value goes into [1]
int sensor0History[] = {1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023};
int sensor1History[] = {1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023};
int sensor2History[] = {1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023};
int sensor3History[] = {1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023};
int sensorHistoryCount = 10;

unsigned long previousMillis = 0;   // last time sensor data was taken
unsigned long currentMillis = 0;    // current time
unsigned long delayTime = 10000;    // 10 sec delay between sensor data update


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

  // setup measurement state
  pinMode(MEASUREMENT_PIN, OUTPUT);
    digitalWrite(MEASUREMENT_PIN, LOW);
  
  // add wifi connection
  thing.add_wifi(WIFI_SSID, WIFI_WPA2_PASSWORD);

  // setup for the delay thingy
  thing["delayTime"] << [](pson& in){
    if(in.is_empty()) in = delayTime;
    else delayTime = in;
  };

  // setup for the threshold thingies
  thing["wateringHighThreshold"] << [](pson& in){
    if(in.is_empty()) in = wateringHighThreshold;
    else wateringHighThreshold = in;
  };
  thing["wateringLowThreshold"] << [](pson& in){
    if(in.is_empty()) in = wateringLowThreshold;
    else wateringLowThreshold = in;
  };

  // setup for the pump thingies
  thing["pump0"] << [](pson& in){
    if(in.is_empty()) in = (bool) digitalRead(PUMP0_PIN);
    else digitalWrite(PUMP0_PIN, in ? HIGH : LOW);
  };
  thing["pump1"] << [](pson& in){
    if(in.is_empty()) in = (bool) digitalRead(PUMP1_PIN);
    else digitalWrite(PUMP1_PIN, in ? HIGH : LOW);
  };
  thing["pump2"] << [](pson& in){
    if(in.is_empty()) in = (bool) digitalRead(PUMP2_PIN);
    else digitalWrite(PUMP2_PIN, in ? HIGH : LOW);
  };
  thing["pump3"] << [](pson& in){
    if(in.is_empty()) in = (bool) digitalRead(PUMP3_PIN);
    else digitalWrite(PUMP3_PIN, in ? HIGH : LOW);
  };

  // setup for the sensor thingies
  thing["sensor0"] >> [](pson& out){
    out = sensor0History[0];
  };
  thing["sensor1"] >> [](pson& out){
    out = sensor1History[0];
  };
  thing["sensor2"] >> [](pson& out){
    out = sensor2History[0];
  };
  thing["sensor3"] >> [](pson& out){
    out = sensor3History[0];
  };

}

void loop() {
  thing.handle();

  currentMillis = millis();
  if (currentMillis - previousMillis > delayTime) {
    updateSensorHistory();
    previousMillis = millis();
    handlePumps();
  }
}

void updateSensorHistory() {
  int i;
  
  // enable the sensor current
  digitalWrite(MEASUREMENT_PIN, HIGH);

  // circle the buffer
  for (i = (sensorHistoryCount-1); i > 1; i--) {
    sensor0History[i] = sensor0History[i-1];
    sensor1History[i] = sensor1History[i-1];
    sensor2History[i] = sensor2History[i-1];
    sensor3History[i] = sensor3History[i-1];
  }

  // read the new values from the sensors
  sensor0History[1] = adc.readADC(0);
  sensor1History[1] = adc.readADC(1);
  sensor2History[1] = adc.readADC(2);
  sensor3History[1] = adc.readADC(3);

  // reset the average values
  sensor0History[0] = 0;
  sensor0History[0] = 0;
  sensor0History[0] = 0;
  sensor0History[0] = 0;

  // take the new average
  for (i = 1; i < sensorHistoryCount; i++) {
    sensor0History[0] = sensor0History[0] + sensor0History[i];
    sensor1History[0] = sensor1History[0] + sensor1History[i];
    sensor2History[0] = sensor2History[0] + sensor2History[i];
    sensor3History[0] = sensor3History[0] + sensor3History[i];
  }

  sensor0History[0] = round(sensor0History[0] / (sensorHistoryCount - 1));
  sensor1History[0] = round(sensor1History[0] / (sensorHistoryCount - 1));
  sensor2History[0] = round(sensor2History[0] / (sensorHistoryCount - 1));
  sensor3History[0] = round(sensor3History[0] / (sensorHistoryCount - 1));

  // disable the sensor current
  digitalWrite(MEASUREMENT_PIN, LOW);
}

void handlePumps(){
  // turn on the pump if we are lower than the Low treshold
  // turn off the pump if we are higher than the High treshold
  
  if (sensor0History[0] < wateringLowThreshold) digitalWrite(PUMP0_PIN, HIGH);
  if (wateringHighThreshold < sensor0History[0]) digitalWrite(PUMP0_PIN, LOW);
  
  if (sensor1History[0] < wateringLowThreshold) digitalWrite(PUMP1_PIN, HIGH);
  if (wateringHighThreshold < sensor1History[0]) digitalWrite(PUMP1_PIN, LOW);

  if (sensor2History[0] < wateringLowThreshold) digitalWrite(PUMP2_PIN, HIGH);
  if (wateringHighThreshold < sensor2History[0]) digitalWrite(PUMP2_PIN, LOW);

  if (sensor3History[0] < wateringLowThreshold) digitalWrite(PUMP3_PIN, HIGH);
  if (wateringHighThreshold < sensor3History[0]) digitalWrite(PUMP3_PIN, LOW);
}

