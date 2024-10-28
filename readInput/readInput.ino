// // Constants for input pins
// const int analogInPin = A0;  // Potentiometer pin
// const int buttonPin = 2;     // Pushbutton pin for game action
// const int startButtonPin = 3; // Start button pin
// const int resetButtonPin = 4; // Reset button pin
// const int ledPin = 13;       // LED pin for feedback

// // Game variables
// int sensorValue = 0;         // Potentiometer value
// int buttonState = 0;         // Button state
// int startButtonState = 0;    // Start button state
// int resetButtonState = 0;    // Reset button state
// unsigned long timeLimit = 2000; // Initial time limit for each action (milliseconds)
// unsigned long actionStartTime;  // Time when action started
// int currentAction = 0;       // 0 = button press, 1 = twist potentiometer
// int score = 0;               // Player's score
// bool gameActive = false;     // Whether the game is ongoing
// bool gameStarted = false;    // Whether the game has started

// void setup() {
//   // Initialize serial communication
//   Serial.begin(9600);

//   // Initialize pins
//   pinMode(ledPin, OUTPUT);
//   pinMode(buttonPin, INPUT);
//   pinMode(startButtonPin, INPUT);
//   pinMode(resetButtonPin, INPUT);

//   // Initial prompt to press the start button
//   Serial.println("Press the Start button to begin!");
// }

// void loop() {
//   // Read the states of the start and reset buttons
//   startButtonState = digitalRead(startButtonPin);
//   resetButtonState = digitalRead(resetButtonPin);

//   // Start button logic
//   if (startButtonState == HIGH && !gameStarted) {
//     gameStarted = true;
//     Serial.println("Game started!");
//     restartGame();  // Start the game
//   }

//   // Reset button logic
//   if (resetButtonState == HIGH) {
//     resetGame();  // Reset the game to initial state
//   }

//   // Only run game logic if the game is active
//   if (gameActive) {
//     // Check if the time limit has expired
//     if (millis() - actionStartTime > timeLimit) {
//       gameOver();
//     }

//     // Read inputs
//     sensorValue = analogRead(analogInPin);
//     buttonState = digitalRead(buttonPin);

//     // Check if the player completed the current action
//     if (currentAction == 0 && buttonState == HIGH) {
//       // Button press action completed
//       Serial.println("Button pressed! Success!");
//       increaseScore();
//       pickNewAction();
//     } else if (currentAction == 1 && sensorValue > 800) {
//       // Potentiometer twist action completed
//       Serial.println("Potentiometer twisted! Success!");
//       increaseScore();
//       pickNewAction();
//     }

//     // Add a small delay
//     delay(100);
//   }
// }

// void pickNewAction() {
//   // Randomly pick an action: 0 = press button, 1 = twist potentiometer
//   currentAction = random(0, 2);
//   actionStartTime = millis();
//   Serial.print("New action: ");
//   if (currentAction == 0) {
//     Serial.println("Press the button!");
//   } else {
//     Serial.println("Twist the potentiometer!");
//   }

//   // Flash LED to indicate new action
//   digitalWrite(ledPin, HIGH);
//   delay(200);
//   digitalWrite(ledPin, LOW);
// }

// void increaseScore() {
//   score++;  // Increase the score
//   Serial.print("Score: ");
//   Serial.println(score);

//   // Shorten the time limit as the score increases
//   timeLimit = max(500, 2000 - (score * 100)); // Decreases by 100ms each score, minimum limit of 500ms
//   Serial.print("New time limit: ");
//   Serial.println(timeLimit);
// }

// void restartGame() {
//   // Reset game state but keep the game active
//   score = 0;                  // Reset score
//   timeLimit = 2000;           // Reset time limit
//   gameActive = true;
//   actionStartTime = millis();
//   pickNewAction();
// }

// void resetGame() {
//   // Reset everything to the initial state
//   gameActive = false;   // Deactivate the game
//   gameStarted = false;  // Game is not started
//   score = 0;            // Reset score
//   timeLimit = 2000;     // Reset time limit

//   Serial.println("Game reset. Press the Start button to begin!");
// }

// void gameOver() {
//   // End the game
//   Serial.println("Time's up!");
//   Serial.print("Final score: ");
//   Serial.println(score);
//   gameActive = false;  // Stop the game
// }
