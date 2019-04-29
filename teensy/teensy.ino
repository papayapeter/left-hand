// *** Zeno Gries - 2018
// *** Die Hände R1.2

#include <Metro.h>
#include <AccelStepper.h>

// Pins
const int Microstep = 7;
const int Enable = 8;
const int Step = 9;
const int Direction = 10;
const int RandomSeed = 14;
const int Touch = 15;
const int Switch = 16;
const int LED = 13;

// Konstanten (zu ändern)
const bool Reverse = false;           // ist öffnen und schließen verdreht?

const int TouchThreshhold = 200;      // Wie fein reagiert die Hand auf Berührung?
const int MotorMin = 0;               // An welcher Stelle steht der Motor, wenn die Hand voll auf ist?
const int MotorMax = 180;             // An welcher Stelle steht der Motor, wenn die Hand voll zu ist?
const int MotorLimit = 185;           // Das harte Maximum
const int MotorFailsafe = 350;        // Wann schaltet sich das gerät ab?
const int MotorAcceleration = 50;     // Wie schnell Beschleunigt der Motor, wenn die Hand zu geht?
const int MotorMaxSpeed = 50;         // Wie schnell ist der Motor maximal, wenn die Hand zu geht?
const int MotorAccelerationBack = 25; // Wie schnell Beschleunigt der Motor, wenn die Hand auf geht?
const int MotorMaxSpeedBack = 25;     // Wie schnell ist der Motor maximal, wenn die Hand auf geht?
const int TouchLength = 20;           // Wie viele Intervalle werden für die Berührung gelesen?
const int CalibrationLength = 80;     // Wie viele Intervalle werden für die Kalibrierung gelesen?
const int PauseLength = 500;          // Wie viele Intervalle sind zwischen Berührung und Kalibrierung
const int WiggleTime = 200;           // Intervall für das Zucken
const int WiggleMagnitude = 3;        // Stärke des Zuckens
const int RemainTime = 500;           // Wie lange verharrt die Hand geschlossen?

const int ComeMotorMin = 0;           // Voll auf Stellung der Komm her Bewegung
const int ComeMotorMax = 40;          // Voll zu Stellung der Komm her Bewegung //***1: 50 2: 40 3: 60
const int ComeTime = 4000;            // Dauer der Komm her Bewegung

// Arbeits-Variablen
int TouchCalibration = 0;
int TouchAverage = 0;
int TouchStack[TouchLength + CalibrationLength + PauseLength];
unsigned long RemainNow;

int MotorTarget = -1;

unsigned long ComeNow;
int ComeMotorTarget = -1;

int CurrentWiggle = 0;
int LastWiggle = 0;
unsigned long WiggleNow = 0;

unsigned long LastTouched = 0;
int MoveSinceTouched;

// Objekte
AccelStepper Motor(1, Step, Direction);

Metro TimerPrint(10);
Metro TimerRead(1);
Metro TimerWiggle(WiggleTime);
Metro ComeTimer(ComeTime);

// Interpolationsfunktion
int interpolate(int a, int b, float ratio)
{
  ratio =  min(1.0f, max(0.0f, ratio));
  return int((float(b) - float(a)) * ratio + float(a));
}

