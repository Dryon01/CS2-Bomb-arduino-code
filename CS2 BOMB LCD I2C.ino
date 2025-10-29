#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // adresa I2C obișnuită (verifică dacă e 0x3F la tine)

const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
byte rowPins[ROWS] = {2, 3, 4, 5};
byte colPins[COLS] = {6, 7, 8};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

const int buzzerPin = 10;
const int ledPin = 11;

String plantCode = "1234#";
String disarmCode = "4321#";
String inputCode = "";

const int START_SECONDS = 40;
int timerSeconds = START_SECONDS;
unsigned long lastTimerMillis = 0;
unsigned long nextBeepMillis = 0;
unsigned long beepEndMillis = 0;
bool beepActive = false;
int state = 0; // 0 - standby, 1 - plantat, 2 - dezamorsat, 3 - explodat

void setup() {
 // dacă ești sigur că SDA/SCL sunt pe A0/A1
  lcd.init();
  lcd.backlight();
  lcd.print("BOMBA ARMATA!");
  delay(1000);
  lcd.clear();
  lcd.print("Introdu cod:");

  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  noTone(buzzerPin);
  lastTimerMillis = millis();
  nextBeepMillis = millis() + 1000;
}

void loop() {
  unsigned long now = millis();
  char key = keypad.getKey();

  // --- citire tastatură ---
  if (key) {
    if (key == '*') {
      inputCode = ""; // șterge codul
      lcd.setCursor(0, 1);
      lcd.print("                "); // curăță linia
    } else if (key == '#') {
      // verifică codul complet
      if (inputCode + "#" == plantCode && state == 0) {
        armaBomba(now);
      } else if (inputCode + "#" == disarmCode && state == 1) {
        dezamorseazaBomba();
      } else {
        lcd.clear();
        lcd.print("Cod gresit!");
        delay(700);
        afisTimp();
      }
      inputCode = "";
    } else {
      if (inputCode.length() < 4) inputCode += key;
      lcd.setCursor(0, 1);
      lcd.print(inputCode);
    }
  }

  // --- cronometru ---
  if (state == 1 && now - lastTimerMillis >= 1000) {
    lastTimerMillis += 1000;
    timerSeconds--;
    if (timerSeconds < 0) timerSeconds = 0;
    afisTimp();
  }

  // --- beep accelerat ---
  if (state == 1) {
    unsigned long interval = map(constrain(timerSeconds, 0, START_SECONDS), 0, START_SECONDS, 120, 1000);
    int freq = map(constrain(timerSeconds, 0, START_SECONDS), 0, START_SECONDS, 1600, 800);
    if (now >= nextBeepMillis) {
      unsigned long beepDur = (interval < 200) ? interval / 2 : 100;
      tone(buzzerPin, freq, beepDur);
      digitalWrite(ledPin, HIGH);
      beepActive = true;
      beepEndMillis = now + beepDur;
      nextBeepMillis = now + interval;
    }
  }

  if (beepActive && now >= beepEndMillis) {
    digitalWrite(ledPin, LOW);
    beepActive = false;
  }

  // --- explozie ---
  if (state == 1 && timerSeconds == 0) {
    explodeBomba();
  }
}

void armaBomba(unsigned long now) {
  state = 1;
  lcd.clear();
  lcd.print("BOMBA PLANTATA!");
  delay(600);
  timerSeconds = START_SECONDS;
  lastTimerMillis = now;
  nextBeepMillis = now;
  beepActive = false;
  lcd.clear();
  afisTimp();
}

void dezamorseazaBomba() {
  state = 2;
  stopBeep();
  digitalWrite(ledPin, LOW);
  lcd.clear();
  lcd.print("DEZAMORSATA!");
  delay(1000);
  lcd.clear();
  lcd.print("Introdu cod:");
  state = 0;
}

void explodeBomba() {
  state = 3;
  lcd.clear();
  lcd.print("BOMBA EXPLODATA!");
  for (int i = 0; i < 5; i++) {
    tone(buzzerPin, 1600, 80);
    digitalWrite(ledPin, HIGH);
    delay(80);
    digitalWrite(ledPin, LOW);
    delay(40);
  }
  for (int f = 2000; f > 400; f -= 200) {
    tone(buzzerPin, f, 50);
    digitalWrite(ledPin, HIGH);
    delay(50);
    digitalWrite(ledPin, LOW);
    delay(20);
  }
  stopBeep();
  delay(500);
  lcd.clear();
  lcd.print("Introdu cod:");
  state = 0;
  timerSeconds = START_SECONDS;
  inputCode = "";
}

void afisTimp() {
  lcd.clear();
  lcd.print("Timp ramas: ");
  lcd.print(timerSeconds);
  lcd.print("s");
}

void stopBeep() {
  noTone(buzzerPin);
  digitalWrite(ledPin, LOW);
  beepActive = false;
}
