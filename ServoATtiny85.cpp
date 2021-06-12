/*
 ServoATtiny85.cpp - Interrupt driven Servo library for ATTiny85 using 8 bit timer1 - Version 1
 Sawchuk.
 */
#include <Arduino.h>
#include "ServoATtiny85.h"

// This code prefers a 16MHz clock (don't use 1MHz)
#define usToTicks(_us)    (( clockCyclesPerMicrosecond() * _us) / 256)     // converts microseconds to tick (assumes prescale of 256)
#define ticksToUs(_ticks) (( (unsigned)_ticks * 256) / clockCyclesPerMicrosecond() ) // converts from ticks back to microseconds

#define TRIM_DURATION       2                               // compensation ticks to trim adjust for digitalWrite delays

static servo_t servos[MAX_SERVOS];                          // static array of servo structures
static volatile uint8_t curServo;                           // index to the servo being pulsed
static uint8_t ServoCount;                                  // the total number of attached servos
static volatile uint16_t OCR1Ctotal;                        // cumulative HIGH pulse length in ticks
static volatile uint16_t Refresh_Delay_Counter;             // interrupt count down timer

#define SERVO_MIN() (this->min)  // minimum value in uS for this servo
#define SERVO_MAX() (this->max)  // maximum value in uS for this servo

/************ static functions common to all instances ***********************/

static uint8_t getServo(uint8_t start) {
  for (uint8_t i = start; i < MAX_SERVOS; i++) {
    if (servos[i].isActive) return i;
  }
  return INVALID_SERVO;
}

static void servo_timer()   //set timer1 interrupt
/*
 * when we enter this, servo[curServo].pin has been set HIGH and we must keep it high servo[curServo].ticks
 * when that's done, go to next servo and do the same
 * then wait for refresh
 */
{
  TCCR1 = 0; 
  TCNT1 = 0;  
  if (servos[curServo].pulse == HIGH) { // we must stay high for an interrupt
    OCR1C = servos[curServo].ticks; // how long to stay high
    OCR1Ctotal += OCR1C;
  }
  else { // pulse is LOW
    // if another servo, start its pulse, else begin wait for refresh
    if ((curServo = getServo(curServo+1)) != INVALID_SERVO) {
      digitalWrite(servos[curServo].pin,HIGH);
      servos[curServo].pulse = HIGH;
      OCR1C = servos[curServo].ticks; // how long to stay high
      OCR1Ctotal += OCR1C;
    }
    else { // must wait until refresh interval is complete
      Refresh_Delay_Counter = (usToTicks(20000)-OCR1Ctotal) / 24; // this will count down, represents # interrupts with OCR1C = 24, to reach a 20ms pulse width
      OCR1C = 24;
      OCR1Ctotal = 0;
    }
  }
  // interrupt COMPA
  OCR1A = OCR1C;
  // turn on CTC mode
  TCCR1 |= (1 << CTC1);
  // Set CS13 and CS10 bits for 256 prescaler
  TCCR1 |= (1 << CS13)|(1 << CS10);  
  TIMSK |= _BV(OCIE1A);  // enable the output compare interrupt
}

ISR(TIMER1_COMPA_vect)
{
  if (curServo != INVALID_SERVO && servos[curServo].pulse == HIGH) {     // we have been HIGH for the last 544 - 2400 microseconds
    digitalWrite(servos[curServo].pin,LOW); // so, go LOW
    servos[curServo].pulse = LOW;
    servo_timer();
  }
  else { // we are waiting until 20ms has elapsed (servos use a 50 HZ (20 millisecond) waveform)
    if (--Refresh_Delay_Counter <= 0) { // 20ms has elapsed when countdown expires
      if ((curServo = getServo(0)) != INVALID_SERVO) { // restart servo HIGH pulse
        digitalWrite(servos[curServo].pin,HIGH); // start the ON cycle again (begin a new waveform on HIGH)
        servos[curServo].pulse = HIGH;
        servo_timer();
      }   
    } 
  }
}

static void initISR()
{
  if ((curServo = getServo(0)) != INVALID_SERVO)
  {
    digitalWrite(servos[curServo].pin,HIGH);
    servos[curServo].pulse = HIGH;
    cli();
    servo_timer();
    sei();
  }
}

static void finISR()
{
    //disable use of the timer1 compare
    TIMSK &= ~_BV(OCIE1A) ;  // disable timer1 output compare interrupt
    //timerDetach(TIMER1OUTCOMPAREA_INT);
}

static boolean isTimerActive()
{
  for (int i = 0; i < MAX_SERVOS; i++) {
    if (servos[i].isActive) {
      return true;
    }
  }
  return false;
}


/****************** end of static functions ******************************/

Servo::Servo()
{
  if (ServoCount < MAX_SERVOS)
    this->servoIndex = ServoCount++;    // assign a servo index to this instance
  else
    this->servoIndex = INVALID_SERVO ;  // too many servos
}

bool Servo::attach(int pin)
{
  return this->attach(pin, MIN_PULSE_WIDTH, MAX_PULSE_WIDTH);
}

bool Servo::attach(int pin, int min, int max)
{
  if (this->servoIndex < MAX_SERVOS)  // ensure channel is valid
  {
    pinMode( pin, OUTPUT) ;                                   // set servo pin to output
    digitalWrite(pin,LOW);
    servos[this->servoIndex].pin = pin;
    if (min > 300 && min < 3000) this->min  = min;
    if (max > min && max < 3000) this->max  = max;
    servos[this->servoIndex].ticks = usToTicks(DEFAULT_PULSE_WIDTH);   // store default values
    servos[this->servoIndex].pulse = LOW;
    servos[this->servoIndex].isActive = true; 
    initISR();
    return true;
  }
  return false;
}

void Servo::detach()
{
  if (this->servoIndex < MAX_SERVOS)   // ensure channel is valid
  {
    servos[this->servoIndex].isActive = false;
    if(!isTimerActive()) finISR();
  }
}

void Servo::write(int value)
{
  if (this->servoIndex < MAX_SERVOS)   // ensure channel is valid
  {
    if (servos[this->servoIndex].isActive)
    {
      if (value < MIN_PULSE_WIDTH)
      {  // treat values less than 544 as angles in degrees (valid values in microseconds are handled as microseconds)
        if(value < 0) value = 0;
        if(value > 180) value = 180;
        value = map(value, 0, 180, SERVO_MIN(),  SERVO_MAX());
      }
      this->writeMicroseconds(value);
    }
  }
}

void Servo::writeMicroseconds(int value)
{
  if (this->servoIndex < MAX_SERVOS)   // ensure channel is valid
  {
    if (servos[this->servoIndex].isActive)
    {
    // calculate and store the values for the given channel
      if ( value < SERVO_MIN() )          // ensure pulse width is valid
        value = SERVO_MIN();
      else if ( value > SERVO_MAX() )
        value = SERVO_MAX();
  
      value = value - TRIM_DURATION;
      value = usToTicks(value);  // convert to ticks after compensating for interrupt overhead
  
      servos[this->servoIndex].ticks = value;
    }
  }
}

int Servo::read() // return the value as degrees
{
  return  map( this->readMicroseconds()+1, SERVO_MIN(), SERVO_MAX(), 0, 180);
}

int Servo::readMicroseconds()
{
  unsigned int pulsewidth;
  if( this->servoIndex != INVALID_SERVO )
    pulsewidth = ticksToUs(servos[this->servoIndex].ticks)  + TRIM_DURATION ; 
  else
    pulsewidth  = 0;

  return pulsewidth;
}

bool Servo::attached()
{
  return servos[this->servoIndex].isActive;
}


