/*
  Setup
  Temp sensor T on pin 6
  Buttton B on pin 7
  Relay R on Pin 12
  lcd on pin 3,4,5,6,10,11
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

// LIBRARIES USED
#include <LiquidCrystal.h>
#include <OneWire.h>
#include <DallasTemperature.h>


// LCD INIT
/*
  LCD RS pin to digital pin 12
   LCD Enable pin to digital pin 11
   LCD D4 pin to digital pin 5
   LCD D5 pin to digital pin 4
   LCD D6 pin to digital pin 3
   LCD D7 pin to digital pin 2
   LCD R/W pin to ground
   LCD VSS pin to ground
   LCD VCC pin to 5V
   10K resistor:
   ends to +5V and ground
   wiper to LCD VO pin (pin 3)
*/
// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 11, en = 10, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


//TEMPERATURE SENSOR INIT
//temp sensor pin - one wire
const int tempSensor = 6;

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(tempSensor);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// arrays to hold device address
DeviceAddress kioskThermometer;


//start button pin
const int buttonPin = 7;

// relay pin
const int relayPin = 12;


// Variables will change:
int relayState = LOW;         // the current state of the output pin
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin

//temperature variables
float TempC;
float TempF;

//debounce variables
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers


//heating cycle state variables
unsigned long maxHeatTimeMS = 1000 * 20; //20 seconds
int inHeatingCycle = 0;
unsigned long lastTimerStartTime = 0;  // the last time the timer was started

void setup() {
  // start serial port
  Serial.begin(9600);
  Serial.println("Starting Adaptive Kiosk Heatlamp and Fan");

  // 1-wire temp startup - locate devices on the bus
  Serial.print("Locating temp sensor...");
  sensors.begin();
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");
  if (!sensors.getAddress(kioskThermometer, 0)) Serial.println("Unable to find temp sensor on Device 0");
  Serial.print("Device 0 Address: ");
  printAddress(kioskThermometer);
  Serial.println("Ready Player One. Press button to start sequence.");


  // set the resolution to 9 bit (Each Dallas/Maxim device is capable of several different resolutions)
  sensors.setResolution(kioskThermometer, 2);


  Serial.print("Device 0 Resolution: ");
  Serial.print(sensors.getResolution(kioskThermometer), DEC);
  Serial.println();



  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("hello, world!");

  //button ans relay pins
  pinMode(buttonPin, INPUT);
  pinMode(relayPin, OUTPUT); //the relay for now

  // set initial relay state
  digitalWrite(relayPin, relayState);


}

void getTemperature(DeviceAddress deviceAddress)
{

  float tempC = sensors.getTempC(deviceAddress);
  Serial.print("Temp C: ");
  Serial.print(tempC);
  Serial.print(" Temp F: ");
  Serial.println(DallasTemperature::toFahrenheit(tempC)); // Converts tempC to Fahrenheit
}

void printTemperature(DeviceAddress deviceAddress)
{

  float tempC = sensors.getTempC(deviceAddress);
  Serial.print("Temp C: ");
  Serial.print(tempC);
  Serial.print(" Temp F: ");
  Serial.println(DallasTemperature::toFahrenheit(tempC)); // Converts tempC to Fahrenheit
}

// function to print the temperature for a device to LCD
void printTemperatureToLCD(DeviceAddress deviceAddress)
{
  lcd.setCursor(0, 0);
  float tempC = sensors.getTempC(deviceAddress);
  lcd.print("Temp C: ");
  lcd.print(tempC);

  lcd.setCursor(0, 1);
  lcd.print("Temp F: ");
  lcd.println(DallasTemperature::toFahrenheit(tempC)); // Converts tempC to Fahrenheit
  lcd.setCursor(13, 1);
  lcd.print("   ");
}

void loop() {
  // read the state of the switch into a local variable:
  int reading = digitalRead(buttonPin);
  //String readingStr = "Reading: " + reading;
  //Serial.println(reading);


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
      Serial.println("buttonState: " + buttonState);

      // only toggle the lamp/fan on the relay if the new button state is HIGH
      if (buttonState == HIGH) {
        relayState = !relayState;
        Serial.println(relayState);//"relayState changed: " + 
      }
    }

    //heating cycle - button pressed and lamp is now on, fans are off
    //start timer or start temp, turn off relay once timer or max temp is reached
    if (relayState == HIGH) {
      //timer function
      if (inHeatingCycle == 0) {
        inHeatingCycle = 1; //now in heating cycle
        Serial.println("Heating cycle started");
        lastTimerStartTime = millis();
      }
      // if timer reached limit, turn off lamp and reset
      int elapsedTime = millis() - lastTimerStartTime;
      
      if ((elapsedTime) > maxHeatTimeMS) {
        relayState = !relayState;
        Serial.println("Heating cycle complete");
        Serial.println(relayState);
        inHeatingCycle = 0; // heating cycle complete
        
      } else { //still in loop 
        Serial.println(elapsedTime);
      }

      //temp function
      sensors.requestTemperatures(); // Send the command to get temperatures
      printTemperature(kioskThermometer);

    }

    //cooling cycle - button pressed and lamp is now off, fans are on
    //if button is pushed again while we are heating up, revert to lamp off and fan on
    if (inHeatingCycle == 1 && relayState == LOW) {
      inHeatingCycle = 0; // heating cycle cancelled
      Serial.println("Heating cycle cancelled");
    }


  }

  // set the relay:
  digitalWrite(relayPin, relayState); //using this for the relay for now

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;

}

// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}
