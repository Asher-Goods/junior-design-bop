//adding comment for git assignment--Alli
//adding comment for git assignment--Asher
//adding comment for git assignment--Julia
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>// Constants for input pins

//I2C pins declaration
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); 

const int analogInPin = A3;  // Potentiometer pin
//const int buttonPin = 2;     // Pushbutton pin for game action
const int startButtonPin = 2; // Start button pin
const int resetButtonPin = 3; // Reset button pin
const int button1 = 6;
// const int button2 = 6;
// const int button3 = 6;
int previousSensorValue = 0;  // To track changes in potentiometer

// Game variables
int sensorValue = 0;         // Potentiometer value
//int buttonState = 0;         // Button state
int startButtonState = 0;    // Start button state
int resetButtonState = 0;    // Reset button state
int button1State = 0;
// int button2State = 0;
// int button3State = 0;


//unsigned long timeLimit = 2000; // Initial time limit for each action (milliseconds)
//unsigned long actionStartTime;  // Time when action started
//int currentAction = 0;       // 0 = button press, 1 = twist potentiometer
//int score = 0;               // Player's score
//bool gameActive = false;     // Whether the game is ongoing
bool gameStarted = false;    // Whether the game has started

// Accelerometer Values
// const int MPU_ADDR = 0x68; // I2C address of the MPU-6050. If AD0 pin is set to HIGH, the I2C address will be 0x69.
// int16_t accelerometer_x, accelerometer_y; // variables for accelerometer raw data
// int16_t previous_accelerometer_x, previous_accelerometer_y = 0;
// String output;
// char tmp_str[7]; // temporary variable used in convert function
// char* convert_int16_to_str(int16_t i) { // converts int16 to string. Moreover, resulting strings will have the same length in the debug monitor.
//   sprintf(tmp_str, "%6d", i);
//   return tmp_str;
// }

void setup() {
  Serial.begin(9600);

  lcd.begin(16,2);//Defining 16 columns and 2 rows of lcd display
  lcd.backlight();//To Power ON the back light

  // Accelerometer Setup
  // Wire.begin();
  // Wire.beginTransmission(MPU_ADDR); // Begins a transmission to the I2C slave (GY-521 board)
  // Wire.write(0x6B); // PWR_MGMT_1 register
  // Wire.write(0); // set to zero (wakes up the MPU-6050)
  // Wire.endTransmission(true);

  // Initialize pins
  //pinMode(ledPin, OUTPUT);
  //pinMode(buttonPin, INPUT);
  pinMode(startButtonPin, INPUT);
  pinMode(resetButtonPin, INPUT);
  pinMode(button1, INPUT_PULLUP);
  // pinMode(button2, INPUT_PULLUP);
  // pinMode(button3, INPUT_PULLUP);
  pinMode(1, OUTPUT);
}

void loop() {
  // Read the states of the start and reset buttons
  startButtonState = digitalRead(startButtonPin);
  resetButtonState = digitalRead(resetButtonPin);
  button1State = digitalRead(button1);
  sensorValue = analogRead(analogInPin);

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H) [MPU-6000 and MPU-6050 Register Map and Descriptions Revision 4.2, p.40]
  Wire.endTransmission(false); // the parameter indicates that the Arduino will send a restart. As a result, the connection is kept active.
  Wire.requestFrom(MPU_ADDR, 7*2, true); // request a total of 7*2=14 registers
  accelerometer_x = Wire.read()<<8 | Wire.read(); // reading registers: 0x3B (ACCEL_XOUT_H) and 0x3C (ACCEL_XOUT_L)
  accelerometer_y = Wire.read()<<8 | Wire.read(); 

  lcd.setCursor(0,0); //Defining positon to write from first row,first column .

  // Start button logic
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

  // Button display logic
  if (button1State == LOW){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Button 1");
  }

  // Check for change in potentiometer value
  if (abs(sensorValue - previousSensorValue) >= 50) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Potentiometer:");
    lcd.setCursor(0, 1);
    lcd.print(sensorValue);
    
    // Update previous sensor value to the current value
    previousSensorValue = sensorValue;
  }

  // Change in accelerometer value
  if (abs(accelerometer_x - previous_accelerometer_x) >= 2000 || abs(accelerometer_y - previous_accelerometer_y) >= 2000) {
    lcd.clear();
    lcd.setCursor(0, 0);
    output = "";
    output += convert_int16_to_str(accelerometer_x);
    output += " ";
    output += convert_int16_to_str(accelerometer_y);
    lcd.print(output);

    // Update previous values to the new values
    previous_accelerometer_x = accelerometer_x;
    previous_accelerometer_y = accelerometer_y;
  }

  delay(5);
}
