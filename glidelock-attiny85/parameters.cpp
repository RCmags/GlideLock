/*      CONSTANT     |    VALUE   |  UNIT  |   DESCRIPTION */
/*=========================================================*/
// 1. Delays
#define DELAY_STARTUP     8000    // ms     // Delay to arm ESC. Must be large enough to ensure startup. 
#define DELAY_IDLE        800     // ms     // Delay to allow motor to slow down to idle speed. Must be large enough to ensure decceleration.
#define DELAY_POSITION    40      // ms     // Delay to offset crank relative to sensor. A larger value causes a larger overshoot. 

// 2. PWM signal 
#define PWM_IDLE          1250    // us     // Throttle when iddling to find sensor. A larger values causes the crank to overshoot the sensor.
#define PWM_MIN           1050    // us     // Zero throttle. If using a BLheli esc, PWM_MIN < BLHELI_PWM_MIN  

// 3. Input deadband filter
#define INPUT_CHANGE      12      // us     // Change in PWM signal needed to update receiver inputs

// 4. Hall sensor
//#define USING_ANALOG_SENSOR                 // remove comment if using an analog hall sensor                                  
#define SENSOR_HIGH       100               // Values above threshold are high. Reading is relative to mean sensor output.   
