#include <Wire.h> 
#include <LiquidCrystal_I2C.h>// Constants for input pins

//I2C pins declaration
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); 

//const int analogInPin = A0;  // Potentiometer pin
//const int buttonPin = 2;     // Pushbutton pin for game action
const int startButtonPin = 2; // Start button pin
const int resetButtonPin = 3; // Reset button pin
//const int ledPin = 13;       // LED pin for feedback

// Game variables
//int sensorValue = 0;         // Potentiometer value
//int buttonState = 0;         // Button state
int startButtonState = 0;    // Start button state
int resetButtonState = 0;    // Reset button state
//unsigned long timeLimit = 2000; // Initial time limit for each action (milliseconds)
//unsigned long actionStartTime;  // Time when action started
//int currentAction = 0;       // 0 = button press, 1 = twist potentiometer
//int score = 0;               // Player's score
//bool gameActive = false;     // Whether the game is ongoing
bool gameStarted = false;    // Whether the game has started

void setup() {
  lcd.begin(16,2);//Defining 16 columns and 2 rows of lcd display
  lcd.backlight();//To Power ON the back light

  // Initialize pins
  //pinMode(ledPin, OUTPUT);
  //pinMode(buttonPin, INPUT);
  pinMode(startButtonPin, INPUT);
  pinMode(resetButtonPin, INPUT);
  pinMode(1, OUTPUT);
}

void loop() {
  // Read the states of the start and reset buttons
  startButtonState = digitalRead(startButtonPin);
  resetButtonState = digitalRead(resetButtonPin);

  lcd.setCursor(0,0); //Defining positon to write from first row,first column .

  // // Start button logic
  if (startButtonState == HIGH) {
      gameStarted = true;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Game Started");
      digitalWrite(1, HIGH);
  }

  // Reset button logic
  if (resetButtonState == HIGH) {
    lcd.clear();//Clean the screen
    gameStarted = false;
    //resetGame();  // Reset the game to initial state
  }
}
