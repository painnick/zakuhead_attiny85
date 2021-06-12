#ifndef Scenario_h
#define Scenario_h

#include <Arduino.h>
#include "ServoATtiny85.h"

typedef struct {
    int bright; // 0~255
    int angle; // -90~+90
    int delay; // ms
} Scene;

class Scenario {
public:
  Scenario(int ledPin, int servoPin);
  void attach();
  void playScenes(const Scene*scenes, int count);
  void rorate(int target_angle, int time_spent, int time_step, int bright);
  
  void flash1();
  void scenario1();
  int breathe1(int time_spent, int time_step, int min_bright, int max_bright);
private:
  int _ledPin;
  int _servoPin;
  Servo _servo;
  int _angle;
};

#endif
