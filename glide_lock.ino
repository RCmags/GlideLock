//---- libraries
#include <Servo.h>

//---- constants
#define DELAY_STARTUP     8000         // Delay to arm ESC 
#define DELAY_IDLE        800          // Delay to allow motor to slow down to idle speed
#define DELAY_POSITION    40           // Delay to offset crank relative to sensor

#define PWM_IDLE          1250         // throttle when iddling to find sensor
#define PWM_MIN           1050         // zero throttle. If using a BLheli esc, PWM_MIN < BLHELI_PWM_MIN  

  // analog sensor                                 
#define SENSOR_HIGH       650          // values above threshold are high  // note: SENSOR_HIGH > SENSOR_LOW
#define SENSOR_LOW        600          // values below threshold are low

#define SENSOR_PIN        A0           // must be analog pin if sensor is analog, otherwise use any pin.  
#define INPUT_PIN         3            // receiver input. Digital pin with interrupt
#define MOTOR_PIN         2

//#define USING_DIGITAL_SENSOR           // enable if sensor is digital 

//---- global variables

volatile uint16_t pwm_input = 0;
Servo motor;

//---- PWM input

void checkPwmInput() {
  static uint32_t initial_time = micros();
  if( digitalRead(INPUT_PIN) ) {    // rising 
    initial_time = micros();
  } else {                          // falling
    pwm_input = micros() - initial_time;
  }
}

//---- hall sensor

#ifdef USING_DIGITAL_SENSOR

bool readSensor() {
  return digitalRead(SENSOR_PIN);
}

#else // using analog sensor

bool readSensor() {           
  static bool sensor_state = LOW;
  uint16_t input = analogRead(SENSOR_PIN);
  
  if( input > SENSOR_HIGH ) {
    sensor_state = HIGH;
  } else if ( input < SENSOR_LOW ) { 
    sensor_state = LOW;
  }
  return sensor_state;
}

#endif

//---- main functions:

void setup() {
  // set pins
  pinMode(INPUT_PIN, INPUT_PULLUP);  
  pinMode(SENSOR_PIN, INPUT_PULLUP);
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
