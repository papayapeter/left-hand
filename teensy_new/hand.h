#ifndef HAND_H
#define HAND_H

// libraries -------------------------------------------------------------------
#include <Metro.h>
#include <AccelStepper.h>

// class -----------------------------------------------------------------------
class Hand
{
private:
  // pins
  const uint8_t pinMicrostep;
  const uint8_t pinEnable;
  const uint8_t pinStep;
  const uint8_t pinDirection;
  const uint8_t pinSeed;
  const uint8_t pinTouch;
  const uint8_t pinSwitch;
  const uint8_t pinLed;

  // constants to be set before operation
  uint16_t motorFailsafe;         // when to stop moving during calibration
  uint16_t motorLimit;            // what should be the calibrated position
  uint16_t motorAcceleration;     // how fast does the motor accelerate during closing?
  uint16_t motorMaxSpeed;         // how fast will the motor go during closing?
  uint16_t motorAccelerationBack; // how fast does the motor accelerate during opening?
  uint16_t motorMaxSpeedBack;     // how fast will the motor go during opening?
  //uint16_t motorMaxCome;          // full closed position if the "come here" gesture?
  //uint16_t comeTime;              // duration of the "come here" gesture?

  uint16_t touchThreshhold;       // how sensitive is the hand towards touch?
  uint8_t  touchLength;           // how many intervals to read for touch?

  uint8_t  calibrationLength;     // how many intervals to read for calibration?
  uint8_t  pauseLength;           // how many intervals between touch and calibration?

  // objects
  AccelStepper motor;
  Metro        timerRead;

  // variables to be changed during operation
  boolean reverse;                // is opening and closing reversed?

  // working variables
  uint16_t touchCalibration;
  uint16_t touchAverage;
  uint16_t* touchStack;

public:
  /**
   @brief   basic constructor that sets pins

   @params  pins on the teensy
  */
  Hand(uint8_t microstep,
       uint8_t enable,
       uint8_t step,
       uint8_t direction,
       uint8_t seed,
       uint8_t touch,
       uint8_t momSwitch,
       uint8_t led) :
       pinMicrostep(microstep),
       pinEnable(enable),
       pinStep(step),
       pinDirection(direction),
       pinSeed(seed),
       pinTouch(touch),
       pinSwitch(momSwitch),
       pinLed(led),
       motor(AccelStepper::DRIVER, step, direction),
       timerRead(1) { }

 /**
  @brief   sets motor parameters

  @param failsafe         when to stop moving during calibration
  @param limit            what should be the calibrated position
  @param acceleration     how fast does the motor accelerate during closing?
  @param speed            how fast will the motor go during closing?
  @param accelerationBack how fast does the motor accelerate during opening?
  @param speedBack        how fast will the motor go during opening?
 */
  void setMotor(uint16_t failsafe,
                uint16_t limit,
                uint16_t acceleration,
                uint16_t speed,
                uint16_t accelerationBack,
                uint16_t speedBack);

  /**
   @brief   sets touch parameters

   @param threshhold  how sensitive is the hand towards touch?
   @param lengthT     (length touch) how many intervals to read for touch?
   @param lengthC     (length calibration) how many intervals to read for calibration?
   @param lengthP     (length pause) how many intervals between touch and calibration?
   @param time        time between reads in milliseconds
  */
  void setTouch(uint16_t threshhold,
                uint16_t lengthC,
                uint16_t lengthP,
                uint16_t lengthT,
                uint8_t intervalTime);
};

#endif
