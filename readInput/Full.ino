#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <DFRobotDFPlayerMini.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11);  // Changed pins to avoid Serial conflict
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Game states
enum GameState { SHOUT_IT, TURN_IT_UP, PITCH_IT, PRESS_IT, SPIN_IT };
GameState states[] = { SHOUT_IT, TURN_IT_UP, PITCH_IT, PRESS_IT, SPIN_IT };
int currentIndex = 0;  // Track the current randomized state

// Input Pins
const int buttonNext = 2;
const int buttonPause = 3;
const int buttonPrevious = 4;
const int microphonePin = A3;
const int twistyPin = A1;          // Potentiometer pin -- twisty input
const int resetButton = 4;         // Reset Button
const int spinButton = 5;          // Spinny button
const int slider = A0;             // Potentiometer pin -- this is slider

// Game Variables
int twistyValue = 0;         // Potentiometer value
int button1State = 0;
int resetValue = 0;
int microphoneState = 0;
float microphoneVolts;
int previousTwistyState = 0;  

// Accelerometer Values
const int MPU_ADDR = 0x68;
int16_t accelerometer_x, accelerometer_y;
int16_t previous_accelerometer_x, previous_accelerometer_y = 0;

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  lcd.begin(16, 2);
  lcd.backlight();

  // Initialize Pins
  pinMode(buttonNext, INPUT_PULLUP);
  pinMode(buttonPause, INPUT_PULLUP);
  pinMode(buttonPrevious, INPUT_PULLUP);
  pinMode(microphonePin, INPUT);
  pinMode(twistyPin, INPUT);
  pinMode(spinButton, INPUT_PULLUP);
  pinMode(resetButton, INPUT_PULLUP);

  // Initialize accelerometer
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  // Start Music
  delay(1000);
  setVolume(30);

  // Randomize the game states
  randomizeStates();
}

void loop() {
  // Check for reset
  resetValue = digitalRead(resetButton);
  if (resetValue == LOW) {
    resetGame();
    return;
  }
  
  // Run current game state logic based on the randomized state array
  switch (states[currentIndex]) {
    case SHOUT_IT:
      shoutIt();
      break;
    case TURN_IT_UP:
      turnItUp();
      break;
    case PITCH_IT:
      pitchIt();
      break;
    case PRESS_IT:
      pressIt();
      break;
    case SPIN_IT:
      spinIt();
      break;
  }

  delay(100);  // Loop delay to manage game speed
}

// Game state functions
void shoutIt() {
  microphoneVolts = analogRead(microphonePin) * 5.0 / 1024.0;
  if (microphoneVolts > 2.0) {
    lcd.clear();
    lcd.print("Shout It Success!");
    delay(1000);
    nextState();
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Shout louder!");
  }
}

void turnItUp() {
  twistyValue = analogRead(twistyPin);
  if (twistyValue > 800) {
    lcd.clear();
    lcd.print("Turn It Up Success!");
    delay(1000);
    nextState();
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Turn the knob up!");
  }
}

void pitchIt() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 6, true);
  accelerometer_x = Wire.read() << 8 | Wire.read();
  accelerometer_y = Wire.read() << 8 | Wire.read();
  
  if (abs(accelerometer_x) > 10000) {
    lcd.clear();
    lcd.print("Pitch It Success!");
    delay(1000);
    nextState();
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Move it up/down!");
  }
}

void pressIt() {
  button1State = digitalRead(buttonNext);
  if (button1State == LOW) {
    lcd.clear();
    lcd.print("Press It Success!");
    delay(1000);
    nextState();
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Press the button!");
  }
}

void spinIt() {
  int spinState = analogRead(twistyPin);
  if (spinState > 500) {
    lcd.clear();
    lcd.print("Spin It Success!");
    delay(1000);
    nextState();
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Spin the knob!");
  }
}

// Move to the next state in the randomized sequence
void nextState() {
  currentIndex++;
  if (currentIndex >= 5) {  // If all states are completed, reset game
    resetGame();
  }
}

// Randomize the order of states
void randomizeStates() {
  for (int i = 0; i < 5; i++) {
    int randomIndex = random(0, 5);
    GameState temp = states[i];
    states[i] = states[randomIndex];
    states[randomIndex] = temp;
  }
  currentIndex = 0;  // Start at the beginning of the randomized sequence
}

// Reset the game
void resetGame() {
  lcd.clear();
  lcd.print("Game Resetting!");
  delay(1000);
  randomizeStates();  // Shuffle states for a new game
  currentIndex = 0;   // Reset the index to the start of the new sequence
}
