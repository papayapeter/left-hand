#include "hand.h"

// methods ---------------------------------------------------------------------

// setMotor --------------------------------------------------------------------
void Hand::setMotor(uint16_t failsafe,
                    uint16_t limit,
                    uint16_t max,
                    uint16_t acceleration,
                    uint16_t speed,
                    uint16_t accelerationBack,
                    uint16_t speedBack,
                    uint16_t remainOverstep,
                    uint16_t accelerationCalibration,
                    uint16_t speedCalibration)
{
  // copy variables
  motorFailsafe = failsafe;
  motorLimit = limit;
  motorAcceleration = acceleration;
  motorMaxSpeed = speed;
  motorAccelerationBack = accelerationBack;
  motorMaxSpeedBack = speedBack;
  motorAccelerationCalibration = accelerationCalibration;
  motorMaxSpeedCalibration = speedCalibration;
}

// setTouch --------------------------------------------------------------------
void Hand::setTouch(uint16_t threshhold,
                    uint16_t lengthC,
                    uint16_t lengthP,
                    uint16_t lengthT,
                    uint8_t intervalTime)
{
  // copy variables
  touchThreshhold = threshhold;
  touchLength = lengthT;

  calibrationLength = lengthC;
  pauseLength = lengthP;

  // initialize touchStack with proper length
  touchStack = new uint16_t[lengthC + lengthP + lengthT];

  // setup metro timer for read intervals
  timerRead.interval(intervalTime);
  timerRead.reset();
}

// initialize ------------------------------------------------------------------
void Hand::initialize(uint32_t duration,
                      boolean debug,
                      uint16_t debugTime)
{
  // set state and direction to starting values
  direction = UNSET;
  handState = OPEN;

  // setup metro timer for debug intervals
  timerDebug.interval(debugTime);
  timerDebug.reset();

  // debug
  if (debugging)
  {
    Serial.begin(9600);
    Serial.println("filling touchStack for " + String(duration) + " ms");
  }

  // set up an turn on led
  pinMode(pinLed, OUTPUT);
  digitalWrite(pinLed, HIGH);

  // fill the touchstack for the set duration
  uint32_t time = millis();
  while (time + duration < millis())
  {
    if (timerRead.check())
    {
      // shift every read down
      for (uint16_t i = 0; i < calibrationLength + pauseLength + touchLength - 1; i++)
      {
        touchStack[i] = touchStack[i + 1];
      }
      // put new read on top
      touchStack[calibrationLength + pauseLength + touchLength - 1] = touchRead(pinTouch);
    }
  }

  // turn off led
  digitalWrite(pinLed, LOW);

  // debug
  if (debugging)
  {
    Serial.println("touchStack filled");
  }
}

// calibrate -------------------------------------------------------------------
void Hand::calibrate()
{
  // debug
  if (debugging)
  {
    Serial.println("calibrating");
  }

  // turn on led
  digitalWrite(pinLed, HIGH);

  // set new speeds to be used during calibration
  motor.setAcceleration(motorAccelerationCalibration);
  motor.setMaxSpeed(motorMaxSpeedCalibration);

  // move the motor as long as its safe or it hits the switch
  motor.moveTo(motorFailsafe);
  while (digitalRead(pinSwitch))
  {
    motor.run();

    // give error if last safe position is reached but the switch has not been hit
    if (motor.currentPosition() == motorFailsafe)
    {
      if (debugging) // debug
      {
        Serial.println("error while calibrating");
      }

      while (true)
      {
        digitalWrite(pinLed, HIGH);
        delay(500);
        digitalWrite(pinLed, LOW);
        delay(500);
      }
    }
  }

  // debug
  if (debugging)
  {
    Serial.println("calibration result: " + String(motor.currentPosition()) + " -> " + String(motorLimit));
  }

  // if the switch has been hit, set this position as the new limit
  motor.setCurrentPosition(motorLimit);

  // run the motor bach to zero
  while(motor.currentPosition() > 0)
  {
    motor.runToNewPosition(0);
  }

  // turn off led
  digitalWrite(pinLed, LOW);

  // debug
  if (debugging)
  {
    Serial.println("calibration done");
  }
}

