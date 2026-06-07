// Copyright (c) 2025 BahnOtto

#include "Arduino.h"
#include <EEPROM.h>
#include <NmraDcc.h>
#include <Servo.h>

const char Vers[] = "Weichen BR3 VSC 25ß706";
//--------------------------------------------

#define DCC_PIN 2
NmraDcc Dcc;

const byte servoDelay = 10; // 25
const byte maxServo   = 8;  // needed number

unsigned long servoTime = millis() + servoDelay;

struct servoData {
  uint16_t address;
  byte outputPin; 		// Arduino output pin for additional function (not where servo is attached to)
  byte outputInverse;
  byte pos;
  byte setPos;
  byte minPos;
  byte maxPos;
  Servo servo;
};

servoData Servo[maxServo];

void setServoConfig() {
	Servo[0].address = 93;	//WBR31 stillgelegt jezt Trix 122
	Servo[0].outputPin = 12;
	Servo[0].outputInverse = true;
	Servo[0].servo.attach(A0);
	Servo[0].minPos = 70;   //72
	Servo[0].maxPos = 97;   //118

	Servo[1].address = 94;  //WBR32
	Servo[1].outputPin = 11;
	Servo[1].outputInverse = true;
	Servo[1].servo.attach(A1);
	Servo[1].minPos = 65;   //68
	Servo[1].maxPos = 112;

	Servo[2].address = 95;	//WBR33 BW Nebenbahn
	Servo[2].outputPin = 10;
	Servo[2].outputInverse = true;
	Servo[2].servo.attach(A2);
	Servo[2].minPos = 70;   //85 Hauptstrecke
	Servo[2].maxPos = 135;  //135

	Servo[3].address = 96;  //WBR34 BWl
	Servo[3].outputPin = 9;
	Servo[3].outputInverse = true;
	Servo[3].servo.attach(A3);
	Servo[3].minPos = 70; //70
	Servo[3].maxPos = 130;

	Servo[4].address = 97;	//WBR35 BWr
	Servo[4].outputPin = 8;
	Servo[4].outputInverse = false;
	Servo[4].servo.attach(A4);
	Servo[4].minPos = 48; // 68
	Servo[4].maxPos = 123; //105

	Servo[5].address = 98; //WBR36
	Servo[5].outputPin = 7;
	Servo[5].outputInverse = false;
	Servo[5].servo.attach(A5);
	Servo[5].minPos = 65;
	Servo[5].maxPos = 117;

	Servo[6].address = 99; //WBR37 Bogenweiche
	Servo[6].outputPin = 6;
	Servo[6].outputInverse = false;
	Servo[6].servo.attach(3);
	Servo[6].minPos = 78;
	Servo[6].maxPos = 112; // gerade

	Servo[7].address = 100; //Ek1 Servo Entkuppler
	Servo[7].outputPin = 5;
	Servo[7].outputInverse = false;
	Servo[7].servo.attach(4);
	Servo[7].minPos = 80;
	Servo[7].maxPos = 100;
}

void setRelay(int i) {
	if (Servo[i].setPos == Servo[i].maxPos)
		digitalWrite(Servo[i].outputPin, Servo[i].outputInverse);
	if (Servo[i].setPos == Servo[i].minPos)
		digitalWrite(Servo[i].outputPin, !Servo[i].outputInverse);
}

void notifyDccAccTurnoutOutput(uint16_t Addr, uint8_t Direction, uint8_t OutputPower) {
	Serial.println(Addr);
  Serial.println(Direction);

  for(int i=0; i<maxServo; i++) {
      if(Addr == Servo[i].address)
      {
        Serial.print(Addr);
		if(Direction == 1) {   //prüfen ??????
          EEPROM.write(i, Servo[i].maxPos);
          Servo[i].setPos = Servo[i].maxPos;
		  Serial.println(" > max"); }
        else {
          EEPROM.write(i, Servo[i].minPos);
          Servo[i].setPos = Servo[i].minPos;
		  Serial.println(" > min");
        }
        setRelay(i);
      }
  }
}

void setup() {
	Serial.begin(9600);
	Serial.println(Vers);
	setServoConfig();
	for (int i = 0; i < maxServo; i++) {
		Servo[i].pos = EEPROM.read(i);
		pinMode(Servo[i].outputPin, OUTPUT); ///
		if (Servo[i].pos == Servo[i].maxPos) {
			Servo[i].setPos = Servo[i].maxPos;
		} else {
			Servo[i].setPos = Servo[i].minPos;
		}
		setRelay(i);
	}
  Dcc.pin(digitalPinToInterrupt(DCC_PIN), DCC_PIN, false);
  Dcc.init(MAN_ID_DIY, 1, FLAGS_DCC_ACCESSORY_DECODER | FLAGS_OUTPUT_ADDRESS_MODE, 0);
}

void loop() {
   Dcc.process();
   if (millis() > servoTime) {
      servoTime = millis() + servoDelay;
      for(int i=0; i<maxServo; i++) {
        if (Servo[i].pos < Servo[i].setPos)  Servo[i].pos++;
        if (Servo[i].pos > Servo[i].setPos)  Servo[i].pos--;
        Servo[i].servo.write(Servo[i].pos);
     }
   }
}

