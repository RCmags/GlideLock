//---- Libraries:
#include <PinChangeInterrupt.h>
#include <Servo.h>

//---- Constants:

  //Pins:
const int SENSOR_PIN = A0;
const int MOTOR_PIN = 2;
const int RX_INPUT_PIN = 3;

  //Reciever input:
const int RX_INPUT_DEADBAND = 4;

  //Glide lock:

      //Throttle settings:
const int PWM_IDLE = 1300;
const int PWM_MIN = 1000; 
const int SENSOR_CUTOFF = 700;

      //Delays:
const int DELAY_STARTUP = 5000; 
const int DELAY_IDLE = 500;
const int DELAY_ESC = 250;
const int LED_DELAY = 100;

//---- Variables:
bool val = HIGH;
bool last_case = LOW;
uint32_t time_init = 0;
int pwm_input = 0;
Servo motor;


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
  attachPinChangeInterrupt( digitalPinToPCINT( RX_INPUT_PIN ), checkPwmInput , CHANGE );
  
  pinMode(SENSOR_PIN, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  
  motor.attach(MOTOR_PIN);
  motor.writeMicroseconds( PWM_MIN );
  
  delay( DELAY_STARTUP );

  blinkLed(5);
}

void loop() {  
  //Throttle bellow cutoff - Idling to find sensor
  if( pwm_input < PWM_IDLE ) { 

    //Coming from higher throttle
    if( last_case == HIGH ) { 
      motor.writeMicroseconds( PWM_IDLE );
      last_case = LOW;
      time_init = millis();
    }

    //Delay to slow down motor
    if( last_case == LOW && ( millis() - time_init ) > DELAY_IDLE ) {

      //Motor over sensor
      if( analogRead(SENSOR_PIN) > SENSOR_CUTOFF && val == HIGH ) { 
        val = LOW;
        delay(DELAY_ESC);
      }

      //Turning off motor if over sensor
      if( val == LOW ) { 
        motor.writeMicroseconds( PWM_MIN );
      } else {
        motor.writeMicroseconds( PWM_IDLE );
      
      }
    }
    
  } else { //Throttle beyond cutoff

    //Dissabling glide lock
    motor.writeMicroseconds( pwm_input );
    if( val == LOW ) {
      val = HIGH;
    }

    //Coming from lower throttle
    if( last_case == LOW ) {
      last_case = HIGH;
    }
  }
}
