#include <SPI.h>
#include <ESP8266WiFi.h>
#include <MCP3008.h>

// wifi config
#define WIFI_SSID "your_ssid"
#define WIFI_WPA2_PASSWORD "your_pass"

// SDI pin setup to connect MCP3008
#define CLOCK_PIN D5
#define MISO_PIN D6
#define MOSI_PIN D7
#define CS_PIN D8

// motor pin setup
#define PUMP0_PIN D1
#define PUMP1_PIN D2
#define PUMP2_PIN D3
#define PUMP3_PIN D4

// measurement pin setup
#define MEASUREMENT_PIN D0


MCP3008 adc(CLOCK_PIN, MOSI_PIN, MISO_PIN, CS_PIN); // configure MCP3008 constructor
WiFiServer server(80);


int wateringLowThreshold = 500;     // the threshold to start the pump
int wateringHighThreshold = 900;    // the threshold to stop the pump

// keep a history of the sensor values for plotting in the future
// actual value goes into [0] - this will be used to control the pumps
int sensor0History[] = {1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023};
int sensor1History[] = {1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023};
int sensor2History[] = {1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023};
int sensor3History[] = {1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023};
int sensorHistoryCount = 10;

// pump status array
int pumps[] = {0,0,0,0};

unsigned long currentMillis = 0;    // current time
unsigned long sensorMillis = 0;     // last time sensor data was taken
unsigned long sensorDelay = 10000;  // 10 sec delay between sensor data update
unsigned long historyMillis = 0;        // last time history table was updated
unsigned long historyDelay = 1800000;   // 30 minutes between history update
unsigned long pumpMillis = 0;       // last time pump status was updated
unsigned long pumpDelay = 1000;     // 1 sec between pump status update


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

  Serial.begin(9600);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  
  WiFi.hostname("watering");
  WiFi.begin(WIFI_SSID, WIFI_WPA2_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");
 
  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");

}

void loop() {
  currentMillis = millis();

  handleWifi();

  if (handlePumps()) {
    if (currentMillis - pumpMillis > pumpDelay) {
      updateSensorData();
      pumpMillis = millis();
      debugOnSerial();
    }
  } else {
    if (currentMillis - sensorMillis > sensorDelay) {
      updateSensorData();
      sensorMillis = millis();
      debugOnSerial();
    }
  }

  if (currentMillis - historyMillis > historyDelay) {
    updateSensorHistory();
    historyMillis = millis();
  }
}


void handleWifi(){
  String request;
  String reply;
  int counter = 0;
  int i;
  int v;

  WiFiClient client = server.available();
    if (!client) {
    return;
  }
  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    counter++;
    delay(1);
    if (counter > 250) return;      // do not wait until end of time
  }

  // Read the first line of the request
  request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); //  do not forget this one
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  
  client.println("<head>");
  client.println("<title>Plant irrigation system</title>");
  client.println("<meta http-equiv=\"refresh\" content=\"30\"/>");
  client.println("<style>");
  client.println("table, th, td {border: 2px solid black;border-collapse: collapse;padding: 5px;text-align: left;}");
  client.println("table#t01 th {background-color: black;color: white;}");
  client.println("table#t01 td {text-align: right;}");
  client.println("table#t01 tr:nth-child(even) {background-color:#eee;}");
  client.println("table#t01 tr:nth-child(odd) {background-color:#fff;}");
  client.println("</style>");
  client.println("</head>");
  client.println("<body>");

  client.println("Pump status table<br>");
  client.println("<table>");
  client.println("<tr><th>Pump#</th><th>ON/OFF</th></tr>");
  client.print("<tr><td>Pump0</td><td>");if (digitalRead(PUMP0_PIN)) client.print("ON"); else client.print("OFF");client.println("</td></tr>");
  client.print("<tr><td>Pump1</td><td>");if (digitalRead(PUMP1_PIN)) client.print("ON"); else client.print("OFF");client.println("</td></tr>");
  client.print("<tr><td>Pump2</td><td>");if (digitalRead(PUMP2_PIN)) client.print("ON"); else client.print("OFF");client.println("</td></tr>");
  client.print("<tr><td>Pump3</td><td>");if (digitalRead(PUMP3_PIN)) client.print("ON"); else client.print("OFF");client.println("</td></tr>");
  client.println("</table>");

  client.println("<br><br>");
  client.println("Sensor history table<br>");

  client.println("<table id=\"t01\">");

  reply = "";
  reply += "<tr>";
  reply += "<td>Sensor0</td>";
  for (i=0; i<sensorHistoryCount; i++)
  {
    reply += "<td>";
    reply += (String) sensor0History[i];
    reply += "</td>";
  }
  reply += "</tr>";
  client.println(reply);

  reply = "";
  reply += "<tr>";
  reply += "<td>Sensor1</td>";
  for (i=0; i<sensorHistoryCount; i++)
  {
    reply += "<td>";
    reply += (String) sensor1History[i];
    reply += "</td>";
  }
  reply += "</tr>";
  client.println(reply);

  reply = "";
  reply += "<tr>";
  reply += "<td>Sensor2</td>";
  for (i=0; i<sensorHistoryCount; i++)
  {
    reply += "<td>";
    reply += (String) sensor2History[i];
    reply += "</td>";
  }
  reply += "</tr>";
  client.println(reply);

  reply = "";
  reply += "<tr>";
  reply += "<td>Sensor3</td>";
  for (i=0; i<sensorHistoryCount; i++)
  {
    reply += "<td>";
    reply += (String) sensor3History[i];
    reply += "</td>";
  }
  reply += "</tr>";
  client.println(reply);

  reply = "";
  reply += "<tr>";
  reply += "<th>History</th>";
  for (i=0; i<sensorHistoryCount; i++)
  {
    reply += "<th>";
    reply += (String) i;
    reply += "</th>";
  }
  reply += "</tr>";
  client.println(reply);
  client.println("</table>");

  client.println("</body>");
  client.println("</html>");
}

