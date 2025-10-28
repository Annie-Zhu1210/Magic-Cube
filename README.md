# Magic-Cube

*A personal interactive light project for CASA0014*

The Magic Cube is an interactive light installation that responds to touch.
Imagine a cube with six sides — excluding the top and bottom, the four side faces each represent a primary colour (red, yellow, blue, and white). By touching or combining different sides, you can mix colours in real time and create your own light spectrum.

This project explores the relationship between human interaction and ambient light, combining hardware design, sensor control, and creative coding. The lighting effects can be visualised through Vespera.

## **How it works** ##

Four sides of the cube are connected to a touch sensor that triggers LED colour changes.

The cube uses an MKR WiFi 1010 microcontroller (or similar board) to process input from touch sensors and control an RGB LED strip.

The logic is implemented in Arduino IDE, and messages are sent through MQTT to the Vespera visualisation platform.


## **Hardware Components** ##

- Touch sensors*4
- A microcontroller with WiFi connection (MKR WiFi 1010, etc.)
- A 1000 µF - 2000 µF capacitor
- Power supply
  
## **Images**

### **Final Cube Desgin** ###

![This shows what the cube looks like](/images/CompletedCube/CompletedCube_1.JPG)

### **Light Effects** ###

![This shows what the rainbow effect on the light looks like, by controlling the sensors on the cube](/images/LightEffect.jpg)

### **A sketch of the prototype idea.** ###

![This is a picture of the initial prototype idea of the magic cube.](/images/Sketch_of_Initial_Idea.JPG)
