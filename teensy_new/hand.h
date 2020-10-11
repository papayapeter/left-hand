#ifndef HAND_H
#define HAND_H

// libraries -------------------------------------------------------------------
#include <Metro.h>
#include <AccelStepper.h>

// enums
enum HandState
{
  OPENING,
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
  uint16_t motorAccelerationCalibration; // how fast does the motor accelerate during calibration?
  uint16_t motorMaxSpeedCalibration;     // how fast will the motor go during calibration?
  uint16_t motorRemainOverstep;          // how many steps will the motor do after the audience let go?
  uint16_t motorRemainTime;              // how long does the hand remain still aufter being let go?
  uint16_t motorRemainThreshhold;        // from which position on does the motor "remain"?
  uint16_t motorMaxCome;                 // full closed position if the "come here" gesture
  uint16_t motorComeTime;                // duration of the "come here" gesture (full period)
  uint16_t motorWiggleTime;              // wiggle interval (when hand is closed)
  uint16_t motorWiggleMagnitude;         // wiggle magnitude (when hand is closed)

  uint16_t touchThreshhold;              // how sensitive is the hand towards touch?
  uint16_t touchLength;                  // how many intervals to read for touch?

  uint16_t calibrationLength;            // how many intervals to read for calibration?
  uint16_t pauseLength;                  // how many intervals between touch and calibration?

  boolean  debugging;                    // serial readout enabled?
  boolean  plotting;                     // serial readout made to be plottable?

  // objects
  AccelStepper motor;
  Metro        timerRead;
  Metro        timerDebug;

  // variables to be changed during operation
  Direction direction;                   // is opening and closing reversed?
  HandState handState;                   // state the hand is in (open/.../closing/...)

  // variables
  uint16_t* touchStack;

  uint32_t remainNow;
  uint16_t remainPos;

  uint32_t lastTouched;

  // private methods
  uint16_t interpolate(uint16_t a, uint16_t b, float ratio)
  {
    ratio =  min(1.0f, max(0.0f, ratio));
    return uint16_t((float(b) - float(a)) * ratio + float(a));
  }

  uint16_t smoothStep(uint16_t a, uint16_t b, float ratio)
  {
    return interpolate(a, b, (ratio * ratio) * (3.0f - 2.0f * ratio));
  }

  uint16_t acceleration(uint16_t a, uint16_t b, float ratio)
  {
    return interpolate(a, b, (ratio * ratio));
  }

  uint16_t deceleration(uint16_t a, uint16_t b, float ratio)
  {
    return interpolate(a, b, 1 - (1 - ratio) * (1 - ratio));
  }

  uint16_t come(uint16_t comeMax, uint16_t comeTime, uint32_t time)
  {
    // // linear
    // float ratio = float(millis() % (comeTime + 1)) / float(comeTime / 2);
    // if (ratio <= 1.0f)
    //   return interpolate(0, comeMax, ratio);
    // else
    //   return interpolate(comeMax, 0, ratio - 1.0f);

    // // step
    // if ((millis() % (comeTime + 1)) <= comeTime / 2)
    //   return 0;
    // else
    //   return comeMax;

    // smooth step
    float ratio = float(millis() % (comeTime + 1)) / float(comeTime / 2);
    if (ratio <= 1.0f)
      return smoothStep(0, comeMax, ratio);
    else
      return smoothStep(comeMax, 0, ratio - 1.0f);

    // // acceleration / deceleration
    // float ratio = float(millis() % (comeTime + 1)) / float(comeTime / 2);
    // if (ratio <= 1.0f)
    //   return deceleration(0, comeMax, ratio);
    // else
    //   return acceleration(comeMax, 0, ratio - 1.0f);
  }

  uint16_t wiggle(uint16_t interval, uint16_t magnitude, uint16_t point, int8_t factor, uint32_t time)
  {
    static uint32_t wiggleTime;
    static uint16_t wiggleNow;
    static uint16_t wiggleLast;


    if (time >= wiggleTime)
    {
      wiggleTime = time + interval;

      wiggleLast = wiggleNow;
      wiggleNow = random(0, magnitude);
    }

    return point + factor * interpolate(wiggleLast, wiggleNow, float(time % interval + 1) / float(interval));
  }

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
       timerDebug(1) { }

 /**
  @brief  sets motor parameters

  @call   call once at the beginning

  @param failsafe                 when to stop moving during calibration
  @param limit                    what should be the calibrated position
  @param max                      the maximum position the motor moves to
  @param acceleration             how fast does the motor accelerate during closing?
  @param speed                    how fast will the motor go during closing?
  @param accelerationBack         how fast does the motor accelerate during opening?
  @param speedBack                how fast will the motor go during opening?
  @param accelerationCalibration  how fast does the motor accelerate during calibration?
  @param speedCalibration         how fast will the motor go during calibration?
  @param remainOverstep           how many steps will the motor do after the audience let go?
  @param remainTime               how long does the hand remain still aufter being let go?
  @param remainThreshhold         // from which position on does the motor "remain"?
  @param maxCome                  full closed position if the "come here" gesture?
  @param comeTime                 duration of the "come here" gesture?
  @param wiggleTime               wiggle interval (when hand is closed)
  @param wiggleMagnitude          wiggle magnitude (when hand is closed)
 */
  void setMotor(uint16_t failsafe,
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
                uint16_t wiggleMagnitude);

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
                  boolean plot = false,
                  uint16_t debugTime = 100);

  /**
   @brief  calibrates the hardware with the help of the momentary switch

   @call   call once for every calibration
  */
  void calibrate();

  /**
   @brief  reads for touch at the pre set intervals

   @call   call every loop

   @return  true if touched, false if not
  */
  bool feel();

  /**
   @brief  does all the movement and state handling

   @call   call every loop

   @return  the state of the hand(open/closed/...)
  */
  HandState run();

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
   @brief   returns the time of the last touch

   @return  returns lastTouched
  */
  uint32_t getLastTouched();
};

#endif
