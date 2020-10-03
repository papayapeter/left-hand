// *** Zeno Gries - 2018
// *** Die Hände R2.0

#include "hand.h"

// pins
const uint8_t MICROSTEP = 7;
const uint8_t ENABLE = 8;
const uint8_t STEP = 9;
const uint8_t DIRECTION = 10;
const uint8_t SEED = 14;
const uint8_t TOUCH = 15;
const uint8_t SWITCH = 16;
const uint8_t LED = 13;

// constants
const uint16_t motorFailsafe = 350;         // when to stop moving during calibration
const uint16_t motorLimit= 185;            // what should be the calibrated position
const uint16_t motorAcceleration = 50;     // how fast does the motor accelerate during closing?
const uint16_t motorMaxSpeed = 50;         // how fast will the motor go during closing?
const uint16_t motorAccelerationBack = 25; // how fast does the motor accelerate during opening?
const uint16_t motorMaxSpeedBack = 25;     // how fast will the motor go during opening?
//const uint16_t motorMaxCome = 40;          // full closed position if the "come here" gesture?
//const uint16_t comeTime = 4000;              // duration of the "come here" gesture?

const uint16_t touchThreshhold = 200;       // how sensitive is the hand towards touch?
const uint16_t  calibrationLength = 80;     // how many intervals to read for calibration?
const uint16_t  pauseLength = 500;           // how many intervals between touch and calibration?
const uint16_t  touchLength = 20;           // how many intervals to read for touch?

// objects
Hand hand(MICROSTEP, ENABLE, STEP, DIRECTION, SEED, TOUCH, SWITCH, LED);

void setup() {
  hand.setMotor(motorFailsafe, motorLimit, motorAcceleration, motorMaxSpeed, motorAccelerationBack, motorMaxSpeedBack);
  hand.setTouch(touchThreshhold, calibrationLength, pauseLength, touchLength, 1);
}

void loop() {
  // put your main code here, to run repeatedly:

}
