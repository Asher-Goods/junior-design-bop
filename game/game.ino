#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DFRobotDFPlayerMini.h>
#include <SoftwareSerial.h>
#include "SoftwareSerial.h"
SoftwareSerial mySerial(0, 1);

// preprocessor constants for MP3 Player config
#define Start_Byte 0x7E
#define Version_Byte 0xFF
#define Command_Length 0x06
#define End_Byte 0xEF
#define Acknowledge 0x00 // Returns info with command 0x41 [0x01: info, 0x00: no info]
#define ACTIVATED LOW

// I2C pins declaration for LCD Screen
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
DFRobotDFPlayerMini myDFPlayer;

// Input button declarations
const int twisty = A0;        // Potentiometer pin -- this is twisty input
const int slider = A1;        // Potentiometer pin -- th is is slider
const int microphonePin = A3; // Microphone analog input
const int spinnyButt = 5;     // Spinny button
const int resetButt = 3;      // Reset Game Button

// Game variables
int twistyValue = 0;
int sliderValue = 0;
int spinnyButtValue = 0;
int resetValue = 0;
int microphoneValue = 0;

int previousTwistyValue;
int previousSliderValue;

// game states
enum GameState { PITCH_IT, TURN_IT_UP, SHOUT_IT, SPIN_IT };
GameState states[] = { PITCH_IT, TURN_IT_UP, SHOUT_IT, SPIN_IT };
int successCount = 0;
int result;

// Music variables
int song = 1;
int volume = 30;
int pitch;

// Accelerometer Values
// I2C address of the MPU-6050. If   pin is set to HIGH, the I2C address will be 0x69.
const int MPU_ADDR = 0x68;
// variables for accelerometer raw data
int16_t accelerometer_x, accelerometer_y;
int16_t previous_accelerometer_x, previous_accelerometer_y = 0;

// Variables for song
int countTwist = 0;
int countSlider = 0;
int countSpin = 0;
int countPitch = 0;

void setup()
{
    mySerial.begin(9600);
    delay(1000);

    // LCD Setup
    lcd.begin(16, 2);
    lcd.backlight();

    // Accelerometer Setup
    Wire.begin();
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x6B);
    Wire.write(0);
    Wire.endTransmission(true);

    // Initialize pins
    pinMode(spinnyButt, INPUT_PULLUP);
    pinMode(resetButt, INPUT_PULLUP);
    pinMode(twisty, INPUT);
    pinMode(slider, INPUT);
    pinMode(microphonePin, INPUT);

    if (!myDFPlayer.begin(mySerial, /*isACK = */true, /*doReset = */true)) {  //Use serial to communicate with mp3.
      while(true){
        delay(0); // Code to compatible with ESP8266 watch dog.
      }
    }

    myDFPlayer.volume(30);
    myDFPlayer.play(1);

    // initialize previous values 
    setPreviousValue();
}

void loop()
{
  // read in all values
  readValues();
  // Check for reset
  if (resetValue == LOW) {
    gameRecap();
    resetGame();
    return;
  }
  //playSelected(1);

  // Run current game state logic based on the randomized state array
  switch (states[successCount]) {
    case PITCH_IT:
      result = testValues(1);
      lcd.clear();
      lcd.print("Pitch it!");
      if (result == 0) 
      {
        lcd.clear();
        lcd.print("Wrong Input!");
        delay(750);
        gameRecap();
        resetGame();
      }
      else if (result == 1)
      {
        myDFPlayer.play(2);
        lcd.clear();
        lcd.print("Success!");
        countPitch++;
        nextState();
      }
      break;
    case TURN_IT_UP:
      lcd.clear();
      lcd.print("Turn it up!");
      result = testValues(2);
      if (result == 0) 
      {
        lcd.clear();
        lcd.print("Wrong Input!");
        delay(750);
        gameRecap();
        resetGame();
      }
      else if (result == 1)
      {
        lcd.clear();
        lcd.print("Success!");
        nextState();
        countTwist++;
      }
      break;
    case SHOUT_IT:
      lcd.clear();
      lcd.print("Shout it!");
      result = testValues(3);
      if (result == 0) 
      {
        lcd.clear();
        lcd.print("Wrong Input!");
        delay(750);
        gameRecap();
        resetGame();
      }
      else if (result == 1)
      {
        lcd.clear();
        lcd.print("Success!");
        nextState();
      }
      break;
    case SPIN_IT:
      lcd.clear();
      lcd.print("Spin it!");
      result = testValues(5);
      if (result == 0) 
      {
        lcd.clear();
        lcd.print("Wrong Input!");
        delay(750);
        gameRecap();
        resetGame();
      }
      else if (result == 1)
      {
        //myDFPlayer.play(8);
        if (countSpin == 0){
          myDFPlayer.play(8);
          countSpin++;
          break;
        }
        if (countSpin == 1)
        {
          myDFPlayer.play(5);
          countSpin = 0;
        }

        lcd.clear();
        lcd.print("Success!");
        nextState();
      }
      break;
  }

  // repeat every 100ms
  delay(100);
}

