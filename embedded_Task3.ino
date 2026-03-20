#include <Wire.h>
#include <RTClib.h>

RTC_DS1307 rtc;

// -------- SEGMENT PINS --------
int segB = 2;
int segF = 3;
int segA = 4;
int segG = 5;
int segC = 6;
int segD = 7;
int segE = 8;

// -------- DIGIT PINS --------
int digitLeft = 10;
int digitRight = 9;

// -------- OTHER PINS --------
int buttonPin = 11;
int ledPin = 12;

// -------- GAME STATES --------
const int IDLE = 0;
const int SHOW_TARGET = 1;
const int RUNNING = 2;

int gameState = IDLE;

// -------- GAME VARIABLES --------
int counter = 0;
int target = 5;

unsigned long lastStep = 0;
unsigned long startTime = 0;
unsigned long targetTime = 0;
unsigned long targetShowStart = 0;

const unsigned long stepInterval = 1000;
const unsigned long allowedError = 100;
const unsigned long targetShowDuration = 2000;

// ---------------- SETUP ----------------
void setup() {
  pinMode(segA, OUTPUT);
  pinMode(segB, OUTPUT);
  pinMode(segC, OUTPUT);
  pinMode(segD, OUTPUT);
  pinMode(segE, OUTPUT);
  pinMode(segF, OUTPUT);
  pinMode(segG, OUTPUT);

  pinMode(digitLeft, OUTPUT);
  pinMode(digitRight, OUTPUT);

  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);

  digitalWrite(ledPin, LOW);

  Wire.begin();

  if (!rtc.begin()) {
    while (1);
  }

  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  randomSeed(analogRead(A0));
}

// ---------------- DISPLAY FUNCTIONS ----------------
void clearSegments() {
  digitalWrite(segA, LOW);
  digitalWrite(segB, LOW);
  digitalWrite(segC, LOW);
  digitalWrite(segD, LOW);
  digitalWrite(segE, LOW);
  digitalWrite(segF, LOW);
  digitalWrite(segG, LOW);
}

void allDigitsOff() {
  digitalWrite(digitLeft, HIGH);
  digitalWrite(digitRight, HIGH);
}

void showDigit(int num) {
  clearSegments();

  if (num == 0) {
    digitalWrite(segA, HIGH);
    digitalWrite(segB, HIGH);
    digitalWrite(segC, HIGH);
    digitalWrite(segD, HIGH);
    digitalWrite(segE, HIGH);
    digitalWrite(segF, HIGH);
  }
  else if (num == 1) {
    digitalWrite(segB, HIGH);
    digitalWrite(segC, HIGH);
  }
  else if (num == 2) {
    digitalWrite(segA, HIGH);
    digitalWrite(segB, HIGH);
    digitalWrite(segD, HIGH);
    digitalWrite(segE, HIGH);
    digitalWrite(segG, HIGH);
  }
  else if (num == 3) {
    digitalWrite(segA, HIGH);
    digitalWrite(segB, HIGH);
    digitalWrite(segC, HIGH);
    digitalWrite(segD, HIGH);
    digitalWrite(segG, HIGH);
  }
  else if (num == 4) {
    digitalWrite(segB, HIGH);
    digitalWrite(segC, HIGH);
    digitalWrite(segF, HIGH);
    digitalWrite(segG, HIGH);
  }
  else if (num == 5) {
    digitalWrite(segA, HIGH);
    digitalWrite(segC, HIGH);
    digitalWrite(segD, HIGH);
    digitalWrite(segF, HIGH);
    digitalWrite(segG, HIGH);
  }
  else if (num == 6) {
    digitalWrite(segA, HIGH);
    digitalWrite(segC, HIGH);
    digitalWrite(segD, HIGH);
    digitalWrite(segE, HIGH);
    digitalWrite(segF, HIGH);
    digitalWrite(segG, HIGH);
  }
  else if (num == 7) {
    digitalWrite(segA, HIGH);
    digitalWrite(segB, HIGH);
    digitalWrite(segC, HIGH);
  }
  else if (num == 8) {
    digitalWrite(segA, HIGH);
    digitalWrite(segB, HIGH);
    digitalWrite(segC, HIGH);
    digitalWrite(segD, HIGH);
    digitalWrite(segE, HIGH);
    digitalWrite(segF, HIGH);
    digitalWrite(segG, HIGH);
  }
  else if (num == 9) {
    digitalWrite(segA, HIGH);
    digitalWrite(segB, HIGH);
    digitalWrite(segC, HIGH);
    digitalWrite(segD, HIGH);
    digitalWrite(segF, HIGH);
    digitalWrite(segG, HIGH);
  }
}

void refreshNumber(int number) {
  int tens = number / 10;
  int ones = number % 10;

  allDigitsOff();

  if (number >= 10) {
    showDigit(tens);
    digitalWrite(digitLeft, LOW);
    delay(2);
    allDigitsOff();
  }

  showDigit(ones);
  digitalWrite(digitRight, LOW);
  delay(2);

  allDigitsOff();
}

// ---------------- FEEDBACK ----------------
void successFeedback() {
  digitalWrite(ledPin, HIGH);
  delay(1000);
  digitalWrite(ledPin, LOW);
}

void failureFeedback() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(ledPin, HIGH);
    delay(200);
    digitalWrite(ledPin, LOW);
    delay(200);
  }
}

void resetGame() {
  gameState = IDLE;
  counter = 0;
  digitalWrite(ledPin, LOW);
}

// ---------------- MAIN LOOP ----------------
void loop() {
  DateTime now = rtc.now();

  // -------- IDLE --------
  if (gameState == IDLE) {
    refreshNumber(0);   // always show 0 while waiting

    if (digitalRead(buttonPin) == LOW) {
      delay(200);
      target = random(3, 10);
      targetShowStart = millis();
      gameState = SHOW_TARGET;
    }
  }

  // -------- SHOW TARGET --------
  else if (gameState == SHOW_TARGET) {
    refreshNumber(target);

    if (millis() - targetShowStart >= targetShowDuration) {
      counter = 0;
      startTime = millis();
      lastStep = startTime;
      targetTime = startTime + (target * stepInterval);
      gameState = RUNNING;
    }
  }

  // -------- RUNNING --------
  else if (gameState == RUNNING) {
    refreshNumber(counter);

    if (digitalRead(buttonPin) == LOW) {
      delay(200);

      unsigned long pressTime = millis();
      unsigned long difference;

      if (pressTime >= targetTime)
        difference = pressTime - targetTime;
      else
        difference = targetTime - pressTime;

      if (difference <= allowedError)
        successFeedback();
      else
        failureFeedback();

      resetGame();
    }

    if (millis() - lastStep >= stepInterval) {
      lastStep += stepInterval;
      counter++;

      if (counter > 10) {
        failureFeedback();
        resetGame();
      }
    }
  }
}