//---- Libraries:
#include <Servo.h>
#include <PinChangeInterrupt.h>

//---- Constants:

  //Pins:
const int SENSOR_PIN = 2;
const int MOTOR_PIN = 4;
const int RX_INPUT_PIN = 3;

  //Reciever input:
const int RX_INPUT_DEADBAND = 4;

  //Glide lock:

      //Throttle settings:
const int PWM_IDLE = 1300;
const int PWM_MIN = 1000; 

      //Delays:
const int DELAY_STARTUP = 5000; 
const int DELAY_IDLE = 2000;
const int DELAY_ESC = 1000;
const int LED_DELAY = 100;

//---- Variables:
int pwm_input = 0;
Servo motor;
bool lock = LOW;
uint32_t time_init = 0;

//---- Functions:

    // PWM Interrupt function:
void checkPwmInput( void ) {
  
  static boolean last_state = 0;
  static int curr_time = 0;
 
    if( digitalRead( RX_INPUT_PIN ) == HIGH && last_state == LOW ) {
      last_state = HIGH;
      curr_time = micros(); 
    }

    if( digitalRead( RX_INPUT_PIN ) == LOW && last_state == HIGH ) {
      
      last_state = LOW;

      //Deadband filter:
      int change_time = micros() - curr_time; 
      int diff = pwm_input - change_time;

      if( diff > RX_INPUT_DEADBAND ) {
        pwm_input = change_time - RX_INPUT_DEADBAND;
      } 
      
      if( diff < -RX_INPUT_DEADBAND) { 
        pwm_input = change_time + RX_INPUT_DEADBAND;
      }    
    }
}

//Hall switch Interrupt function:
void CheckSensor( void ) {
  if( pwm_input < PWM_IDLE && time_init == HIGH ) {
    lock = HIGH;
  }
}

// LED blinking function:
void blinkLed( int n_times ) {
  
  for( int index = 0; index < n_times; index += 1 ) {
    digitalWrite( LED_BUILTIN, HIGH );
    delay( LED_DELAY );
    digitalWrite( LED_BUILTIN, LOW );
    delay( LED_DELAY );
  }
}

//---- Main functions:

void setup() {

  pinMode( RX_INPUT_PIN, INPUT_PULLUP );
  attachInterrupt( digitalPinToInterrupt( RX_INPUT_PIN ), checkPwmInput , CHANGE );
  
  pinMode( SENSOR_PIN, INPUT_PULLUP );
  attachInterrupt( digitalPinToInterrupt( SENSOR_PIN ), CheckSensor , FALLING );
    
  motor.attach(MOTOR_PIN);
  motor.writeMicroseconds( PWM_MIN );
 
  delay( DELAY_STARTUP );

  pinMode( LED_BUILTIN, OUTPUT );
  blinkLed( 5 );
}

void loop() {
  //Throttle bellow cutoff - Idling to find sensor
  if( pwm_input < PWM_IDLE ) {

	// start timer once power decreases
    if( time_init == LOW ) {
      time_init = millis();
    }
 
    if( (millis() - time_init) > DELAY_IDLE ) {
      time_init = HIGH;
    }
    
	// Idle motor is glide lock is off
    if( lock == LOW ) { 
      motor.writeMicroseconds( PWM_IDLE );
    } else {
      delay(DELAY_ESC); 
      motor.writeMicroseconds( PWM_MIN );
    }
  
  } else { //Throttle beyond cutoff
    
	// Dissable glide lock and stop timer
    if( lock == HIGH ) {
      lock = LOW;
    }
    if( time_init == HIGH ) { 
      time_init = LOW;
    }
	// Enable throttle input
    motor.writeMicroseconds( pwm_input ); 
  }
}
