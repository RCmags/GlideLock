/* ORNITHOPTER GLIDE LOCK */

/* Sketch for an ATTiny85 that controls the motor of an RC ornithopter. 
 * It commands a the motor to stop once the crank shaft reaches a hall sensor pickup.
 * Author: RCmags https://github.com/RCmags
 * 
 * NOTE: Code is intended to be used with ATTinyCore
*/

//=============== Connections ================
// See included schematic
  // Inputs:
// Pin 2 -> Receiver CH3
// The sensor pin depends on the type of sensor:
/* Pin 4 -> Analog Hall sensor 
 * Pin 0 -> Digital Hall sensor */
  // Outputs:
// Pin 3 -> Motor ESC 

//=================== Code ===================
#include <Servo_ATTinyCore.h>
#include "parameters.cpp"

//---- constants
#define NUMBER_MEAN     50    // number of readings used for calibration mean 
#define TIME_CALIBRATE  1000  // timer interval over which readings are averaged [microseconds]

//---- global variables

volatile uint16_t pwm_input = 0;
Servo motor;

//----- Input signal
// PORTB = {0 .. 5} -> using pin {2} = B00000100

/* port change interrupt to read PWM input from receiver */
ISR( PCINT0_vect ) {
  static uint32_t initial_time = 0; 
  static uint8_t port_last = PINB;  
  
  // port changes
  uint32_t current_time = micros(); 
  uint8_t port_rise = ~port_last & PINB;
  uint8_t port_fall = port_last & ~PINB;
  
  // check changing pin
  constexpr uint8_t mask = 1 << PCINT2; 
  if( port_rise & mask ) {                
      initial_time = current_time;
  } else if ( port_fall & mask ) {       
      pwm_input = current_time - initial_time;
  }
  port_last = PINB;    
}

void setupISR() {
  // enable pin change interrupts
  GIMSK |= (1 << PCIE);     
  // set pin as PCINT
  PCMSK |= (1 << PCINT2);   
  // set as input
  pinMode(PB2, INPUT_PULLUP);    
}

//---- input filter

/* apply deadband to pwm signal to reduce noise */
int filterInput() {
  static int last_input = 0;
  int change = int(pwm_input) - last_input;
  last_input = change >  INPUT_CHANGE ? pwm_input - INPUT_CHANGE :
               change < -INPUT_CHANGE ? pwm_input + INPUT_CHANGE : last_input;
  return last_input;
}

//---- hall sensor

#ifdef USING_ANALOG_SENSOR

  // Pin mapping: PB4 -> Analog 2 
#define ANALOG_PIN    2

/* average of sensor output */
int calibrateSensor() {
  constexpr int DELAY = int( float(TIME_CALIBRATE)/float(NUMBER_MEAN) );
  float sum = 0;
  for( uint8_t index = 0; index < NUMBER_MEAN; index += 1 ) {
    sum += float( analogRead(ANALOG_PIN) );
    delayMicroseconds(DELAY); 
  } 
  sum /= float(NUMBER_MEAN);
  return int(sum);
}

/* set pin and calibrate sensor */
void setupSensor() {
  pinMode(PB4, INPUT_PULLUP);
  readSensor();                                 // Calibrate with first call
}

/* convert analog reading to digital output */
bool readSensor() {
  static int sensor_mean = calibrateSensor();   // Store mean in local scope
  int input = analogRead(ANALOG_PIN) - sensor_mean;      
  return input > SENSOR_HIGH ? HIGH : LOW;
}

#else // Digital sensor. Uses different pin for stability

bool readSensor() {
  return !digitalRead(PB0);
}

void setupSensor() {
  pinMode(PB0, INPUT_PULLUP);  
}
#endif

//---- main loop

void setup() {
  // set pins
  setupISR();             // rx signal
  setupSensor();          // sensor
  pinMode(PB3, OUTPUT);   // motor
  pinMode(LED_BUILTIN, OUTPUT);

  // set throttle to zero
  motor.attach(PB3);
  motor.writeMicroseconds(PWM_MIN);
  delay(DELAY_STARTUP);
  
  // blink led - startup warning
  for( int index = 0; index < 5; index += 1 ) {
    digitalWrite( LED_BUILTIN, HIGH );
    delay(100);
    digitalWrite( LED_BUILTIN, LOW );
    delay(100);
  }
}

void loop() {  
  // delay timer
  static uint32_t idle_timer = millis();
  
  // Select mode
  int input = filterInput();
  
  if( input < PWM_IDLE ) {
    // idle motor
    uint32_t dt = millis() - idle_timer;
    if( dt < DELAY_IDLE ) {
      motor.writeMicroseconds(PWM_IDLE);
    } else {
      // stop crank above sensor
      if( readSensor() ) {  
        delay(DELAY_POSITION);
        motor.writeMicroseconds(PWM_MIN);
      }
    }     
  // throttle beyond cutoff
  } else { 
    motor.writeMicroseconds(input);
    idle_timer = millis();
  }
}

// Note: Not all pin selections result in the desired output, hence the pin switching.
