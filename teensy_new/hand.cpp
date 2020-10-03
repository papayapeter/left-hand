#include "hand.h"

// methods ---------------------------------------------------------------------
void Hand::setMotor(uint16_t failsafe,
                    uint16_t limit,
                    uint16_t acceleration,
                    uint16_t speed,
                    uint16_t accelerationBack,
                    uint16_t speedBack)
{
  motorFailsafe = failsafe;
  motorLimit = limit;
  motorAcceleration = acceleration;
  motorMaxSpeed = speed;
  motorAccelerationBack = accelerationBack;
  motorMaxSpeedBack = speedBack;
}

void Hand::setTouch(uint16_t threshhold,
                    uint8_t lengthT,
                    uint8_t lengthC,
                    uint8_t lengthP,
                    uint8_t time)
{
  touchThreshhold = threshhold;
  touchLength = lengthT;

  calibrationLength = lengthC;
  pauseLength = lengthP;

  timerRead.interval(time);
  timerRead.reset();
}
