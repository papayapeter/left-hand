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
                    uint16_t accelerationCalibration,
                    uint16_t speedCalibration,
                    uint16_t remainOverstep,
                    uint16_t remainTime,
                    uint16_t remainThreshhold,
                    uint16_t maxCome,
                    uint16_t comeTime,
                    uint16_t wiggleTime,
                    uint16_t wiggleMagnitude)
{
  // copy variables
  motorFailsafe = failsafe;
  motorLimit = limit;
  motorMax = max;
  motorAcceleration = acceleration;
  motorMaxSpeed = speed;
  motorAccelerationBack = accelerationBack;
  motorMaxSpeedBack = speedBack;
  motorAccelerationCalibration = accelerationCalibration;
  motorMaxSpeedCalibration = speedCalibration;
  motorRemainOverstep = remainOverstep;
  motorRemainTime = remainTime;
  motorRemainThreshhold = remainThreshhold;
  motorMaxCome = maxCome;
  motorComeTime = comeTime;
  motorWiggleTime = wiggleTime;
  motorWiggleMagnitude = wiggleMagnitude;
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
void Hand::initialize(boolean debug,
                      boolean plot,
                      uint16_t debugTime)
{
  // set state and direction to starting values
  direction = UNSET;
  handState = OPENING;

  // setup debug
  timerDebug.interval(debugTime);
  timerDebug.reset();

  debugging = debug;
  plotting = plot;

  // debug
  if (debugging)
    Serial.begin(9600);

  // set up pins
  pinMode(pinSwitch, INPUT_PULLUP);
  pinMode(pinLed, OUTPUT);
}

// fill ------------------------------------------------------------------------
void Hand::fill(uint32_t duration)
{
  if (debugging && !plotting)
    Serial.println("filling touchStack for " + String(duration) + " ms");

  // turn on led
  digitalWrite(pinLed, HIGH);

  // run the motor back to 0
  motor.moveTo(0);

  // fill the touchstack for the set duration
  uint32_t time = millis();
  while (millis() < time + duration)
  {
    if (timerRead.check())
    {
      // read the touch
      uint16_t value = touchRead(pinTouch);
      // shift every read down
      for (uint16_t i = 0; i < calibrationLength + pauseLength + touchLength - 1; i++)
        touchStack[i] = touchStack[i + 1];
      // put new read on top
      touchStack[calibrationLength + pauseLength + touchLength - 1] = value;
    }

    // run motor
    motor.run();
  }

  // turn off led
  digitalWrite(pinLed, LOW);

  // debug
  if (debugging && !plotting)
    Serial.println("touchStack filled");
}

// calibrate -------------------------------------------------------------------
void Hand::calibrate()
{
  // debug
  if (debugging && !plotting)
    Serial.println("calibrating");

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
    if (motor.currentPosition() >= motorFailsafe)
    {
      if (debugging) // debug
        Serial.println("error while calibrating");

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
  if (debugging && !plotting)
    Serial.println("calibration result: " + String(motor.currentPosition()) + " -> " + String(motorLimit));

  // if the switch has been hit, set this position as the new limit
  motor.setCurrentPosition(motorLimit);

  // turn off led
  digitalWrite(pinLed, LOW);

  // set lastTouched to now
  lastTouched = millis();

  // debug
  if (debugging && !plotting)
    Serial.println("calibration done");
}

// setReading ------------------------------------------------------------------
void Hand::setReading(boolean readState)
{
  reading = readState;
}

// feel ------------------------------------------------------------------------
bool Hand::feel()
{
  // if it's time to read
  if (timerRead.check())
  {
    // read the touch
    uint16_t value = touchRead(pinTouch);
    // shift every read down
    for (uint16_t i = 0; i < calibrationLength + pauseLength + touchLength - 1; i++)
      touchStack[i] = touchStack[i + 1];
    // put new read on top
    touchStack[calibrationLength + pauseLength + touchLength - 1] = value;

    // calculate calibration average
    uint32_t touchCalibration = 0;
    for (uint16_t i = 0; i < calibrationLength; i++)
      touchCalibration += touchStack[i];
    touchCalibration /= calibrationLength;

    // calculate touch average
    uint32_t touchAverage = 0;
    for (uint16_t i = calibrationLength + pauseLength; i < calibrationLength + pauseLength + touchLength; i++)
      touchAverage += touchStack[i];
    touchAverage /= touchLength;

    // if it has not been figured out if touching increased or decreases the average
    if (direction == UNSET)
    {
      // figure it out
      if (touchAverage > touchCalibration + touchThreshhold)
      {
        direction = NORMAL;

        reading = true;

        // debug
        if (debugging && !plotting)
          Serial.println("touch delta normal");
      }
      else if (touchAverage < touchCalibration - touchThreshhold)
      {
        direction = REVERSE;

        reading = true;

        // debug
        if (debugging && !plotting)
          Serial.println("touch delta reversed");
      }
    }
    // if it has been figured out
    else
    {
      // return true if touching increases the reading and the reading increased
      if (direction == NORMAL && touchAverage > touchCalibration + touchThreshhold)
        reading = true;
      else if (direction == NORMAL && touchAverage < touchCalibration - touchThreshhold)
        reading = false;
      // return true if touching decreases the reading and the reading decreased
      if (direction == REVERSE && touchAverage < touchCalibration - touchThreshhold)
        reading = true;
      else if (direction == REVERSE && touchAverage > touchCalibration + touchThreshhold)
        reading = false;
    }

    // debug
    if (debugging && plotting && timerDebug.check())
    {
      Serial.print("current: " + String(value));
      Serial.print("\t");
      Serial.print("calibration: " + String(touchCalibration));
      Serial.print("\t");
      Serial.print("average: " + String(touchAverage));
      Serial.print("\t");
      Serial.print("touched: " + String(reading));
      Serial.print("\t");
      Serial.print("target: " + String(motor.targetPosition()));
      Serial.print("\t");
      Serial.print("position: " + String(motor.currentPosition()));
      Serial.println();
    }
  }

  // turn led on if touched, off if not & set lastTouched to now
  if (reading)
  {
    digitalWrite(pinLed, HIGH);

    lastTouched = millis();
  }
  else
    digitalWrite(pinLed, LOW);

  return reading;
}

// run -------------------------------------------------------------------------
HandState Hand::run()
{
  // set motor positions according to states & handle states
  switch (handState)
  {
    case OPENING:
      motor.setAcceleration(motorAccelerationBack);
      motor.setMaxSpeed(motorMaxSpeedBack);

      motor.moveTo(come(motorMaxCome, motorComeTime, millis()));

      break;
    case CLOSING:
      motor.setAcceleration(motorAcceleration);
      motor.setMaxSpeed(motorMaxSpeed);

      motor.moveTo(wiggle(motorWiggleTime, motorWiggleMagnitude, motorMax, -1, millis()));

      break;
    case REMAINING:
      motor.setAcceleration(motorAccelerationBack);
      motor.setMaxSpeed(motorMaxSpeedBack);

      motor.moveTo(min(motorMax, remainPos + motorRemainOverstep));

      if (millis() > remainNow + motorRemainTime)
      {
        handState = OPENING;

        // debug
        if (debugging && !plotting)
          Serial.println("hand opening");
      }

      break;
  }

  // run the motor
  motor.run();

  // calibrate on the fly if necessary
  if (!digitalRead(pinSwitch))
  {
    motor.setCurrentPosition(motorLimit);
  }

  return handState;
}

// close -----------------------------------------------------------------------
void Hand::close()
{
  // set hand state
  handState = CLOSING;

  // debug
  if (debugging && !plotting)
    Serial.println("hand closing");
}

// open ------------------------------------------------------------------------
void Hand::open()
{
  // if the hand is closed farther than the threshhold, then remain, else open right away
  if (motor.currentPosition() >= motorRemainThreshhold)
    handState = REMAINING;
  else
    handState = OPENING;

  // remember time and place when stopped;
  remainNow = millis();
  remainPos = motor.currentPosition();

  // debug
  if (debugging && !plotting)
    Serial.println("hand remaining, then opening");
}

// getLastTouched --------------------------------------------------------------
uint32_t Hand::getLastTouched()
{
  return lastTouched;
}
