# Magic-Cube

*A personal interactive light project for Connected Environments CASA0014*

The Magic Cube is an interactive light installation that responds to touch.
Imagine a cube with six sides — apart from the top and the bottom, the four side faces each represent a **primary colour in traditional art theory** (red, yellow, blue, and white). By touching or combining different sides, you can mix colours in real time and create your own colour palette.

This project highlights **the relationship between people and their environment** through an IoT system, through practical experience in electronic prototyping using hardware and software. The lighting effects were visualised on the light **"Vespera"**.

## How it works

There are two states for each sensor: ON and OFF. One touch switches the state, and the corresponding colour represented by that sensor will be added or subtracted.

* Red - Red
* Yellow -Yellow
* Blue - Blue
* White - White
* Red + Yellow = Orange
* Red + Blue = Purple
* Yellow + Blue = Green
* Red + Yellow + Blue = A Colourful Rainbow Animation

The white side touch sensor controls not only the white light, but also the brightness. A single **short touch** of the white side sensor will turn on/off the white light, while a **long touch** of this sensor will change the **brightness** of the existing colour.

*(Note:If you don't need the white light and expect to see other colours on, remember to turn off the white light. Otherwise, there might be unsatble colours or flickers.)*


## Hardware Components

- Touch sensors*4
- A microcontroller with WiFi connection (MKR WiFi 1010, etc.)
- A 1000 µF - 2000 µF capacitor
- Power supply
- Multiple jumper wires

## The IoT Architecture

The system followed an IoT architecture. Four sides of the cube are connected to four touch sensors that trigger LED colour changes. The cube uses an MKR WiFi 1010 microcontroller (or similar board) to process input from touch sensors and control an RGB LED strip. The logic is implemented in Arduino IDE, and messages are sent through MQTT to the Vespera and a light simulation webpage, Luminaire, through Wi-Fi. The Luminaire simulates the colour grids of the Vespera light with my allocated MQTT topic _student/CASA0014/luminaire/31_.

When touches are detected on the touch sensors, messages are sent to the MQTT topic, and the corresponding coloured light appears.

<img src="/images/The IoT Arcitecture.png" width="800">

## Images

### Final Cube Desgin ###

<img src="/images/CompletedCube/CompletedCube_1.JPG" width="500">

### Light Effects ###

<img src="/images/LightEffect.jpg" width="500">

### A sketch of the prototype idea ###

<img src="/images/Sketch_of_Initial_Idea.JPG" width="500">

## How To Use

The main structure, the network connection, the LED on the microcontroller as an indicator, and the sensitive data were split into different files. 
1. First, download the three files from the [cube_light_test](/cube_light_test) and open them on Arduino IDE.
2. Create a new **secret.h** file in the same folder. Use the format below to enter your sensitive data.
```
define SECRET_SSID "ssid name"
define SECRET_PASS "ssid password"
define SECRET_MQTTUSER "user name"
define SECRET_MQTTPASS "password"
```
3. Update your **MQTT information** and **pin numbers** [in the main file](/cube_light_test/cube_light_test.ino).
4. Update the **NeoPixel configuration** of your light in the [main file](/cube_light_test/cube_light_test.ino).
5. Select your board and **Upload**.
6. The LED indicator light on the microcontroller will represent the expected colour of your light if it runs successfully.
