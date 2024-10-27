#include "BluetoothSerial.h"

// Initialize Bluetooth Serial object
BluetoothSerial SerialBT;

// Define segment pins (active-low)
const int segA = 4;   // Segment a or ball 1 / top
const int segB = 22;   // Segment b or ball 2
const int segC = 21;  // Segment c or ball 3
const int segD = 19;  // Segment d or strike 1 / bottom
const int segE = 18;  // Segment e or strike 2
const int segF = 5;  // Segment f or out 1
const int segG = 23;  // Segment g or out 2

// Define digit control pins (active-low)
int digits[] = {12, 14, 27, 26, 25, 32, 33, 13, 15};  // Adjustable order

// Pin connected to the LED
const int ledPin = 2;   // D2 pin connected to LED

// Scoreboard variables
int inning = 1;
bool topOfInning = true;
int homeScore = 00;
int guestScore = 00;
int pitcherCountdown = 12;
bool pitcherRunning = false;
int strikeCount = 0;
int ballCount = 0;
int outCount = 0;
bool on = true;

// Define the 7-segment display pattern for digits 0-9 (active-low)
const int digitPatterns[10][7] = {
  {0, 0, 0, 0, 0, 0, 1},  // 0
  {1, 0, 0, 1, 1, 1, 1},  // 1
  {0, 0, 1, 0, 0, 1, 0},  // 2
  {0, 0, 0, 0, 1, 1, 0},  // 3
  {1, 0, 0, 1, 1, 0, 0},  // 4
  {0, 1, 0, 0, 1, 0, 0},  // 5
  {0, 1, 0, 0, 0, 0, 0},  // 6
  {0, 0, 0, 1, 1, 1, 1},  // 7
  {0, 0, 0, 0, 0, 0, 0},  // 8
  {0, 0, 0, 0, 1, 0, 0}   // 9
};

// Timing variables for multiplexing and blinking
unsigned long lastUpdateTime = 0;
int displayInterval = 2;  // Time (in milliseconds) to show each digit
unsigned long lastBlinkTime = 0;  // Tracks time for blinking before pairing
bool ledState = false;            // LED state for blinking
unsigned long lastConnectionBlinkTime = 0;
bool isPaired = false;  // Track if a device is connected
int currentDigit = 0;   // Keep track of which digit is currently being displayed
bool isBlinkingCommand = false;   // Tracks if we're in the middle of a 5-blink sequence
int blinkCount = 0;               // Count of blinks during command reception
unsigned long lastCommandBlinkTime = 0;  // Tracks time for blinks during command reception

// Helper function to control segment pins based on the current digit
void setSegments(int number) {
  digitalWrite(segA, digitPatterns[number][0] ? HIGH : LOW);  // Active low
  digitalWrite(segB, digitPatterns[number][1] ? HIGH : LOW);
  digitalWrite(segC, digitPatterns[number][2] ? LOW : HIGH);
  digitalWrite(segD, digitPatterns[number][3] ? HIGH : LOW);
  digitalWrite(segE, digitPatterns[number][4] ? HIGH : LOW);
  digitalWrite(segF, digitPatterns[number][5] ? HIGH : LOW);
  digitalWrite(segG, digitPatterns[number][6] ? HIGH : LOW);
}

void setSBO() {
  digitalWrite(segA, ballCount > 0 ? LOW : HIGH);  // Active low
  digitalWrite(segF, ballCount > 1 ? LOW : HIGH);
  digitalWrite(segE, ballCount > 2 ? LOW : HIGH);
  digitalWrite(segD, strikeCount > 0 ? LOW : HIGH);
  digitalWrite(segC, strikeCount > 1 ? HIGH : LOW);
  digitalWrite(segB, outCount > 0 ? LOW : HIGH);
  digitalWrite(segG, outCount > 1 ? LOW : HIGH);
}

