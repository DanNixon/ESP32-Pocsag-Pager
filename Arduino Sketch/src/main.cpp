/*
ESP32 Pager Proof Of Concept
This code implements a basic pager, initally designed for DAPNET use, but can be modified to suit any need.

Additional files:
-config.h contains the user configuration (frequency, offset, RIC, ringtones, etc)
-periph.h contains pin assignment

Frequency offset must be configured for reliable decoding. At present time, there isn't a "cal" mode available, but it is to be implemented.


*/
#include <Arduino.h>
#include "periph.h"
#include "config.h"
#include <RadioLib.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

SX1278 radio = new Module(LORA_SS, LORA_DIO0, LORA_RST, LORA_DIO1); // Radio module instance
// create Pager client instance using the FSK module
PagerClient pager(&radio); // Pager client instance

#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST); // OLED display instance

void pocsagInit() {
    // initialize SX1278 with default settings
    Serial.print(F("[SX1278] Initializing ... ")); // Print a message to the serial port
    int state = radio.beginFSK(); // Initialize the radio module

    if (state == RADIOLIB_ERR_NONE) {
        Serial.println(F("success!")); // Report success
    } else {
        Serial.print(F("failed, code "));
        Serial.println(state); // Report error
        while (true); // Halt
    }

    // initialize Pager client
    Serial.print(F("[Pager] Initializing ... ")); // Print a message to the serial port
    state = pager.begin(frequency + offset, 1200); // Initialize the pager client
    if (state == RADIOLIB_ERR_NONE) {
        Serial.println(F("success!")); // Report success
    } else {
        Serial.print(F("failed, code ")); // Report error
        Serial.println(state); // Report error
        while (true); // Halt
    }
}

void pocsagStartRx() {
    // start receiving POCSAG messages
    Serial.print(F("[Pager] Starting to listen ... ")); // Print a message to the serial port
    // address of this "pager":     1234567
    int state = pager.startReceive(LORA_DIO2, 200, 0); // Start receiving messages
    if (state == RADIOLIB_ERR_NONE) {
        Serial.println(F("success!")); // Report success
    } else {
        Serial.print(F("failed, code ")); // Report error
        Serial.println(state); // Report error
        while (true); // Halt
    }
}

void displayInit() {

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed")); // Report error
        while (true);  // Don't proceed, loop forever
    }
    // Clear the buffer
    display.clearDisplay();  // clears the screen and buffer
    display.display(); // Display the logo
}

void setup() {
    Serial.begin(115200); // Initialize serial port
    pinMode(LED, OUTPUT); // Set LED pin as output
    displayInit(); // Initialize the display
    pocsagInit(); // Initialize the pager
    pocsagStartRx(); // Start receiving messages
    pinMode(35, INPUT);
}

auto batteryLastRefresh = 0;

void displayBattery() {
    float vbat = ((float)(analogRead(35)) / 4095.0) * 2.0 * 3.3;

    display.fillRect(0, 55, 127, 63, WHITE);

    display.setCursor(1, 56);
    display.setTextColor(BLACK);
    display.print(vbat);
    display.print("V");

    display.display();
}

void displayPage(String address, String text) { // Display the received message
    display.clearDisplay();
    display.setTextSize(1);

    display.fillRect(0, 0, 127, 9, WHITE);

    display.setCursor(1, 1);
    display.setTextColor(BLACK);
    display.print(address);

    display.setCursor(0, 12);
    display.setTextColor(WHITE);
    display.print(text);

    display.display();

    displayBattery();
}

void ringBuzzer(int ringToneChoice) {
    for (int i = 0; i < NOTENUMBER; i++) {
        tone(BUZZER, beepTones[ringToneChoice][i], 130); // Play the tone
    }
    for (int i = 0; i < 20; i++) { // Blink the LED
        digitalWrite(LED, HIGH); // Turn the LED on
        delay(100); // Wait for 100 millisecond(s)
        digitalWrite(LED, LOW); // Turn the LED off
        delay(100); // Wait for 100 millisecond(s)
    }
}

void loop() {
    // the number of batches to wait for
    // 2 batches will usually be enough to fit short and medium messages
    if (pager.available() >= 2) {
        Serial.print(F("[Pager] Received pager data, decoding ... ")); // Print a message to the serial port
        // you can read the data as an Arduino String
        String str; // Create a string
        uint32_t addr = 0; // Create a variable to store the address
        int state = pager.readData(str, 0, &addr); // Read the data
        if (state == RADIOLIB_ERR_NONE) { // Check for errors
            Serial.println(F("success!")); // Report success

            // print the received data
            Serial.print(F("[Pager] Address:\t")); // Print a message to the serial port
            Serial.print(String(addr)); // Print the address
            Serial.print(F("[Pager] Data:\t")); // Print a message to the serial port
            Serial.println(str); // Print the data

            for (int i = 0; i < RICNUMBER; i++) { // Check if the address is in the list
                if (addr == ric[i].ricvalue) { // If the address is in the list
                    displayPage(ric[i].name, str); // Display the message
                    ringBuzzer(ric[i].ringtype); // Ring the buzzer
                }
            }
        } else {
            Serial.print(F("failed, code ")); // Report error
            Serial.println(state);  // Report error
        }
    }

    auto now = millis();
    if (now - batteryLastRefresh > 2000) {
        displayBattery();
        batteryLastRefresh = now;
    }
}
