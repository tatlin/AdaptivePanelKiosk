/*
Setup
Temp sensor T on pin 1
Buttton B on pin 2
Relay R on Pin 3
lcd on pin 4
Timer
TimerMax = 2 min

Init button, relay and sensor vars 
Button and relay should be Low
Relay off / low means fan on and light off


Run loop

Read temp
Write temp to lcd
If B == high, //start button pressed
  Set runningLoop = true
  Start timer
  Set relay to high //light on fan off

If runningLoop == true
  While Timer<TimerMax
   Sleep
Else
  Set runningLoop false
  Set relay to low /light off fan on

*/

// include the library code:
#include <LiquidCrystal.h>

// relay pin
int relay = 13;         

// lcd pins
/*
* LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
*/
// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 11, en = 10, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//temp sensor pin - one wire
const int tempSensor = 6;

//start button pin
const int buttonPin = 7;

// relay pin
const int relayPin = 12;


// Variables will change:
int relayState = HIGH;         // the current state of the output pin
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

void setup() {
// start serial port
  Serial.begin(9600);
  Serial.println("Starting Adaptive Kiosk Heatlamp and Fan");
    
  pinMode(buttonPin, INPUT);
  pinMode(relayPin, OUTPUT); //the relay for now

  // set initial relay state
  digitalWrite(relayPin, relayState);
  

}

void loop() {
// read the state of the switch into a local variable:
  int reading = digitalRead(buttonPin);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;
      Serial.println(buttonState);

      // only toggle the lamp/fan on the relay if the new button state is HIGH
      if (buttonState == HIGH) {
        relayState = !relayState;
        Serial.println(relayState);
      }
    }
  }

  // set the relay:
  digitalWrite(relayPin, relayState); //using this for the relay for now

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;

}
