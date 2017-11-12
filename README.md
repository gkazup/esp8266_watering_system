# esp8266_watering_system
Internet enabled small scale watering system based on thinger.io

BOM:
- NodeMcu Lua v3 development board based on esp8266,
- 4 soil moisture sensors,
- 4 small water pumps,
- 4-way relay board,
- free thinger.io account,
- MCP3008 or MCP3004 - 10-Bit ADC With SPI Interface

Each motor is associated with a moisture sensor and connected to a relay.

There are multiple options to handle data visualization:
- Sensor data and motor state is sent to web service - I choose thinger.io for visualization and remote control
  - ==> esp8266_watering_thinger_v1
  - ==> esp8266_watering_thinger_v2
- Data is shown on a web page hosted locally on the esp
  - ==> esp8266_watering_website
- Data could also be sent to an MQTT server to be processed
- etc.
