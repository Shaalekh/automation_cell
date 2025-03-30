#include <SoftwareSerial.h>
#include "VoiceRecognitionV3.h"

VR myVR(2 ,3);  // RX = 10, TX = 11

uint8_t buf[64];

// Voice Command Identifiers
enum VoiceCommands {
    LIGHT = 0,
    FAN = 1,
    ONE = 2,
    TWO = 3,
    THREE = 4,
    FOUR = 5,
    FIVE = 6,
    SIX = 7,
    ALL = 8,
    ON = 9,
    OFF = 10
};

// State Variables
bool waitingForDevice = true;  // Waiting for "light" or "fan"
bool waitingForNumber = false; // Waiting for "one" to "all"
bool waitingForAction = false; // Waiting for "on" or "off"

void setup() {
    Serial.begin(115200);
    myVR.begin(9600);

    if (myVR.clear() == 0) {
        Serial.println("Voice Recognition Module Ready.");
    } else {
        Serial.println("Module Not Found! Check connections.");
        while (1);
    }

    // Load the first group (light and fan)
    myVR.load(LIGHT);
    myVR.load(FAN);
}

void loop() {
    int ret = myVR.recognize(buf, 50);
    if (ret > 0) {
        Serial.print("Recognized Command: ");
        Serial.println(buf[1]);

        if (waitingForDevice) {
            handleDeviceSelection(buf[1]);
        } else if (waitingForNumber) {
            handleNumberSelection(buf[1]);
        } else if (waitingForAction) {
            handleAction(buf[1]);
        }
    }
}

// ========== Function Definitions ==========

// Step 1: Select "light" or "fan"
void handleDeviceSelection(int cmd) {
    if (cmd == LIGHT || cmd == FAN) { 
        Serial.print("Selected: ");
        Serial.println(cmd == LIGHT ? "Light" : "Fan");

        // Move to the next step: Select a number
        waitingForDevice = false;
        waitingForNumber = true;

        // Load next set of commands (1 to 6 + "all")
        myVR.clear();
        myVR.load(ONE);
        myVR.load(TWO);
        myVR.load(THREE);
        myVR.load(FOUR);
        myVR.load(FIVE);
        myVR.load(SIX);
        myVR.load(ALL);
    }
}

// Step 2: Select "one" to "all"
void handleNumberSelection(int cmd) {
    if (cmd >= ONE && cmd <= ALL) {
        Serial.print("Selected: ");
        if (cmd == ALL) {
            Serial.println("All");
        } else {
            Serial.println(cmd - ONE + 1); // Print number (e.g., "1", "2", etc.)
        }

        // Move to the next step: Select "on" or "off"
        waitingForNumber = false;
        waitingForAction = true;

        // Load next set of commands (on, off)
        myVR.clear();
        myVR.load(ON);
        myVR.load(OFF);
    }
}

// Step 3: Select "on" or "off"
void handleAction(int cmd) {
    if (cmd == ON || cmd == OFF) {
        Serial.print("Action: ");
        Serial.println(cmd == ON ? "ON" : "OFF");

        // Reset the cycle: Go back to "light" or "fan"
        waitingForAction = false;
        waitingForDevice = true;

        // Load "light" and "fan" again
        myVR.clear();
        myVR.load(LIGHT);
        myVR.load(FAN);
    }
}
