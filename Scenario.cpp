#include <Arduino.h>
#include "Scenario.h"

#define NOT_USE_SERIAL

Scenario::Scenario(int ledPin, int servoPin)
  : _ledPin(ledPin), _servoPin(servoPin) {
}

void Scenario::attach() {
  _servo.attach(_servoPin);
  _angle = 0;
  _servo.write(90 + 0);
  delay(1000);
}

void Scenario::playScenes(const Scene*scenes, int count) {
  for(int i = 0; i < count; i ++) {
    const Scene* scene = scenes + i;
    analogWrite(_ledPin, scene->bright);
    if(_angle != scene->angle) {
      _servo.write(90 + scene->angle);
      delay(scene->delay);
    }
  }
}

void Scenario::rorate(int target_angle, int time_total, int time_step, int bright) {

  #ifdef USE_SERIAL
    Serial.print("[rorate] ");
    Serial.print("current_angle=");Serial.print(_angle);Serial.print(", ");
    Serial.print("target_angle=");Serial.print(target_angle);Serial.print(", ");
    Serial.print("time_total=");Serial.print(time_total);Serial.print(", ");
    Serial.print("time_step=");Serial.print(time_step);Serial.print(", ");
    Serial.print("bright=");Serial.println(bright);
  #endif

  Scene scenes[10];
  int change_angle = target_angle - _angle;
  int direction = change_angle > 0 ? 1 : -1;
  int count = (time_total % time_step == 0) ? time_total / time_step : (time_total / time_step) + 1;
  int angle_step = (change_angle * direction) / count;
  
  #ifdef USE_SERIAL
    Serial.print("[rorate] - ");
    Serial.print("change_angle=");Serial.print(change_angle);Serial.print(", ");
    Serial.print("direction=");Serial.print(direction);Serial.print(", ");
    Serial.print("count=");Serial.print(count);Serial.print(", ");
    Serial.print("angle_step=");Serial.println(angle_step);
  #endif

  int next_angle;
  for(int i = 0; i < count; i ++) {
    next_angle = _angle + angle_step * (i + 1) * direction;
    
    #ifdef USE_SERIAL
      Serial.print("[rorate] - ");
      Serial.print("(");Serial.print(i);Serial.print(") ");
      Serial.print("next_angle=");Serial.println(next_angle);
    #endif
    
    scenes[i] = {bright, next_angle, time_step};
  }

  // Return last angle
  _angle = next_angle;
  
  playScenes(scenes, count);
}

void Scenario::flash1() {
  Scene scenes[7];
  scenes[0] = { 63,  _angle,  300};
  scenes[1] = {127,  _angle,  300};
  scenes[2] = {195,  _angle,  300};
  scenes[3] = {255,  _angle,  300};
  scenes[4] = {195,  _angle,  300};
  scenes[5] = {127,  _angle,  300};
  scenes[6] = { 31,  _angle,  500};
  playScenes(scenes, 7);
}

int Scenario::breathe1(int time_total, int time_step, int min_bright, int max_bright) {

  #ifdef USE_SERIAL
    Serial.print("[breathe1] ");
    Serial.print("time_total=");Serial.print(time_total);Serial.print(", ");
    Serial.print("time_step=");Serial.print(time_step);Serial.print(", ");
    Serial.print("min_bright=");Serial.print(min_bright);Serial.print(", ");
    Serial.print("max_bright=");Serial.println(max_bright);
  #endif

  int count = time_total / time_step;
  count += (count % 2) - 1; // Make an odd number

  int bright_step = (max_bright - min_bright) / (count / 2);
  
  #ifdef USE_SERIAL
    Serial.print("[breathe1] - ");
    Serial.print("count=");Serial.print(count);Serial.print(", ");
    Serial.print("bright_step=");Serial.println(bright_step);
  #endif

  for(int i = 0; i < count/ 2; i ++) {
    int bright = min_bright + (bright_step * i);
    
    #ifdef USE_SERIAL
      Serial.print("[breathe1] - ");
      Serial.print("(");Serial.print(i);Serial.print(") ");
      Serial.print("bright=");Serial.println(bright);
    #endif
    
    analogWrite(_ledPin, bright);
    delay(time_step);
  }
    
  #ifdef USE_SERIAL
    Serial.print("[breathe1] - ");
    Serial.print("(");Serial.print((count / 2) % 2 == 0 ? (count / 2) : (count / 2) + 1);Serial.print(") ");
    Serial.print("bright=");Serial.println(max_bright);
  #endif
    
  analogWrite(_ledPin, max_bright);
  delay(time_step);

  for(int i = 1; i < count/ 2 + 1; i ++) {
    int bright = max_bright - bright_step * i;

    #ifdef USE_SERIAL
      Serial.print("[breathe1] - ");
      Serial.print("(");Serial.print((count / 2) +i);Serial.print(") ");
      Serial.print("bright=");Serial.println(bright);
    #endif
    
    analogWrite(_ledPin, bright);
    delay(time_step);
  }
}

void Scenario::scenario1() {
  #ifdef USE_SERIAL
    Serial.println("Start scenario1 =====");
  #endif
  Scene scenes[3];
  
  scenes[0] = { 0,  0,  2000}; // Init.
  playScenes(scenes, 1);

  flash1();
  flash1();
  flash1();

  scenes[1] = {127,  _angle,  500}; // Wait for 500ms
  playScenes(scenes + 1, 1);

  rorate( -45, 2000, 500, 127);
  delay(1000);
  rorate(30, 1200, 400, 127);
  delay(1000);

  flash1();

  scenes[2] = {255,  _angle,  500}; // Wait for 500ms
  playScenes(scenes + 2, 1);

  rorate(0, 1000, 200, 127);
  delay(1000);
  flash1();
  flash1();

  rorate(-45, 1000, 500, 127);
  delay(1000);

  rorate(45, 1000, 500, 255);
  delay(1000);
  flash1();

  rorate(0, 1000, 200, 127);
  delay(1000);
  flash1();
  
  #ifdef USE_SERIAL
    Serial.println("End scenario1 =====");
  #endif
}