// Function to display the scoreboard data
void printSerial() {
  String inningHalf = topOfInning ? "Top" : "Bottom";
  
  Serial.print("Inning: ");
  Serial.print(inning);
  Serial.print(" (");
  Serial.print(inningHalf);
  Serial.print(") | Home: ");
  Serial.print(homeScore);
  Serial.print(" | Guest: ");
  Serial.print(guestScore);
  Serial.print(" | Strike: ");
  Serial.print(strikeCount);
  Serial.print(" | Ball: ");
  Serial.print(ballCount);
  Serial.print(" | Out: ");
  Serial.print(outCount);
  Serial.print(" | Pitcher Countdown: ");
  Serial.println(pitcherRunning ? String(pitcherCountdown) : "Paused");
}


// Function to clear all digit control pins (turn off all digits)
void clearDigits() {
  for (int i = 0; i < 9; i++) {
    digitalWrite(digits[i], HIGH);  // Active low, so HIGH = off
  }
}

void updateDigits(){
    // Turn on the next digit in sequence
    switch (currentDigit) {
      case 0:  // Tens digit of home score
        activateDigit(0);
        setSegments(homeScore / 10);
        break;
      case 1:  // Ones digit of home score
        activateDigit(1);
        setSegments(homeScore % 10);
        break;
      case 2:  // Inning digit
        activateDigit(2);
        setSegments(inning % 10);  // Inning is a single digit (1-9)
        break;
      case 3:  // Tens digit of guest score
        activateDigit(3);
        setSegments(guestScore / 10);
        break;
      case 4:  // Ones digit of guest score
        activateDigit(4);
        setSegments(guestScore % 10);
        break;
      case 5:  // Tens digit of pitcher countdown
        activateDigit(5);
        setSegments(pitcherCountdown / 10);
        break;
      case 6:  // Ones digit of pitcher countdown
        activateDigit(6);
        setSegments(pitcherCountdown % 10);
        break;
      case 7:  // Display ball, strike, out
        activateDigit(7);
        setSBO();  // Can map strikes, balls, and outs to specific segments
        break;
      case 8:  // Display top/bottom of inning
        activateDigit(8);
        if (topOfInning) {
          digitalWrite(segA, LOW);  // Top = segment A
          digitalWrite(segD, HIGH);  // Bottom = segment D off
        } else {
          digitalWrite(segA, HIGH);  // Top off
          digitalWrite(segD, LOW);   // Bottom = segment D
        }
        break;
    }
}

// Function to activate one specific digit
void activateDigit(int digit) {
  clearDigits();
  digitalWrite(digits[digit], LOW);  // Turn on the desired digit (active low)
}

// Function to update display using time division multiplexing (non-blocking)
void updateDisplay() {
  unsigned long currentMillis = millis();

  if (!on) {
    clearDigits();
    return;
  }

  // Check if it's time to update the display
  if (currentMillis - lastUpdateTime >= displayInterval) {
    lastUpdateTime = currentMillis;

    
    // Turn on the next digit in sequence
    switch (currentDigit) {
      case 0:  // Tens digit of home score
        activateDigit(0);
        setSegments(homeScore / 10);
        break;
      case 1:  // Ones digit of home score
        activateDigit(1);
        setSegments(homeScore % 10);
        break;
      case 2:  // Inning digit
        activateDigit(2);
        setSegments(inning % 10);  // Inning is a single digit (1-9)
        break;
      case 3:  // Tens digit of guest score
        activateDigit(3);
        setSegments(guestScore / 10);
        break;
      case 4:  // Ones digit of guest score
        activateDigit(4);
        setSegments(guestScore % 10);
        break;
      case 5:  // Tens digit of pitcher countdown
        activateDigit(5);
        setSegments(pitcherCountdown / 10);
        break;
      case 6:  // Ones digit of pitcher countdown
        activateDigit(6);
        setSegments(pitcherCountdown % 10);
        break;
      case 7:  // Display ball, strike, out
        activateDigit(7);
        setSBO();  // Can map strikes, balls, and outs to specific segments
        break;
      case 8:  // Display top/bottom of inning
        activateDigit(8);
        if (topOfInning) {
          digitalWrite(segA, LOW);  // Top = segment A
          digitalWrite(segD, HIGH);  // Bottom = segment D off
        } else {
          digitalWrite(segA, HIGH);  // Top off
          digitalWrite(segD, LOW);   // Bottom = segment D
        }
        break;
    }

    // Move to the next digit
    currentDigit = (currentDigit + 1) % 9;
  }
}
// Function to handle LED blinking when not paired
void blinkLED(unsigned long currentMillis) {
  if (!isPaired && !isBlinkingCommand) {
    // Before pairing: Blink the LED every 0.5 seconds (500ms)
    if (currentMillis - lastBlinkTime >= 500) {
      lastBlinkTime = currentMillis;
      ledState = !ledState;  // Toggle LED state
      digitalWrite(ledPin, ledState ? LOW : HIGH);  // Active low: LOW = ON, HIGH = OFF
    }
  } else if (isPaired && !isBlinkingCommand) {
    // After pairing: Turn LED off
    digitalWrite(ledPin, HIGH);  // Turn LED off (active low)
  }
}

