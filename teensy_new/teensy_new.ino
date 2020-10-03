// *** Zeno Gries - 2018
// *** Die Hände R2.0

#include "hand.h"

// Pins
const int MICROSTEP = 7;
const int ENABLE = 8;
const int STEP = 9;
const int DIRECTION = 10;
const int SEED = 14;
const int TOUCH = 15;
const int SWITCH = 16;
const int LED = 13;

// constants
const int TouchThreshhold = 200;      // Wie fein reagiert die Hand auf Berührung?
const int MotorLimit = 185;           // Das harte Maximum
const int MotorFailsafe = 350;        // Wann schaltet sich das gerät ab?
const int MotorAcceleration = 50;     // Wie schnell Beschleunigt der Motor, wenn die Hand zu geht?
const int MotorMaxSpeed = 50;         // Wie schnell ist der Motor maximal, wenn die Hand zu geht?
const int MotorAccelerationBack = 25; // Wie schnell Beschleunigt der Motor, wenn die Hand auf geht?
const int MotorMaxSpeedBack = 25;     // Wie schnell ist der Motor maximal, wenn die Hand auf geht?

const int TouchLength = 20;           // Wie viele Intervalle werden für die Berührung gelesen?
const int CalibrationLength = 80;     // Wie viele Intervalle werden für die Kalibrierung gelesen?
const int PauseLength = 500;          // Wie viele Intervalle sind zwischen Berührung und Kalibrierung

const int WiggleTime = 200;           // Intervall für das Zucken
const int WiggleMagnitude = 3;        // Stärke des Zuckens
const int RemainTime = 500;           // Wie lange verharrt die Hand geschlossen?

const int ComeMotorMin = 0;           // Voll auf Stellung der Komm her Bewegung
const int ComeMotorMax = 40;          // Voll zu Stellung der Komm her Bewegung //***1: 50 2: 40 3: 60
const int ComeTime = 4000;            // Dauer der Komm her Bewegung

// objects
Hand hand(MICROSTEP, ENABLE, STEP, DIRECTION, SEED, TOUCH, SWITCH, LED);

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
