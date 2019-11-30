# ArduinoAdvent

Code for Arduino powered advent calendar that automatically opens doors every advent day using micro-servos. Code contains extra comments as explaination to someone not into coding.

 Used in combination with:
 * 24x SG90 Micro-Servo
 * 2x PCA9685 16 Channel I2C PWM Servo Driver
 * 1x TinyRTC I2C
 * 1x Arduino Nano (clone)
 * 1x USB power supply
 (* 1x Potentiometer)

 Details:
 - Assumes all doors are closed manually before advent period.
 - Implements manual mode using serial interface and potentiometer to test door operation.
 - Power consumption measured at roughly 0.01-0.05A idle. (Not implemented Arduino sleep modes)
 - Power consumption when opening door around 0.1A for a brief period.
 - Maximum 1 servo operational at a time to limit current draw.
 - All servo's disabled when idle.
