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
                    uint16_t lengthC,
                    uint16_t lengthP,
                    uint16_t lengthT,
                    uint8_t intervalTime)
{
  touchThreshhold = threshhold;
  touchLength = lengthT;

  calibrationLength = lengthC;
  pauseLength = lengthP;

  touchStack = new uint16_t[lengthC + lengthP + lengthT];
  touchCalibration = 0;
  touchAverage = 0;

  timerRead.interval(intervalTime);
  timerRead.reset();
}

void Hand::initialize(uint16_t duration)
{
  reverse = 0;
  handState = OPEN;
}

void Hand::calibrate()
{

}

bool Hand::feel(HandState& state)
{
  return false;
}

void Hand::close()
{

}

void Hand::open()
{

}

void Hand::comeHere()
{

}

void Hand::wiggle()
{

}