void readValues(void)
{
    // Read pin values
    twistyValue     = analogRead(twisty);
    sliderValue     = analogRead(slider);
    microphoneValue = analogRead(microphonePin);
    spinnyButtValue = digitalRead(spinnyButt);
    resetValue      = digitalRead(resetButt);

    // Read accelerometer values
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_ADDR, 7 * 2, true);
    accelerometer_x = Wire.read() << 8 | Wire.read();
    accelerometer_y = Wire.read() << 8 | Wire.read();
}

// read all input values
// @args inputType:
// 1 = twisty
// 2 = slider
// 3 = microphone 
// 4 = spinny
// 5 = accelerometer
int testValues(int expectedInput)
{
    // Check the expected input first
    if (expectedInput == 1 && handleTwisty()) return 1;
    if (expectedInput == 2 && handleSlider()) return 1;
    if (expectedInput == 3 && handleMicrophone()) return 1;
    if (expectedInput == 4 && handleSpinnyButt()) return 1;
    if (expectedInput == 5 && handleAccelerometer()) return 1;

    // Check other inputs
    // if (expectedInput != 1 && handleTwisty()) return 0;
    // if (expectedInput != 2 && handleSlider()) return 0;
    // if (expectedInput != 3 && handleMicrophone()) return 0;
    // if (expectedInput != 4 && handleSpinnyB`utt()) return 0;
    // if (expectedInput != 5 && handleAccelerometer()) return 0;

    // If no inputs return 2
    return 2;
}
// check if accelerometer value changed
bool handleAccelerometer(void)
{
    int thresholdValue = 5000;
    bool valueChange = (abs(accelerometer_x - previous_accelerometer_x) >= thresholdValue || abs(accelerometer_y - previous_accelerometer_y) >= thresholdValue);
    // update value
    if (valueChange)
    {
        previous_accelerometer_x = accelerometer_x;
        previous_accelerometer_y = accelerometer_y;
    }
    return valueChange;
}

bool handleTwisty(void)
{
    int thresholdValue = 10;
    bool valueChange = (abs(twistyValue - previousTwistyValue) >= thresholdValue);

    if (valueChange)
    {
        previousTwistyValue = twistyValue;
    }
    return valueChange;
}


// check if slider potentiometer changed
bool handleSlider(void)
{
    int thresholdValue =  1;
    bool valueChange = 0;

    readValues();
    // bool valueChange = (abs(sliderValue - previousSliderValue) >= thresholdValue);
    // if (valueChange)
    // {
    //     previousSliderValue = sliderValue;
    // }
    // // include logic for proportional pitch change
    // return valueChange;

    if (previousSliderValue == 1020)
    {
      if (sliderValue != previousSliderValue)
      {
        valueChange = 1;
        previousSliderValue = sliderValue;
      }
    }

    else if (previousSliderValue == 1023)
    {
      if (sliderValue != previousSliderValue)
      {
        valueChange = 1;
        previousSliderValue = sliderValue;
      }
    }

    else if (previousSliderValue == 1021)
    {
      if (sliderValue != previousSliderValue)
      {
        valueChange = 1;
        previousSliderValue = sliderValue;
      }
    }

    // else if (previousSliderValue == 1022)
    // {
    //   if (sliderValue != previousSliderValue)
    //   {
    //     valueChange = 1;
    //     previousSliderValue = sliderValue;
    //   }
    // }

    else{
      valueChange = 0;
      previousSliderValue = sliderValue;
    }

    return valueChange;
}

// check if microphone passes threshold value
bool handleMicrophone(void)
{
    float thresholdValue = 100.0;
    // float conversionValue = 5.0 / 1023.0;
    // float voltageThreshold = 2.2;
    // float microphoneVoltage = conversionValue * microphoneValue;
    return microphoneValue < thresholdValue;
}

// check if reset button was pressed
bool handleResetButt(void)
{
    return resetValue == LOW;
}

// check if spinnyButton was pressed
bool handleSpinnyButt(void)
{
    return spinnyButtValue == LOW;
}

void setPreviousValue(void)
{
    readValues();
    previousTwistyValue = twistyValue;
    previousSliderValue = sliderValue;
    previous_accelerometer_x = accelerometer_x;
    previous_accelerometer_y = accelerometer_y;
}

// State machine methods
// Move to the next state in the randomized sequence
void nextState() {
  successCount++;
  setPreviousValue();
  delay(1000);
  if (successCount >= 5) {  // If all states are completed, reset game
    // gameRecap();
    // resetGame();
    successCount =0;
  }
}

// Randomize the order of states
void randomizeStates() {
  for (int i = 0; i < 3; i++) {
    // int randomIndex = random(0, 5);
    // GameState temp = states[i];
    states[i] = i;
    // states[randomIndex];
    // states[randomIndex] = temp;
  }
  successCount = 0;  // Start at the beginning of the randomized sequence
}

// Reset the game
void resetGame() {
  lcd.clear();
  myDFPlayer.play(1);
  lcd.print("Game Resetting!");
  delay(1000);
  randomizeStates();  // Shuffle states for a new game
  setPreviousValue();
  successCount = 0;   // Reset the index to the start of the new sequence
}

void gameRecap() {
    lcd.clear();
    String output = "You succeeded ";
    output += String(successCount);
    output += " times!";
    lcd.print(output);
    delay(2500);
}