bool lightDanceActive = false;

// Function to perform the light dance sequence
void lightDance() {
    Serial.println("Starting light dance...");

    // Number of iterations for the light dance
    int danceIterations = 1;

    for (int i = 0; i < danceIterations; i++) {
        // Blink the LED
        digitalWrite(ledPin, LOW);  // LED ON
        delay(100);  // Short delay
        digitalWrite(ledPin, HIGH);  // LED OFF
        delay(100);

        // Turn on all segments of the current digit sequentially
        for (int j = 0; j < 7; j++) {
            // Cycle through each digit and light up its segments one by one
            for (int digit = 0; digit < 9; digit++) {
                activateDigit(digit);
                // Set the segments for the current digit, lighting up one segment at a time
                digitalWrite(segA, j == 0 ? LOW : HIGH);  // Active low
                digitalWrite(segB, j == 1 ? LOW : HIGH);
                digitalWrite(segC, j == 2 ? HIGH : LOW);
                digitalWrite(segD, j == 3 ? LOW : HIGH);
                digitalWrite(segE, j == 4 ? LOW : HIGH);
                digitalWrite(segF, j == 5 ? LOW : HIGH);
                digitalWrite(segG, j == 6 ? LOW : HIGH);
                delay(50);  // Delay between each segment lighting
            }
            clearDigits();  // Turn off all digits
        }

        // After each iteration, turn off all segments and wait before restarting
        clearDigits();
        delay(100);
    }

    // After finishing the light dance, turn off all LEDs and segments
    digitalWrite(ledPin, HIGH);  // Ensure LED is OFF
    clearDigits();
    Serial.println("Light dance finished.");
}

// Function to blink LED 5 times in 0.5 seconds when receiving a command
void blinkAfterCommand(unsigned long currentMillis) {
  if (isBlinkingCommand) {
    // Blink every 50ms to achieve 5 blinks in 0.5 seconds
    if (currentMillis - lastCommandBlinkTime >= 50) {
      lastCommandBlinkTime = currentMillis;
      ledState = !ledState;  // Toggle LED state for blink
      digitalWrite(ledPin, ledState ? LOW : HIGH);  // Blink LED

      // Count the number of blinks
      blinkCount++;
      if (blinkCount >= 10) {  // 10 changes = 5 blinks
        isBlinkingCommand = false;  // Stop blinking after 5 blinks
        digitalWrite(ledPin, HIGH);  // Ensure LED is off after blinking
      }
    }
  }
}

