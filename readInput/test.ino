//adding comment for git assignment--Alli
//adding comment for git assignment--Asher
//adding comment for git assignment--Julia
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>// Constants for input pins
#include <DFRobotDFPlayerMini.h>
#include <SoftwareSerial.h>
#include "SoftwareSerial.h"
SoftwareSerial mySerial(0, 1);

# define Start_Byte 0x7E
# define Version_Byte 0xFF
# define Command_Length 0x06
# define End_Byte 0xEF
# define Acknowledge 0x00 //Returns info with command 0x41 [0x01: info, 0x00: no info]
# define ACTIVATED LOW

//Music module
int buttonNext = 2;
int buttonPause = 3;
int buttonPrevious = 4;
boolean isPlaying = false;

//I2C pins declaration
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); 

const int analogInPin = 1;  // Potentiometer pin

// Spinny button
const int button1 = 5;

//This is acutally the rest button
const int startStopButt = 4;
const int microphonePin = A3;

int previousSensorValue = 0;  

// Game variables
int sensorValue = 0;         // Potentiometer value
int button1State = 0;
int startStopState = 0;
int microphoneState = 0;
int state = 0;
float volts;

// Accelerometer Values
const int MPU_ADDR = 0x68; // I2C address of the MPU-6050. If AD0 pin is set to HIGH, the I2C address will be 0x69.
int16_t accelerometer_x, accelerometer_y; // variables for accelerometer raw data
int16_t previous_accelerometer_x, previous_accelerometer_y = 0;
String output;
char tmp_str[7]; // temporary variable used in convert function
char* convert_int16_to_str(int16_t i) { // converts int16 to string. resulting strings will have the same length in the debug monitor.
  sprintf(tmp_str, "%6d", i);
  return tmp_str;
}

void setup() {
  Serial.begin(9600);

  lcd.begin(16,2);
  lcd.backlight();

  //Accelerometer Setup
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR); 
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  //Initialize pins
  pinMode(button1, INPUT_PULLUP);
  pinMode(startStopButt, INPUT_PULLUP);
  pinMode(1, OUTPUT);

  // Music setup
  mySerial.begin(9600);
  delay(1000);
  setVolume(30);

  // Microphone
  pinMode(microphonePin, INPUT);

}

void loop() {
  playSelected(1);
  delay(100000);

  // Read the states of the start and reset buttons
  button1State = digitalRead(button1);
  sensorValue = analogRead(analogInPin);
  startStopState = digitalRead(startStopButt);
  microphoneState = analogRead(microphonePin);

  lcd.backlight();
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); 
  Wire.endTransmission(false); 
  Wire.requestFrom(MPU_ADDR, 7*2, true); 
  accelerometer_x = Wire.read()<<8 | Wire.read(); 
  accelerometer_y = Wire.read()<<8 | Wire.read(); 

  lcd.setCursor(0,0); 

  if (microphoneState == HIGH){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("NOISE");
  }

  if(startStopState == LOW){
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Game Reset");
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
    //lcd.print(output);

    // Update previous values to the new values
    previous_accelerometer_x = accelerometer_x;
    previous_accelerometer_y = accelerometer_y;
  }

  delay(5);
}

void playSelected(byte track)
{
  execute_CMD(0x3F, 0, track);
  delay(500);
  setVolume(20);
  delay(500);
  execute_CMD(0x11,0,1); 
  delay(500);
}

void setVolume(int volume)
{
  execute_CMD(0x06, 0, volume); // Set the volume (0x00~0x30)
  delay(2000);
}

void execute_CMD(byte CMD, byte Par1, byte Par2)
// Excecute the command and parameters
{
  // Calculate the checksum (2 bytes)
  word checksum = -(Version_Byte + Command_Length + CMD + Acknowledge + Par1 + Par2);
  // Build the command line
  byte Command_line[10] = { Start_Byte, Version_Byte, Command_Length, CMD, Acknowledge,
  Par1, Par2, highByte(checksum), lowByte(checksum), End_Byte};
  //Send the command line to the module
  for (byte k=0; k<10; k++){
    mySerial.write( Command_line[k]);
  }
}


//MICROPHONE LOGIC THAT NEEDS TO BE ADDED
// void loop() {
//   volts = analogRead(A3); // returns bit (0= 0 volts, 1024=5.0 volts)
//   volts = volts * 5.0 / 1024.0; // convert bits to volts


//   // now do your test
//   if (volts >= 2.0) {
//   digitalWrite(8, HIGH);
//   }
//   else {
//   digitalWrite(8, LOW);

//   }
// }
