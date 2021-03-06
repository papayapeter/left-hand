// *** Zeno Gries - 2018
// *** Die Hände R2.0

#include "hand.h"

// pins ------------------------------------------------------------------------
const uint8_t MICROSTEP = 7;
const uint8_t ENABLE = 8;
const uint8_t STEP = 9;
const uint8_t DIRECTION = 10;
const uint8_t SEED = 14;
const uint8_t TOUCH = 15;
const uint8_t SWITCH = 16;
const uint8_t LED = 13;

// constants -------------------------------------------------------------------
// motor
const uint16_t motorFailsafe = 300;                 // when to stop moving during calibration
const uint16_t motorLimit = 185;                    // what should be the calibrated position
const uint16_t motorMax = 180;                      // what should be the maximum position (always less than the calibrated position)
const uint16_t motorAcceleration = 50;              // how fast does the motor accelerate during closing?
const uint16_t motorMaxSpeed = 50;                  // how fast will the motor go during closing?
const uint16_t motorAccelerationBack = 25;          // how fast does the motor accelerate during opening?
const uint16_t motorMaxSpeedBack = 25;              // how fast will the motor go during opening?
const uint16_t motorAccelerationCalibration = 2000; // how fast does the motor accelerate during calibration?
const uint16_t motorMaxSpeedCalibration = 20;       // how fast will the motor go during calibration?
const uint16_t motorRemainOverstep = 10;             // how many steps will the motor do after the audience let go?
const uint16_t motorRemainTime = 1000;               // how long does the hand remain still aufter being let go?
const uint16_t motorRemainThreshhold = 80;          // from which position on does the motor "remain"?

const uint16_t motorMaxCome = 50;                   // full closed position if the "come here" gesture
const uint16_t motorComeTime = 8000;                // duration of the "come here" gesture (full period)

const uint16_t motorWiggleTime = 200;               // wiggle interval (when hand is closed)
const uint16_t motorWiggleMagnitude = 4;            // wiggle magnitude (when hand is closed)

// touch
const uint16_t touchThreshhold = 700;               // how sensitive is the hand towards touch?
const uint16_t calibrationLength = 80;              // how many intervals to read for calibration?
const uint16_t pauseLength = 500;                   // how many intervals between touch and calibration?
const uint16_t touchLength = 20;                    // how many intervals to read for touch?

const uint32_t touchOpen = 60000;                   // when to open the hand after it's last been closed

// objects ---------------------------------------------------------------------
Hand hand(MICROSTEP, ENABLE, STEP, DIRECTION, TOUCH, SWITCH, LED);

// variables -------------------------------------------------------------------
HandState state = OPENING;

uint32_t timeToCalibration;

uint32_t closeTime;

// setup -----------------------------------------------------------------------
void setup()
{
  // set random seed from floating pin
  randomSeed(analogRead(SEED));

  // set motor parameters
  hand.setMotor(motorFailsafe,
                motorLimit,
                motorMax,
                motorAcceleration,
                motorMaxSpeed,
                motorAccelerationBack,
                motorMaxSpeedBack,
                motorAccelerationCalibration,
                motorMaxSpeedCalibration,
                motorRemainOverstep,
                motorRemainTime,
                motorRemainThreshhold,
                motorMaxCome,
                motorComeTime,
                motorWiggleTime,
                motorWiggleMagnitude);

  // set touch parameters
  hand.setTouch(touchThreshhold,
                calibrationLength,
                pauseLength,
                touchLength);

  // set debug parameters and set states
  hand.initialize(true, true, 100);

  // calibrate mechanically
  hand.calibrate();

  // calibrate electrically
  hand.fill(5000);

  // set the first time when to calibrate again after nobody touched the hand
  timeToCalibration = 120000 + random(60000);
}

// loop ------------------------------------------------------------------------
void loop()
{
  // sense for touch and set the proper states
  if (hand.feel())
  {
    if (state == OPENING)
    {
      hand.close();

      // remember when the hand was last closed
      closeTime = millis();
    }
  }
  else
  {
    if (state == CLOSING)
      hand.open();
  }

  // if the hand was last closed before the touchOpen time, then make it open
  if (millis() > closeTime + touchOpen)
  {
    hand.setReading(false);
  }

  // if the time to calibrate after the last touch has elapsed, then calibrate
  if (millis() > hand.getLastTouched() + timeToCalibration)
  {
    timeToCalibration = 120000 + random(60000);

    hand.calibrate();

    hand.fill(3000);
  }

  // run the hand
  state = hand.run();
}
