/*

  LIN Interface for VW HV AC compressor and PTC Heater

  Tom de Bree - Volt Influx Ltd

********************************************************************
*/

#include "Lin_Tom.h"

LIN lin1;  // LIN channel 1

Lin_Frame_t outFrame;
Lin_Frame_t inFrame;

/////Version Identifier/////////
uint32_t firmver = 240604;  //Year Month Day Ver

int ledState = LOW;               // ledState used to set the LED
unsigned long interval = 200000;  // interval at which to blinkLED to run every 0.2 seconds

// Create an IntervalTimer object
IntervalTimer myTimer;

// LIN serial Interface

int Lin1serSpeed = 19200;  //20000;    // speed LIN
int Lin2serSpeed = 19200;  //20000;    // speed LIN

int Lin1CSPin = 32;  // CS Pin
int Lin1_Fault = 28;
int Lin2CSPin = 18;  // CS Pin

//LIN bus IDs//

int LinTestId = 0;

//////////////////////////////////////////

////Software Timers////
unsigned long looptime500, looptime100, looptime10 = 0;

// Global Variables
byte LinMessageA[200] = { 0 };
byte lin22[9] = { 0 };

uint16_t StartBreak = 0;

int led1 = 23;

//
uint8_t can_data;
uint8_t MSG_CNT = 3;

//SerialUSB comms

uint8_t menuload = 0;
char msgString[128];  // Array to store serial string
byte incomingByte = 0;

// Configuration Debug
bool debug_Lin = 1;
bool debug_rawLin = 1;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(Lin1_Fault, INPUT);
  pinMode(Lin1CSPin, OUTPUT);  // CS Signal LIN Tranceiver

  digitalWrite(Lin1CSPin, HIGH);

  pinMode(Lin2CSPin, OUTPUT);  // CS Signal LIN Tranceiver
  digitalWrite(Lin2CSPin, HIGH);

  digitalWrite(LED_BUILTIN, HIGH);

  //lin1.begin(&Serial3, Lin1serSpeed);
  lin1.begin(&Serial3, Lin1serSpeed);

  //Lin2Serial.begin(Lin2serSpeed);

  Serial.begin(115200);

  delay(1000);
  pinMode(led1, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.print("Teensy 4.0 Lin Test");
  Serial.println();
  Serial.print(Lin1serSpeed);
  Serial.println(" Baud Rate");

  myTimer.begin(blinkLED, interval);
}

void loop() {
  if (Serial.available() > 0) {
    Serialcomms();
  }
  if (lin1.CheckBuffer(inFrame)) {
    DecodeLin(inFrame, 1);
  }
  if (millis() - looptime100 > 500) {
    looptime100 = millis();
    Tasks100ms();
  }
}

void Tasks100ms() {
  send_LIN();
}

void blinkLED() {
  ledState = !ledState;

  digitalWrite(LED_BUILTIN, ledState);
  digitalWrite(led1, ledState);
}

void Serialcomms() {
  incomingByte = Serial.read();

  if (menuload == 0)  //normal run no serial menu opened
  {
    switch (incomingByte) {
      case 'p':
        if (Serial.available() > 0) {
        }
        break;

      case 'h':
        if (Serial.available() > 0) {
        }
        break;

      default:
        // if nothing else matches, do the default
        // default is optional
        break;
    }
  }
}


void send_LIN()  // this runs every 50ms and read/write data
{
  lin1.readLinFrame(LinTestId, 8, ENHANCED);

  LinTestId++;

  if (LinTestId > 59) {
    LinTestId = 1;
  }
}

void DecodeLin(Lin_Frame_t &frame, uint8_t bus) {
  if (debug_rawLin) {
    Serial.print("Lin");
    Serial.print(bus);
    Serial.print("|ID: ");
    Serial.print(frame.id, HEX);
    Serial.print("- Data ");
    for (int ixx = 0; ixx < 8; ixx++) {
      Serial.print(frame.buf[ixx], HEX);
      Serial.print(":");
    }
    Serial.print("- Check: ");
    Serial.print(frame.chk, HEX);
    Serial.println();
  }
}