void updateSensorData() {
  // enable the sensor current
  digitalWrite(MEASUREMENT_PIN, HIGH);

  delay(500);
  
  // read the new values from the sensors
  sensor0History[0] = adc.readADC(0);
  sensor1History[0] = adc.readADC(1);
  sensor2History[0] = adc.readADC(2);
  sensor3History[0] = adc.readADC(3);

  // disable the sensor current
  digitalWrite(MEASUREMENT_PIN, LOW);
}

void updateSensorHistory() {
  int i;
  
  // circle the buffer
  for (i = (sensorHistoryCount-1); i > 0; i--) {
    sensor0History[i] = sensor0History[i-1];
    sensor1History[i] = sensor1History[i-1];
    sensor2History[i] = sensor2History[i-1];
    sensor3History[i] = sensor3History[i-1];
  }
}

int handlePumps(){
  // turn on the pump if we are lower than the Low treshold
  // turn off the pump if we are higher than the High treshold
  
  if (sensor0History[0] < wateringLowThreshold) {
    digitalWrite(PUMP0_PIN, HIGH);
    pumps[0] = 1;
  }
  if (wateringHighThreshold < sensor0History[0]) {
    digitalWrite(PUMP0_PIN, LOW);
    pumps[0] = 0;
  }
  
  if (sensor1History[0] < wateringLowThreshold) {
    digitalWrite(PUMP1_PIN, HIGH);
    pumps[1] = 1;
  }
  if (wateringHighThreshold < sensor1History[0]) {
    digitalWrite(PUMP1_PIN, LOW);
    pumps[1] = 0;
  }

  if (sensor2History[0] < wateringLowThreshold) {
    digitalWrite(PUMP2_PIN, HIGH);
    pumps[2] = 1;
  }
  if (wateringHighThreshold < sensor2History[0]) {
    digitalWrite(PUMP2_PIN, LOW);
    pumps[2] = 0;
  }

  if (sensor3History[0] < wateringLowThreshold) {
    digitalWrite(PUMP3_PIN, HIGH);
    pumps[3] = 1;
  }
  if (wateringHighThreshold < sensor3History[0]) {
    digitalWrite(PUMP3_PIN, LOW);
    pumps[3] = 0;
  }

  if (pumps[0]) return(1);
  if (pumps[1]) return(1);
  if (pumps[2]) return(1);
  if (pumps[3]) return(1);
  return(0);
}

void debugOnSerial(){
  int i;
  
  Serial.print(sensor0History[0]);
  Serial.print("\t");
  Serial.print(sensor1History[0]);
  Serial.print("\t");
  Serial.print(sensor2History[0]);
  Serial.print("\t");
  Serial.print(sensor3History[0]);
  Serial.print("\t");

  Serial.print(digitalRead(PUMP0_PIN));
  Serial.print("\t");
  Serial.print(digitalRead(PUMP1_PIN));
  Serial.print("\t");
  Serial.print(digitalRead(PUMP2_PIN));
  Serial.print("\t");
  Serial.println(digitalRead(PUMP3_PIN));

/*  for (i = 0; i < sensorHistoryCount; i++) {
    Serial.print(sensor0History[i]);
    Serial.print("\t");
  }
  Serial.println();
  
  for (i = 0; i < sensorHistoryCount; i++) {
    Serial.print(sensor1History[i]);
    Serial.print("\t");
  }
  Serial.println();
  
  for (i = 0; i < sensorHistoryCount; i++) {
    Serial.print(sensor2History[i]);
    Serial.print("\t");
  }
  Serial.println();

  for (i = 0; i < sensorHistoryCount; i++) {
    Serial.print(sensor3History[i]);
    Serial.print("\t");
  }
  Serial.println();
*/  Serial.println();

}

