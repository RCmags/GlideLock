/* ORNITHOPTER GLIDE LOCK */

/* Sketch for an arduino NANO that controls the motor of an RC ornithopter. 
 * It commands a the motor to stop once the crank shaft reaches a hall sensor pickup.
 * Author: RCmags https://github.com/RCmags
*/

//=============== Connections ================
// See included schematic
  // Inputs:
// Pin 3  -> Receiver CH3
// Pin A0 -> Hall sensor
  // Outputs:
// Pin 2  -> Motor ESC 

//=================== Code ===================
#include <Servo.h>
#include "parameters.cpp"

//---- constants
#define SENSOR_PIN      A0    // Must be analog pin if sensor is analog, otherwise use any pin.  
#define INPUT_PIN       3     // Receiver input. Digital pin with interrupt
#define MOTOR_PIN       2     // Output Pin for motor ESC. 

#define NUMBER_MEAN     50    // number of readings used for calibration mean 
#define TIME_CALIBRATE  1000  // timer interval over which readings are averaged [microseconds]

//---- global variables

volatile uint16_t pwm_input = 0;
Servo motor;

//---- PWM input

/* interrupt routine to read pwm pulse */
void checkPwmInput() {
  static uint32_t initial_time = micros();
  if( digitalRead(INPUT_PIN) ) {    // rising 
    initial_time = micros();
  } else {                          // falling
    pwm_input = micros() - initial_time;
  }
}

//---- hall sensor

#ifdef USING_ANALOG_SENSOR

/* average of sensor output */
int calibrateSensor() {
  constexpr int DELAY = int( float(TIME_CALIBRATE)/float(NUMBER_MEAN) );
  float sum = 0;
  for( uint8_t index = 0; index < NUMBER_MEAN; index += 1 ) {
    sum += float( analogRead(SENSOR_PIN) );
    delayMicroseconds(DELAY); 
  } 
  sum /= float(NUMBER_MEAN);
  return int(sum);
}

/* set pin and calibrate sensor */
void setupSensor() {
  pinMode(SENSOR_PIN, INPUT_PULLUP);
  readSensor();                                 // Calibrate with first call
}

/* convert analog reading to digital output */
bool readSensor() {
  static int sensor_mean = calibrateSensor();   // Store mean in local scope
  int input = analogRead(SENSOR_PIN) - sensor_mean;  
  return input > SENSOR_HIGH ? HIGH : LOW;
}

#else // using digital sensor

void setupSensor() {
  pinMode(SENSOR_PIN, INPUT_PULLUP);
}

bool readSensor() {
  return !digitalRead(SENSOR_PIN);
}
#endif

//---- main loop

void setup() {
  // set pins
  setupSensor();
  pinMode(INPUT_PIN, INPUT_PULLUP);  
  pinMode(LED_BUILTIN, OUTPUT);

  // pwm interrupt
  attachInterrupt( digitalPinToInterrupt(INPUT_PIN), checkPwmInput, CHANGE );

  // set throttle to zero
  motor.attach(MOTOR_PIN);
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
  if( pwm_input < PWM_IDLE ) {
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
    motor.writeMicroseconds(pwm_input);
    idle_timer = millis();
  }
}
