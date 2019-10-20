// Code from other people we want to use
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include "RTClib.h"

// Constants (Stuff that never changes)
const int door_open_speed = 2; // lower is faster
const int servo_min = 150;
const int servo_max = 600;
const int nr_doors = 2;

// Packages to bundle things together
enum DoorState { // Enumeration of possible door states
  DoorUnknown,
  DoorOpen,
  DoorClosed
};
typedef struct Door { // Structure for door properties
  int number;
  Adafruit_PWMServoDriver driver;
  int servo;
  DoorState state;
} Door;

// Global variables (Variables that always exist)
RTC_DS1307 RTC; // Real Time Clock
Adafruit_PWMServoDriver driver1 = Adafruit_PWMServoDriver(0x40); // Servo driver
Door doors[] = { // List of doors and their properties
  {1, driver1, 14, DoorUnknown},
  {2, driver1, 15, DoorUnknown}
};

// Method to toggle door open/closed
void set_door(int doornum, DoorState goal) {
  Door& door = doors[doornum - 1];
  Serial.print("Door ");
  Serial.print(door.number);

  if (door.state == goal) {
    if (goal == DoorOpen) Serial.println(" already open");
    else Serial.println(" already closed");
  }
  else {
    if (goal == DoorOpen) {
      Serial.print(" opening... ");
      for (int pulselen = servo_min; pulselen < servo_max; pulselen++) {
        driver1.setPWM(door.servo, 0, pulselen);
        delay(door_open_speed);
      }
    } else {
      Serial.print(" closing... ");
      for (int pulselen = servo_max; pulselen > servo_min; pulselen--) {
        driver1.setPWM(door.servo, 0, pulselen);
        delay(door_open_speed);
      }
    }
    Serial.println("done!");
    door.state = goal;
  }
}

// Method for printing the time
void print_time(DateTime time) {
  Serial.print('[');
  Serial.print(time.day(), DEC);
  Serial.print('/');
  Serial.print(time.month(), DEC);
  Serial.print('/');
  Serial.print(time.year(), DEC);
  Serial.print(' ');
  Serial.print(time.hour(), DEC);
  Serial.print(':');
  Serial.print(time.minute(), DEC);
  Serial.print(':');
  Serial.print(time.second(), DEC);
  Serial.println(']');
}

// Method for blinking LED in case of problem
void blink_led() {
  while (true) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }
}

// The program starts here
void setup() {
  // Setup serial
  Serial.begin(115200);
  Serial.println("Starting setup");

  // Setup LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // Setup clock
  Wire.begin();
  RTC.begin();
  if (!RTC.isrunning()) { // Only set time if no time is present
    Serial.println("Set date on RTC");
    RTC.adjust(DateTime(__DATE__, __TIME__)); // Set date to compile date
  }
  if (!RTC.isrunning()) {
    Serial.println("RTC error!");
    blink_led();
  }
  
  // Setup servo driver
  driver1.begin();
  driver1.setPWMFreq(60);

  Serial.println("Starting loop");
}

// The loop is repeated indefinitely after setup
void loop() {
  // Get time from the clock
  DateTime now = RTC.now();
  print_time(now);

  // If not December 1-25 open all doors
  if (now.month() != 12 || (now.month() == 12 && now.day() > 25) ) {
    for ( int i = 1; i <= nr_doors; i++)
      set_door(i, DoorOpen);
  } else { // If in advent period open door for each day
    for ( int i = 1; i <= nr_doors; i++) {
      if (i <= now.day())
        set_door(i, DoorOpen);
      else
        set_door(i, DoorClosed);
    }
  }

  if (!RTC.isrunning()) {
    Serial.println("RTC error!");
    blink_led();
  }

  delay(6000); // TODO: Put to sleep and turn off servo power
}