void setup()
{
  // Pins und Kommunikation aufbauen
  Serial.begin(9600);

  pinMode(Microstep, OUTPUT);
  pinMode(Enable, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(Switch, INPUT_PULLUP);
  pinMode(RandomSeed, INPUT);

  digitalWrite(Enable, LOW);
  digitalWrite(Microstep, LOW);

  // TouchStack einmal füllen
  digitalWrite(LED, HIGH);

  // Den Intervall zur Hardwarekalibrierung zufällig festlegen
  randomSeed(analogRead(RandomSeed));
  MoveSinceTouched = 90000 + random(30000);

  while (millis() < 5 * 1000)
  {
    for (int i = 0; i < TouchLength + CalibrationLength + PauseLength; i++)
    {
      while (!TimerRead.check()) { }
      TouchStack[i] = touchRead(Touch);
    }
  }

  // Hardwarekalibireirung
  Motor.setAcceleration(2000);
  Motor.setMaxSpeed(20);

  Motor.moveTo(MotorFailsafe);
  while(digitalRead(Switch))
  {
    Motor.run();
    if (Motor.currentPosition() == MotorFailsafe)
    {
      while(true)
      {
        digitalWrite(LED, HIGH);
        delay(500);
        digitalWrite(LED, LOW);
        delay(500);
      }
    }
  }
  Motor.setCurrentPosition(MotorLimit);

  while(Motor.currentPosition() > 0)
  {
    Motor.runToNewPosition(0);
  }

  digitalWrite(LED, LOW);

  LastTouched = millis();
}

void loop()
{
  // Variation ausrechnen
  if (TimerWiggle.check())
  {
    LastWiggle = CurrentWiggle;

    CurrentWiggle = random(-WiggleMagnitude, WiggleMagnitude + 1);

    WiggleNow = millis();
  }

  // Hand Hardwarekalibrieren
  if (millis() > LastTouched + MoveSinceTouched)
  {
    MoveSinceTouched = 90000 + random(30000);

    Motor.setAcceleration(2000);
    Motor.setMaxSpeed(20);

    Motor.moveTo(MotorFailsafe);
    while(digitalRead(Switch))
    {
      Motor.run();
      if (Motor.currentPosition() == MotorFailsafe)
      {
        while(true)
        {
          digitalWrite(LED, HIGH);
          delay(500);
          digitalWrite(LED, LOW);
          delay(500);
        }
      }
    }
    Motor.setCurrentPosition(MotorLimit);

    while(Motor.currentPosition() > 0)
    {
      Motor.runToNewPosition(0);
    }

    LastTouched = millis();
    MotorTarget = -1;
  }

  if (TimerRead.check())
  {
    // Auslesen
    int Value = touchRead(Touch);

    // Array nach unten verschieben und neuen Wert einsetzten
    for (int i = 0; i < TouchLength + CalibrationLength + PauseLength - 1; i ++)
    {
      TouchStack[i] = TouchStack[i + 1];
    }
    TouchStack[TouchLength + CalibrationLength + PauseLength - 1] = Value;

    // Calibration (erster Teil des Stacks) berechnen
    TouchCalibration = 0;
    for (int i = 0; i < CalibrationLength; i++)
    {
      TouchCalibration += TouchStack[i];
    }
    TouchCalibration /= CalibrationLength;

    // Average berechnen
    TouchAverage = 0;
    for (int i = CalibrationLength + PauseLength; i < TouchLength + CalibrationLength + PauseLength; i++)
    {
      TouchAverage += TouchStack[i];
    }
    TouchAverage /= TouchLength;



    // Feststellen ob der die Hand greifen oder loslassen soll
    // Hand loslassen lassen
    if (!Reverse)
    {
      if (TouchAverage < TouchCalibration - TouchThreshhold)
      {
        LastTouched = millis();
        if (Motor.currentPosition() >= MotorMax - 10)
        {
          digitalWrite(LED, LOW);
          MotorTarget = 0;
          RemainNow = millis();
        }
        else
        {
          digitalWrite(LED, LOW);
          MotorTarget = -1;
        }
      }
      // Hand greifen lassen
      else if (TouchAverage > TouchCalibration + TouchThreshhold)
      {
        digitalWrite(LED, HIGH);
        MotorTarget = 1;
      }
      // Hand verharren lassen
      else if (millis() > RemainNow + RemainTime && MotorTarget == 0)
      {
        MotorTarget = -1;
      }
    }
    else
    {
      if (TouchAverage > TouchCalibration + TouchThreshhold)
      {
        LastTouched = millis();
        if (Motor.currentPosition() >= MotorMax - 10)
        {
          digitalWrite(LED, LOW);
          MotorTarget = 0;
          RemainNow = millis();
        }
        else
        {
          digitalWrite(LED, LOW);
          MotorTarget = -1;
        }
      }
      // Hand greifen lassen
      else if (TouchAverage < TouchCalibration - TouchThreshhold)
      {
        digitalWrite(LED, HIGH);
        MotorTarget = 1;
      }
      // Hand verharren lassen
      else if (millis() > RemainNow + RemainTime && MotorTarget == 0)
      {
        MotorTarget = -1;
      }
    }

    // Komm her Richtung ändern
    if (ComeTimer.check())
    {
      ComeNow = millis();
      ComeMotorTarget *= -1;
    }

    // Motor bewegen (+Variation)
    if (MotorTarget == -1)
    {
      Motor.setAcceleration(MotorAccelerationBack);
      Motor.setMaxSpeed(MotorMaxSpeedBack);

      if (ComeMotorTarget == 1)
      {
        Motor.moveTo(min(ComeMotorMax, interpolate(ComeMotorMin, ComeMotorMax + 30, float(millis() - ComeNow) / float(ComeTime))));
      }
      else if (ComeMotorTarget == -1)
      {
        Motor.moveTo(max(ComeMotorMin, interpolate(ComeMotorMax, ComeMotorMin - 30, float(millis() - ComeNow) / float(ComeTime))));
      }
    }
    else
    {
      Motor.setAcceleration(MotorAcceleration);
      Motor.setMaxSpeed(MotorMaxSpeed);

      Motor.moveTo(MotorMax + interpolate(LastWiggle, CurrentWiggle, float(millis() - WiggleNow) / float(WiggleTime)));
    }

    // Serielle Aufzeichnung
    if (TimerPrint.check())
    {
      Serial.print("Value: " + String(Value));
      Serial.print("\t");
      Serial.print("Calibration: " + String(TouchCalibration));
      Serial.print("\t");
      Serial.print("Average: " + String(TouchAverage));
      Serial.print("\t");
      /*Serial.print("Touch?: " + String(MotorTarget));
      Serial.print("\t");
      Serial.print("CalibrationI: " + String(MoveSinceTouched));
      Serial.print("\t");
      Serial.print("CalibrationLT: " + String(LastTouched));
      Serial.print("\t");
      Serial.print("CalibrationT: " + String(millis() - LastTouched));*/
      Serial.println();
    }
  }

  Motor.run();

  if (!digitalRead(Switch))
  {
    Motor.setCurrentPosition(185);
  }
}
