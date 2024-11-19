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
int microphoneValue = 101;

int previousTwistyValue;
int previousSliderValue;

// game states
const int num_rounds = 10;
enum GameState { PITCH_IT, TURN_IT_UP, SHOUT_IT, SPIN_IT };
GameState states[num_rounds];// = { PITCH_IT, TURN_IT_UP, SHOUT_IT, SPIN_IT };
int successCount = 0;
int result;
bool gameStart = false;
int currentVolume;

// Music variables
int pitch = 0;

// Accelerometer Values
// I2C address of the MPU-6050. If   pin is set to HIGH, the I2C address will be 0x69.
const int MPU_ADDR = 0x68;
// variables for accelerometer raw data
int16_t accelerometer_x, accelerometer_y;
int16_t previous_accelerometer_x, previous_accelerometer_y = 0;

// Variables for song
int song = 0;
int countSpin = 0;

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

    myDFPlayer.volume(15);
    song = 1;
    
    // initialize previous values 
    setPreviousValue();
}

void loop() {
  // Read input values
  readValues();

  // Check for reset
  if (resetValue == LOW) {
    if (!gameStart) {
      startGame();
    } else {
      gameRecap();
      resetGame();
    }
    return;
  }

  if (!gameStart) {
    lcd.clear();
    lcd.print("Press the button to start!");
    delay(50);
    return;
  }

  // Execute the current state logic
  switch (states[successCount]) {
    case PITCH_IT:
      lcd.clear();
      lcd.print("Pitch it!");
      result = testValues(1);
      handleResult(result);
      break;

    case TURN_IT_UP:
      lcd.clear();
      lcd.print("Turn it up!");
      result = testValues(2);
      handleResult(result);
      break;

    case SHOUT_IT:
      lcd.clear();
      lcd.print("Shout it!");
      result = testValues(3);
      handleResult(result);
      break;

    case SPIN_IT:
      lcd.clear();
      lcd.print("Spin it!");
      result = testValues(5);
      handleResult(result);
      break;
  }

  delay(100); // Small delay to allow for button debouncing
}


void handleResult(int result) {
  if (result == 0) { // Incorrect input
    lcd.clear();
    lcd.print("Wrong Input!");
    delay(750);
    gameRecap();
    resetGame();
  } else if (result == 1) { // Correct input
    lcd.clear();
    lcd.print("Success!");
    if(states[successCount] == SPIN_IT) handleSpin();
    if(states[successCount] == PITCH_IT) handlePitchChange();
    nextState(); // Move to the next state
  }
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
    if (expectedInput == 5 && handleAccelerometer()) return 1;

    // Check other inputs
    // if (expectedInput != 1 && handleTwisty()) 
    // {
    //   lcd.clear();
    //   lcd.print("twisty...");
    //   delay(2000);
    //   return 0; 
    // }
    if (expectedInput != 2 && handleSlider())
    {
      lcd.clear();
      lcd.print("slider...");
      delay(2000);
      return 0; 
    }
    if (expectedInput != 5 && handleAccelerometer())
    {
      lcd.clear();
      lcd.print("accel...");
      delay(2000);
      return 0; 
    }

    // If no inputs return 2
    return 2;
}

void startGame(void)
{
  gameStart = 1;
  lcd.clear();
  lcd.print("Starting game...");
  delay(1000);
  randomizeStates();  // Shuffle states for a new game
  setPreviousValue();
  myDFPlayer.play(song);
  successCount = 0;   // Rfeset the index to the start of the new sequence

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
    int thresholdValue = 100;
    int change = twistyValue - previousTwistyValue;
    bool valueChange = (abs(change) >= thresholdValue);

    if (valueChange)
    {
        previousTwistyValue = twistyValue;
    }

    if (change < 0) {
      pitch = 1;
    } else {
      pitch = -1;
    }

    return valueChange;
}


// check if slider potentiometer changed
bool handleSlider(void)
{
   int thresholdValue =  100;

    readValues();
    bool valueChange = (abs(sliderValue - previousSliderValue) >= thresholdValue);
    if (valueChange)
    {
        if (previousSliderValue > sliderValue)
        {
          myDFPlayer.volume(15);
        }
        if (previousSliderValue < sliderValue)
        {
          myDFPlayer.volume(20
          );
        }

        previousSliderValue = sliderValue;
    }

    return valueChange;

}

void handleSpin()
{
  switch(countSpin) {
      case 0 :
      song = 5;
      break;
      case 1 :
      song = 8;
      break;
      default :
      song = 1;
      break;
    }
    myDFPlayer.play(song);
    countSpin++;
}

void handlePitchChange() {
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
        // charlie pitch changes
        if (song == 1){
          if(pitch == 1) {
            song = 2;
            myDFPlayer.play(song);
          } else if(pitch == -1) {
            song = 3;
            myDFPlayer.play(song);
          }
        }
        if (song == 2){
          if(pitch == -1) {
            song = 1;
            myDFPlayer.play(song);
          }
        }
        if (song == 3){
          if(pitch == 1) {
            song = 1;
            myDFPlayer.play(song);
          }
        }
        // pitbull pitch changes
        if (song == 5)
        {
          if(pitch == 1) {
            song = 9;
            myDFPlayer.play(song);
          } else if(pitch == -1) {
            song = 4;
            myDFPlayer.play(song);
          }
        }
        if (song == 4){
          if(pitch == -1) {
            song = 5;
            myDFPlayer.play(song);
          }
        }
        if (song == 9){
          if(pitch == 1) {
            song = 5;
            myDFPlayer.play(song);
          }
        }
        // alabama pitch change
        if (song == 8)
        {
          if(pitch == 1) {
            song = 7;
            myDFPlayer.play(song);
          } else if(pitch == -1) {
            song = 6;
            myDFPlayer.play(song);
          }
        }
        if (song == 7){
            myDFPlayer.play(song);
          if(pitch == -1) {
            song = 8;
          }
        }
        if (song == 6){
          if(pitch == 1) {
            song = 8;
            myDFPlayer.play(song);
          }
        }
}
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
  setPreviousValue(); // Ensure input values are reset for the next state
  delay(1000); // Pause to allow the player to see the "Success!" message
  
  if (successCount >= num_rounds) {  // Check if the game has completed all rounds
    gameRecap();  // Show final recap
    resetGame();  // Reset the game
    return;       // Exit function to prevent further state transitions
  }
  
  lcd.clear();
  lcd.print("Get ready!");
  lcd.print(states[successCount]);
  delay(1000); // Optional delay to prepare the user for the next input
}


// Randomize the order of states
void randomizeStates() {
  for (int i = 0; i < num_rounds; i++) {  // Loop through all indices up to num_rounds
    int randomIndex = random(0, 4);       // Generate a random index (0 to 3 inclusive)
    states[i] = (randomIndex);  // Assign a valid enum value
  }
  successCount = 0;  // Reset success count
}



// Reset the game
void resetGame() {
  lcd.clear();
  lcd.print("Game Resetting!");
  delay(1000);
  randomizeStates();  // Shuffle states for a new game
  setPreviousValue();
  gameStart = 0;
  successCount = 0;   // Reset the index to the start of the new sequence
  myDFPlayer.volume(15);
  song = 1;
}

void gameRecap() {
    lcd.clear();
    lcd.print("You succeeded ");
    lcd.print(successCount);
    lcd.print(" times!");
    delay(2500);
}