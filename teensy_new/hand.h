#ifndef HAND_H
#define HAND_H

// libraries -------------------------------------------------------------------
#include <Metro.h>
#include <AccelStepper.h>

// enums
enum HandState
{
  OPEN,
  OPENING,
  CLOSED,
  CLOSING,
  REMAINING
};

enum Direction
{
  REVERSE,
  UNSET,
  NORMAL
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
  uint16_t motorFailsafe;                // when to stop moving during calibration
  uint16_t motorLimit;                   // what should be the calibrated position
  uint16_t motorMax;                     // what should be the maximum position (always less than the calibrated position)
  uint16_t motorAcceleration;            // how fast does the motor accelerate during closing?
  uint16_t motorMaxSpeed;                // how fast will the motor go during closing?
  uint16_t motorAccelerationBack;        // how fast does the motor accelerate during opening?
  uint16_t motorMaxSpeedBack;            // how fast will the motor go during opening?
  uint16_t motorRemainOverstep;          // how many steps will the motor do after the audience let go?
  uint16_t motorAccelerationCalibration; // how fast does the motor accelerate during closing?
  uint16_t motorMaxSpeedCalibration;     // how fast will the motor go during closing?
  //uint16_t motorMaxCome;                 // full closed position if the "come here" gesture?
  //uint16_t comeTime;                     // duration of the "come here" gesture?

  uint16_t touchThreshhold;              // how sensitive is the hand towards touch?
  uint16_t touchLength;                  // how many intervals to read for touch?

  uint16_t calibrationLength;            // how many intervals to read for calibration?
  uint16_t pauseLength;                  // how many intervals between touch and calibration?

  boolean  debugging;                    // serial readout enabled?

  // objects
  AccelStepper motor;
  Metro        timerRead;
  Metro        timerDebug;

  // variables to be changed during operation
  Direction direction;                   // is opening and closing reversed?
  HandState handState;                   // state the hand is in (open/.../closing/...)

  // working variables
  uint16_t* touchStack;
  boolean   reading;

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
       timerRead(1),
       timerDebug(100) { }

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
                uint16_t max,
                uint16_t acceleration,
                uint16_t speed,
                uint16_t accelerationBack,
                uint16_t speedBack,
                uint16_t remainOverstep,
                uint16_t accelerationCalibration,
                uint16_t speedCalibration);

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
  void initialize(uint32_t duration,
                  boolean debug = false,
                  uint16_t debugTime = 100);

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

   @call   call once to start closing
  */
  void close();

  /**
   @brief  opens the hand

   @call   call once to start opening
  */
  void open();

  /**
   @brief  halts the movement

   @call   call once to halt
  */
  void stop();

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