// Bluetooth command handler
void handleCommand(char command) {
    switch (command) {
        case 'I':  // INNING_UP
            inning++;
            break;
        case 'i':  // INNING_DOWN
            inning = max(1, inning - 1);
            break;
        case 'T':  // TOGGLE_TOP_BOTTOM
            topOfInning = !topOfInning;
            break;
        case 'R':  // PITCHER_RESET
            pitcherCountdown = 20;
            pitcherRunning = false;
            break;
        case 'r':  // PITCHER_RESET
            pitcherCountdown = 12;
            pitcherRunning = false;
            break;
        case 'S':  // PITCHER_START
            pitcherRunning = true;
            break;
        case 'P':  // PITCHER_STOP
            pitcherRunning = false;
            break;
        case 'H':  // TEAM1_SCORE_UP (Home score)
            homeScore++;
            break;
        case 'h':  // TEAM1_SCORE_DOWN (Home score)
            homeScore = max(0, homeScore - 1);
            break;
        case 'G':  // TEAM2_SCORE_UP (Guest score)
            guestScore++;
            break;
        case 'g':  // TEAM2_SCORE_DOWN (Guest score)
            guestScore = max(0, guestScore - 1);
            break;
        case 'K':  // STRIKE
            strikeCount = (strikeCount + 1) % 3;
            break;
        case 'B':  // BALL
            ballCount = (ballCount + 1) % 4;
            break;
        case 'O':  // OUT
            outCount = (outCount + 1) % 3;
            break;
        case 'D':  // Start the light dance mode
            lightDanceActive = true;
            lightDance();  // Trigger the light dance function
            lightDanceActive = false;  // Set back to false after the dance ends
            break;
        case 'n':
            on = ~on;
            break;
        case 'V':
            if (displayInterval < 1000) {
              displayInterval = 10000000000;
            } else {
              displayInterval = 2;
            }
            break;
        case 'N':{
          currentDigit = (currentDigit + 1) % 9;
          break;
        }
        case 'b':{
          currentDigit = (currentDigit + 8) % 9;
          break;
        }
            
    }

    // Start command blink (5 blinks in 0.5 seconds)
    isBlinkingCommand = true;
    blinkCount = 0;
    lastCommandBlinkTime = millis();  // Reset time for command blink sequence

    // Update the scoreboard display after handling command
    updateDisplay();
}

void setup() {
  // Start Serial communication
  Serial.begin(115200);

  // Start Bluetooth Serial
  SerialBT.begin("ESP32_Scoreboard");  // Bluetooth device name

  Serial.println("Bluetooth device started. You can now pair it with your device.");

  // Set up segment pins as OUTPUT and set them HIGH (off)
  pinMode(segA, OUTPUT); digitalWrite(segA, HIGH);
  pinMode(segB, OUTPUT); digitalWrite(segB, HIGH);
  pinMode(segC, OUTPUT); digitalWrite(segC, LOW);
  pinMode(segD, OUTPUT); digitalWrite(segD, HIGH);
  pinMode(segE, OUTPUT); digitalWrite(segE, HIGH);
  pinMode(segF, OUTPUT); digitalWrite(segF, HIGH);
  pinMode(segG, OUTPUT); digitalWrite(segG, HIGH);

  // Set up digit control pins as OUTPUT and set them HIGH (off)
  for (int i = 0; i < 9; i++) {
    pinMode(digits[i], OUTPUT);
    digitalWrite(digits[i], HIGH);
  }

  // Set up the LED pin as OUTPUT
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);  // LED off initially (active low)

  // Initialize the scoreboard display
  updateDisplay();
}


void loop() {
    unsigned long currentMillis = millis();

    // Check for Bluetooth connection
    if (SerialBT.hasClient()) {
        if (!isPaired) {
            isPaired = true;
            Serial.println("Device connected via Bluetooth.");
        }
    } else {
        isPaired = false;  // If no client is connected
    }

    // Handle LED blinking based on pairing status or command reception
    blinkLED(currentMillis);
    blinkAfterCommand(currentMillis);

    // Check for incoming single-character Bluetooth commands
    if (SerialBT.available()) {
        char command = SerialBT.read();  // Read one character
        Serial.print("Received command: ");
        Serial.println(command);

        handleCommand(command);  // Handle the single-character command
    }

    // Handle pitcher countdown
    if (pitcherRunning && pitcherCountdown > 0) {
        static unsigned long lastCountdownTime = 0;

        // Only decrement pitcher countdown every second
        if (currentMillis - lastCountdownTime >= 1000) {
            lastCountdownTime = currentMillis;
            pitcherCountdown--;
            updateDisplay();
        }
    }

    // Continuously update the display (multiplexing)
    updateDisplay();
}
