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

//Lin testing

int Cnt1 = 0;

//None heatpump ETXV ID34
#define PTC_Req 0x13
#define PTC_Cmd 0x26
uint16_t PTC_HV = 0;  //0-2250 range
float PTC_temp1 = 0;

uint16_t PTC_TempReq = 0;
uint16_t PTC_LimReq = 0;

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
uint8_t MSG_CNT = 0;

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

  //Boot LIN

  outFrame.id = 0x3C;
  outFrame.len = 7;
  outFrame.chktype = ENHANCED;
  outFrame.buf[0] = 0x00;
  outFrame.buf[1] = 0xFF;
  outFrame.buf[2] = 0xFF;
  outFrame.buf[3] = 0xFF;
  outFrame.buf[4] = 0xFF;
  outFrame.buf[5] = 0xFF;
  outFrame.buf[6] = 0xFF;
  outFrame.buf[7] = 0x00;
  lin1.sendLinFrame(outFrame);
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
  switch (MSG_CNT) {
    case 0:
      {
        outFrame.id = 0x26;
        outFrame.len = 8;
        outFrame.chktype = ENHANCED;
        outFrame.buf[0] = 0x39;
        if (Cnt1 < 7) {
          outFrame.buf[1] = 0xC4;
        } else {
          outFrame.buf[1] = 0xC5;
        }
        if (PTC_TempReq > 0 && PTC_LimReq > 0) {
          outFrame.buf[2] = PTC_TempReq; //Any above 0 works
          outFrame.buf[3] = PTC_TempReq;
        } else {
          outFrame.buf[2] = 0;
          outFrame.buf[3] = 0;
        }
        outFrame.buf[4] = 0x00;
        outFrame.buf[5] = 0xAA;
        if (Cnt1 < 3) {
          outFrame.buf[6] = 0x90;
          outFrame.buf[7] = 0x01;
        } else {
          if (PTC_LimReq > 0) {
            outFrame.buf[6] = PTC_LimReq; //needs highest bit set 80 works, 96 works
          } else {
            outFrame.buf[6] = 0x00;
          }
          outFrame.buf[7] = 0x00;
        }
        if (Cnt1 < 20) {
          Cnt1++;
        }
        lin1.sendLinFrame(outFrame);
      }
      break;
    case 1:  //Lin botschaft für A/C Kompressor
      {      // fill data vor AC
        lin1.readLinFrame(0x13, 8, ENHANCED);
      }
      break;
    default:
      {
      }
      break;
  }
  MSG_CNT++;
  if (MSG_CNT > 1) MSG_CNT = 0;
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
          PTC_LimReq = Serial.parseInt();
          if (PTC_LimReq > 255) {
            PTC_LimReq = 0;
          }
          Serial.println();
          Serial.print("! PTC Lim Req :");
          Serial.print(PTC_LimReq);
          Serial.println();
        }
        break;

      case 'h':
        if (Serial.available() > 0) {
          PTC_TempReq = Serial.parseInt();
          if (PTC_TempReq > 255) {
            PTC_TempReq = 0;
          }
          Serial.println();
          Serial.print("! PTC Temp Req :");
          Serial.print(PTC_TempReq);
          Serial.println();
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
  if (frame.id == 0xD3)  //0x13
  {
    if (frame.buf[1] < 0xFE) {
      PTC_HV = frame.buf[1] * 2;
      PTC_temp1 = frame.buf[5] * 0.25;

      Serial.print("PTC| HV: ");
      Serial.print(PTC_HV);
      Serial.print("V| Temp1: ");
      Serial.print(PTC_temp1);
      Serial.print("C|");
      Serial.println();
    }
  }
}