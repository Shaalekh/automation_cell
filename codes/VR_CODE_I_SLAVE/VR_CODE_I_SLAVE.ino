#include <Wire.h>

#define SLAVE_ADDRESS 8

// Relay Pins (Change as per your setup)
int lightRelays[6] = {2, 3, 4, 5, 6, 7};
int fanRelays[6] = {8, 9, 10, 11, 12, 13};

void setup() {
    Wire.begin(SLAVE_ADDRESS);
    Wire.onReceive(receiveEvent);
    Serial.begin(115200);

    // Set relay pins as OUTPUT and default HIGH (OFF)
    for (int i = 0; i < 6; i++) {
        pinMode(lightRelays[i], OUTPUT);
        pinMode(fanRelays[i], OUTPUT);
        digitalWrite(lightRelays[i], HIGH);
        digitalWrite(fanRelays[i], HIGH);
    }
    Serial.println("Slave Ready");
}

void loop() {
    // Nothing here, everything runs via I2C interrupt
}

void receiveEvent(int bytes) {
    if (bytes == 3) { // Expecting 3 bytes: Device ('L' or 'F'), Number (1-6 or 99), State (1 or 0)
        char device = Wire.read();  // 'L' for Light, 'F' for Fan
        int number = Wire.read();   // 1-6, 99 for all
        int state = Wire.read();    // 1 = ON (LOW), 0 = OFF (HIGH)

        Serial.print("Received: ");
        Serial.print(device);
        Serial.print(number);
        Serial.print(" ");
        Serial.println(state);

        if (device == 'L') {
            controlRelays(lightRelays, number, state);
        } else if (device == 'F') {
            controlRelays(fanRelays, number, state);
        }
    }
}

void controlRelays(int relays[], int number, int state) {
    int relayState = state == 1 ? LOW : HIGH; // 1 = ON (LOW), 0 = OFF (HIGH)
    
    if (number == 99) {  // Control all relays
        for (int i = 0; i < 6; i++) {
            digitalWrite(relays[i], relayState);
        }
    } else if (number >= 1 && number <= 6) {
        digitalWrite(relays[number - 1], relayState);
    }
}
