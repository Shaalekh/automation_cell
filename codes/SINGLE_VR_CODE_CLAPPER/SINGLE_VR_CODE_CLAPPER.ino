#include <SoftwareSerial.h>
#include "VoiceRecognitionV3.h"

// VR Module uses SoftwareSerial on pins 2 (RX) and 3 (TX)
SoftwareSerial vrSerial(2, 3);
VR myVR(2, 3);

// Relay definitions:
// Lights on digital pins 4, 5, 6, 7, 8, 9
// Fans on digital pins 10, 11, 12, 13, and analog pins A0, A1
const int NUM_LIGHTS = 6;
const int NUM_FANS   = 6;
int lightRelays[NUM_LIGHTS] = {4, 5, 6, 7, 8, 9};
int fanRelays[NUM_FANS]   = {10, 11, 12, 13, A0, A1};

// Voice Command Mapping (indices):
// 0: "light", 1: "fan"
// 2: "one", 3: "two", 4: "three", 5: "four", 6: "five", 7: "six"
// 8: "all"
// 9: "on", 10: "off"

// State machine for the voice flow
enum State { WAIT_DEVICE, WAIT_NUMBER, WAIT_ACTION };
State currentState = WAIT_DEVICE;
uint8_t selectedDevice = 255; // 0: light, 1: fan
uint8_t selectedNumber = 255; // 0: all, 1-6 for specific relay

// Threshold for sensor on A2
const int SENSOR_THRESHOLD = 80;

// Function prototypes
void loadDeviceCommands();
void loadNumberCommands();
void loadActionCommands();
void handleCommand(uint8_t cmd);
void executeAction(uint8_t device, uint8_t number, bool actionOn);
void patternLightsOn();
  
void setup() {
  Serial.begin(115200);
  vrSerial.begin(9600);
  myVR.begin(9600);

  if (myVR.clear() == 0) {
    Serial.println("VR Module Ready.");
  } else {
    Serial.println("VR Module not detected!");
    while (1);
  }
  
  // Set relay pins as OUTPUT and default HIGH (relays are active LOW)
  for (int i = 0; i < NUM_LIGHTS; i++) {
    pinMode(lightRelays[i], OUTPUT);
    digitalWrite(lightRelays[i], HIGH);
  }
  for (int i = 0; i < NUM_FANS; i++) {
    pinMode(fanRelays[i], OUTPUT);
    digitalWrite(fanRelays[i], HIGH);
  }
  
  loadDeviceCommands();
  Serial.println("System Ready.");
}

void loop() {
  // Check sensor value on A2
  int sensorVal = analogRead(A2);
  if (sensorVal > SENSOR_THRESHOLD) {
    Serial.print("Sensor triggered, value = ");
    Serial.println(sensorVal);
    patternLightsOn();
    // Wait a bit to avoid retriggering repeatedly
    delay(2000);
  }
  
  // Handle voice recognition commands
  uint8_t buf[64];
  int ret = myVR.recognize(buf, 50);
  if (ret > 0) {
    uint8_t cmd = buf[1];
    Serial.print("Received Command Index: ");
    Serial.println(cmd);
    handleCommand(cmd);
  }
}

void handleCommand(uint8_t cmd) {
  switch (currentState) {
    case WAIT_DEVICE:
      if (cmd == 0 || cmd == 1) {
        selectedDevice = cmd; // 0: light, 1: fan
        if (selectedDevice == 0)
          Serial.println("Device Selected: Light");
        else
          Serial.println("Device Selected: Fan");
        currentState = WAIT_NUMBER;
        loadNumberCommands();
      } else {
        Serial.println("Waiting for device command (0:light, 1:fan)...");
      }
      break;
      
    case WAIT_NUMBER:
      if (cmd >= 2 && cmd <= 7) {
        // "one" to "six": convert (2->1, 3->2, ..., 7->6)
        selectedNumber = cmd - 2 + 1;
        Serial.print("Number Selected: ");
        Serial.println(selectedNumber);
        currentState = WAIT_ACTION;
        loadActionCommands();
      } else if (cmd == 8) {
        selectedNumber = 0;  // 0 indicates "all"
        Serial.println("Number Selected: All");
        currentState = WAIT_ACTION;
        loadActionCommands();
      } else {
        Serial.println("Waiting for number command (2-8)...");
      }
      break;
      
    case WAIT_ACTION:
      if (cmd == 9 || cmd == 10) {
        bool actionOn = (cmd == 9); // 9: on, 10: off
        if (actionOn)
          Serial.println("Action: ON");
        else
          Serial.println("Action: OFF");
        executeAction(selectedDevice, selectedNumber, actionOn);
        // Reset state machine
        currentState = WAIT_DEVICE;
        loadDeviceCommands();
      } else {
        Serial.println("Waiting for action command (9:on, 10:off)...");
      }
      break;
  }
}

void executeAction(uint8_t device, uint8_t number, bool actionOn) {
  int relayState = actionOn ? LOW : HIGH;  // Active LOW: LOW turns relay ON
  if (device == 0) { // Light group
    if (number == 0) {  // "All" lights selected
      if (actionOn) {
        patternLightsOn();
      } else {
        // Turn all lights OFF
        for (int i = 0; i < NUM_LIGHTS; i++) {
          digitalWrite(lightRelays[i], HIGH);
        }
        Serial.println("All Lights turned OFF");
      }
    } else if (number >= 1 && number <= NUM_LIGHTS) {
      digitalWrite(lightRelays[number - 1], relayState);
      Serial.print("Light ");
      Serial.print(number);
      if (actionOn)
        Serial.println(" turned ON");
      else
        Serial.println(" turned OFF");
    }
  } else if (device == 1) { // Fan group
    if (number == 0) {  // All fans
      for (int i = 0; i < NUM_FANS; i++) {
        digitalWrite(fanRelays[i], relayState);
      }
      Serial.print("All Fans turned ");
      if (actionOn)
        Serial.println("ON");
      else
        Serial.println("OFF");
    } else if (number >= 1 && number <= NUM_FANS) {
      digitalWrite(fanRelays[number - 1], relayState);
      Serial.print("Fan ");
      Serial.print(number);
      if (actionOn)
        Serial.println(" turned ON");
      else
        Serial.println(" turned OFF");
    }
  }
}

// Function to perform the special pattern: turn on lights in groups of two with a 500ms delay
void patternLightsOn() {
  Serial.println("Turning ALL Lights ON with pattern:");
  for (int i = 0; i < NUM_LIGHTS; i += 2) {
    digitalWrite(lightRelays[i], LOW);
    digitalWrite(lightRelays[i+1], LOW);
    Serial.print("Lights ");
    Serial.print(i+1);
    Serial.print(" & ");
    Serial.print(i+2);
    Serial.println(" ON");
    delay(500);
  }
}

void loadDeviceCommands() {
  myVR.clear();
  myVR.load((uint8_t)0);
  myVR.load((uint8_t)1);
  Serial.println("Loaded Device Commands (0: light, 1: fan)");
}

void loadNumberCommands() {
  myVR.clear();
  for (uint8_t i = 2; i <= 8; i++) {
    myVR.load(i);
  }
  Serial.println("Loaded Number Commands (2-8)");
}

void loadActionCommands() {
  myVR.clear();
  myVR.load((uint8_t)9);
  myVR.load((uint8_t)10);
  Serial.println("Loaded Action Commands (9: on, 10: off)");
}
