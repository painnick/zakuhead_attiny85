#include  "Scenario.h"

#define LED_PIN   0
#define SERVO_PIN 2

Scenario *scenario;

void setup() {
  scenario = new Scenario(LED_PIN, SERVO_PIN);
  Serial.begin(115200);
  scenario->attach();
}

int loop_index = 0;
int play_step = 5;

void loop() {
  if(loop_index == 0) {
    scenario->scenario1();
    play_step *= 2;
    if(play_step > 100) {
      play_step = 100;
    }
  }
  
  scenario->breathe1(1000, 100, 15, 195);
  delay(500);
  analogWrite(LED_PIN, 127);
  delay(3000);
  
  loop_index = (loop_index + 1) % play_step;
}
