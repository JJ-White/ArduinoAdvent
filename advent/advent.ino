// Code from other people we want to use
#include <Wire.h>
#include "Adafruit_PWMServoDriver.h"
#include "RTClib.h"

// Constants (Stuff that never changes)
static const int door_open_speed = 5; // lower is faster
static const int servo_min = 100;
static const int servo_max = 500;
static const int nr_doors = 24;

// Packages to bundle things together
enum DoorState { // Enumeration of possible door states
  DoorUnknown,
  DoorOpen,
  DoorClosed
};
typedef struct Door { // Structure for door properties
  const int number;
  const Adafruit_PWMServoDriver driver;
  const int servo;
  DoorState state;
} Door;

// Global variables (Variables that always exist)
RTC_DS1307 RTC; // Real Time Clock
Adafruit_PWMServoDriver driver0 = Adafruit_PWMServoDriver(0x40); // Servo driver
Adafruit_PWMServoDriver driver1 = Adafruit_PWMServoDriver(0x41); // Servo driver
Door doors[] = { // List of doors and their properties
  { 1, driver0,  4, DoorClosed},
  { 2, driver0,  3, DoorClosed},
  { 3, driver0,  2, DoorClosed},
  { 4, driver0,  1, DoorClosed},
  { 5, driver0,  0, DoorClosed},
  { 6, driver0,  9, DoorClosed},
  { 7, driver0,  8, DoorClosed},
  { 8, driver0,  7, DoorClosed},
  { 9, driver0,  6, DoorClosed},
  {10, driver0,  5, DoorClosed},
  {11, driver0, 13, DoorClosed},
  {12, driver0, 12, DoorClosed},
  {13, driver0, 11, DoorClosed},
  {14, driver0, 10, DoorClosed},
  {15, driver1,  3, DoorClosed},
  {16, driver1,  2, DoorClosed},
  {17, driver1,  1, DoorClosed},
  {18, driver1,  0, DoorClosed},
  {19, driver1,  6, DoorClosed},
  {20, driver1,  5, DoorClosed},
  {21, driver1,  4, DoorClosed},
  {22, driver1,  8, DoorClosed},
  {23, driver1,  7, DoorClosed},
  {24, driver1,  9, DoorClosed}
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
    if (goal == DoorClosed) {
      Serial.print(" closing... ");
      for (int pulselen = servo_min; pulselen < servo_max; pulselen++) {
        door.driver.setPin(door.servo, pulselen, false);
        delay(door_open_speed);
      }
    } else {
      Serial.print(" opening... ");
      for (int pulselen = servo_max; pulselen > servo_min; pulselen--) {
        door.driver.setPin(door.servo, pulselen, false);
        delay(door_open_speed);
      }
    }
    Serial.println("done");
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
  Serial.print(']');
  Serial.print(' ');
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

// Disable all servos to limit idle and max current draw
void clear_servos() {
  for (int i = 0; i < nr_doors; i++)
    doors[i].driver.setPin(doors[i].servo, 0, false);
}

// Reset and initialize servo drivers
void reset_drivers() {
  driver0.reset();
  driver1.reset();
  delay(100);
  driver0.begin();
  driver1.begin();
  driver0.setPWMFreq(50);
  driver1.setPWMFreq(50);
}

// Manually control the doors for testing
void manual_control() {
  Serial.println("Switched to manual door control");
  Serial.println("Enter [a-x] to select a door");

  // Clear serial buffer
  while (Serial.read() != -1)
    Serial.read();

  for (int i = 0; i < nr_doors; i++) {
    Serial.print("(");
    Serial.print((char)('a' + i));
    Serial.print(") ");
    Serial.println(doors[i].number);
  }

  Door* manual_door = &doors[0];
  char c = -1;
  while (true) {
    Serial.print("Controlling door ");
    Serial.println(manual_door->number);

    while (c == -1) {
      int val = map(analogRead(A0), 0, 1024, servo_min, servo_max);
      manual_door->driver.setPin(manual_door->servo, val, false);
      delay(10);
      do {
        c = Serial.read();
      } while (c == '\n' );
    }
    clear_servos();

    manual_door = &doors[c - 'a'];
    c = -1;
  }
}

// The program starts here
void setup() {
  // Setup serial
  Serial.begin(115200);
  Serial.println("Program start");
  Serial.println("Setup Serial... done");

  // Setup LED
  Serial.print("Setup LED... ");
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("done");

  // Setup clock
  Serial.print("Setup RTC... ");
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
  Serial.println("done");

  // Setup servo drivers
  Serial.print("Setup PCA9685... ");
  reset_drivers();
  clear_servos();
  Serial.println("done");

  // Pause for switch to manual control
  Serial.println("Press any key within 10s to enter manual mode");
  delay(10000);
  if (Serial.read() != -1)
    manual_control();

  print_time(RTC.now());
  Serial.println("Starting loop");
}

// The loop is repeated indefinitely after setup
void loop() {
  // Get time from the clock
  DateTime now = RTC.now();
  print_time(now);

  // Check if RTC still functions
  if (!RTC.isrunning()) {
    Serial.println("RTC error!");
    blink_led();
  }

  // Wake servo drivers
  driver0.wakeup();
  driver1.wakeup();

  // If not December 1-24 do nothing (assumes doors are closed manually before Dec 1st)
  if (now.month() != 12 || now.day() > 24 ) {
    Serial.println("Nothing to do");
  } else { // If in advent period open door for each day
    for ( int i = 1; i <= nr_doors; i++) {
      if (i == now.day()) {
        set_door(i, DoorOpen);
        clear_servos();
      }
    }
  }

  // Test mode, open all doors in sequentially
  //  for ( int i = 1; i <= nr_doors; i++) {
  //    set_door(i, DoorOpen);
  //    clear_servos();
  //  }

  // Set servo drivers to sleep
  driver0.sleep();
  driver1.sleep();

  delay(30000);
}
