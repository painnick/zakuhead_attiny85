/* ATtiny85 Servo library (5 servos)
   Sawchuk.

  The methods are:

    Servo - Class for manipulating servo motors connected to ATtiny85 pins.

    attach(pin )  - Attaches a servo motor to an i/o pin.
    attach(pin, min, max  ) - Attaches to a pin setting min and max values in microseconds
    default min is 544, max is 2400  
 
    write()     - Sets the servo angle in degrees.  (invalid angle that is valid as pulse in microseconds is treated as microseconds)
    writeMicroseconds() - Sets the servo pulse width in microseconds 
    read()      - Gets the last written servo pulse width as an angle between 0 and 180. 
    readMicroseconds()   - Gets the last written servo pulse width in microseconds. (was read_us() in first release)
    attached()  - Returns true if there is a servo attached. 
    detach()    - Stops an attached servos from pulsing its i/o pin. 
*/
#ifndef ServoATtiny85_h
#define ServoATtiny85_h

#include <Arduino.h>

#define Servo_VERSION           1     // software version of this library

#define MIN_PULSE_WIDTH       544     // the shortest pulse sent to a servo  
#define MAX_PULSE_WIDTH      2400     // the longest pulse sent to a servo 
#define DEFAULT_PULSE_WIDTH  1500     // default pulse width when servo is attached
#define REFRESH_INTERVAL    20000     // minumim time to refresh servos in microseconds 

#define MAX_SERVOS              5     // for ATtiny85

#define INVALID_SERVO         255     // flag indicating an invalid servo index

typedef struct {
  uint8_t pin;               // a pin number from 0 to 4
  bool isActive;             // true if this channel is enabled, pin not pulsed if false 
  volatile uint8_t ticks;    // for compare interrupt, pulse length
  volatile bool pulse;       // HIGH/LOW status of pin
} servo_t;

class Servo
{
public:
  Servo();
  bool attach(int pin);           // attach the given pin to the next free channel, sets pinMode
  bool attach(int pin, int min, int max); // as above but also sets min and max values for writes. 
  void detach();
  void write(int value);             // if value is < 200 its treated as an angle, otherwise as pulse width in microseconds 
  void writeMicroseconds(int value); // Write pulse width in microseconds 
  int read();                        // returns current pulse width as an angle between 0 and 180 degrees
  int readMicroseconds();            // returns current pulse width in microseconds for this servo
  bool attached();                   // return true if this servo is attached, otherwise false 
private:
  uint8_t servoIndex;                // index into the data for this servo
  int min;                           // default is MIN_PULSE_WIDTH    
  int max;                           // default is MAX_PULSE_WIDTH   
};

#endif