// feel ------------------------------------------------------------------------
bool Hand::feel(HandState& state)
{
  // figure out set hand state (expecially if fully open or fully closed)
  if (handState == OPENING && motor.currentPosition() == 0)
  {
    handState = OPEN;
  }
  else if (handState == CLOSING && motor.currentPosition() == motorMax)
  {
    handState = CLOSED;
  }
  // set hand state reference
  state = handState;

  // run the motor
  motor.run();

  // if it's time to read
  if (timerRead.check())
  {
    // read the touch
    uint16_t value = touchRead(pinTouch);
    // shift every read down
    for (uint16_t i = 0; i < calibrationLength + pauseLength + touchLength - 1; i++)
    {
      touchStack[i] = touchStack[i + 1];
    }
    // put new read on top
    touchStack[calibrationLength + pauseLength + touchLength - 1] = value;

    // calculate calibration average
    uint32_t touchCalibration = 0;
    for (uint16_t i = 0; i < calibrationLength; i++)
    {
      touchCalibration += touchStack[i];
    }
    touchCalibration /= calibrationLength;

    // calculate touch average
    uint32_t touchAverage = 0;
    for (uint16_t i = calibrationLength + pauseLength - 1; i < calibrationLength + pauseLength + touchLength; i++)
    {
      touchAverage += touchStack[i];
    }
    touchAverage /= touchLength;

    // debug
    if (debugging && timerDebug.check())
    {
      Serial.print("current: " + String(value));
      Serial.print("\t");
      Serial.print("calibration: " + String(touchCalibration));
      Serial.print("\t");
      Serial.print("average: " + String(touchAverage));
      Serial.println();
    }

    // return false in any other case
    reading = false;
    // if it has not been figured out if touching increased or decreases the average
    if (direction == UNSET)
    {
      // figure it out
      if (touchAverage > touchCalibration + touchThreshhold)
      {
        direction = NORMAL;

        reading = true;
      }
      else if (touchAverage < touchCalibration - touchThreshhold)
      {
        direction = REVERSE;

        reading = true;
      }
    }
    // if it has been figured out
    else if (direction == NORMAL && touchAverage > touchCalibration + touchThreshhold)
    {
      // return true if touching increases the reading and the reading increased
      reading = true;
    }
    // return true if touching decreases the reading and the reading decreased
    else if (direction == REVERSE && touchAverage < touchCalibration - touchThreshhold)
    {
      reading = true;
    }
  }

  if (reading)
  {
    digitalWrite(pinLed, HIGH);
  }
  else
  {
    digitalWrite(pinLed, LOW);
  }

  return reading;
}

// close -----------------------------------------------------------------------
void Hand::close()
{
  // set hand state
  handState = CLOSING;

  // debug
  if (debugging)
  {
    Serial.println("hand closing");
  }

  // setting speed and acceleration for closing
  motor.setAcceleration(motorAcceleration);
  motor.setMaxSpeed(motorMaxSpeed);

  // move motor to maximum
  motor.moveTo(motorMax);
}

// open ------------------------------------------------------------------------
void Hand::open()
{
  // set hand state
  handState = OPENING;

  // debug
  if (debugging)
  {
    Serial.println("hand opening");
  }

  // setting speed and acceleration for closing
  motor.setAcceleration(motorAccelerationBack);
  motor.setMaxSpeed(motorMaxSpeedBack);

  // move motor to 0
  motor.moveTo(0);
}

// stop ------------------------------------------------------------------------
void Hand::stop()
{
  // set hand state
  handState = REMAINING;

  // debug
  if (debugging)
  {
    Serial.println("hand remaining");
  }

  // setting speed and acceleration for remaining
  motor.setAcceleration(motorAccelerationBack);
  motor.setMaxSpeed(motorMaxSpeedBack);

  // move moter a bit further, but not past the maximum position
  motor.moveTo(min(motorMax, motor.currentPosition() + motorRemainOverstep));
}

// comeHere --------------------------------------------------------------------
void Hand::comeHere()
{

}

// wiggle ----------------------------------------------------------------------
void Hand::wiggle()
{

}
