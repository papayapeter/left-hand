#ifndef HAND_H
#define HAND_H

// libraries -------------------------------------------------------------------
#include <Metro.h>
#include <AccelStepper.h>

// enums
enum HandState
{
  OPEN = 0,
  OPENING = 1,
  CLOSED = 2,
  CLOSING = 3,
  REMAINING = 4
};

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
  int8_t reverse;                 // is opening and closing reversed?
  HandState handState;

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
  @brief  sets motor parameters

  @call   call once at the beginning

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
   @brief  sets touch parameters

   @call   call once at the beginning

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
                uint8_t intervalTime = 1);

  /**
   @brief  initializes the readings (fills the touch value array/touch stack)

   @call   call once at the beginning

   @param duration  for how long to fill/initialize
  */
  void initialize(uint16_t duration);

  /**
   @brief  calibrates the hardware with the help of the momentary switch

   @call   call once for every calibration
  */
  void calibrate();

  /**
   @brief  reads for touch at the pre set intervals

   @call   call every loop

   @param state   reference to a variable which will be set to the state (open/closed/...)
  */
  bool feel(HandState& state);

  /**
   @brief  closes the hand

   @call   call every loop while closing
  */
  void close();

  /**
   @brief  opens the hand

   @call   call every loop while opening
  */
  void open();

  /**
   @brief  moves the hand slighty in a "come here" sort of way when open

   @call   every loop while open
  */
  void comeHere();

  /**
   @brief   wiggles the hand when its closed

   @call   every loop while closed fully
  */
  void wiggle();
};

#endif
