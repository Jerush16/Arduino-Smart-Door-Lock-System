/* Arduino-Based Smart Door Lock System
 * -------------------------------------
 * Author  : Jerush Thanusha T
 * Hardware: Arduino Uno, 4x4 Keypad, 16x2 LCD, Servo Motor
 * 
 */

#include <Keypad.h>
#include <LiquidCrystal.h>
#include <Servo.h>
#include <EEPROM.h>

// Pin Definitions

#define SERVO_PIN       10
#define LCD_RS          12
#define LCD_EN          11
#define LCD_D4           5
#define LCD_D5           4
#define LCD_D6           3
#define LCD_D7           2

// Constants

#define MAX_PASSWORD_LEN     6
#define MAX_ATTEMPTS         3
#define LOCKOUT_DURATION_MS  30000UL   // 30 seconds
#define UNLOCK_DURATION_MS    5000UL   // Door stays open 5 seconds
#define EEPROM_FLAG_ADDR        0     
#define EEPROM_PASS_ADDR        1      // Address to store password
#define EEPROM_INIT_FLAG     0xAB      

#define SERVO_LOCKED     0
#define SERVO_UNLOCKED  90

// System States

typedef enum {
  STATE_IDLE,           // Waiting for password input
  STATE_UNLOCKED,       // Door open, countdown running
  STATE_CHANGE_OLD,     // Waiting for current password (change flow)
  STATE_CHANGE_NEW,     // Waiting for new password (change flow)
  STATE_LOCKED_OUT      // Too many wrong attempts
} SystemState;

// Hardware Objects

LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
Servo lockServo;

// Keypad Configuration

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {A0, A1, A2, A3};
byte colPins[COLS] = {A4, A5, 6, 7};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Global State Variables

SystemState currentState = STATE_IDLE;

char storedPassword[MAX_PASSWORD_LEN + 1];   // Loaded from EEPROM
char inputBuffer[MAX_PASSWORD_LEN + 1];      // Current input
char oldPassBuffer[MAX_PASSWORD_LEN + 1];    // Used during password change

uint8_t inputIndex    = 0;
uint8_t wrongAttempts = 0;

unsigned long lockoutStartTime = 0;
unsigned long unlockStartTime  = 0;

// EEPROM 
// Saves password string to EEPROM starting at EEPROM_PASS_ADDR

void savePasswordToEEPROM(const char* pass) {
  for (uint8_t i = 0; i <= strlen(pass); i++) {
    EEPROM.write(EEPROM_PASS_ADDR + i, pass[i]);
  }
}

// Loads password from EEPROM into storedPassword buffer

void loadPasswordFromEEPROM() {
  for (uint8_t i = 0; i <= MAX_PASSWORD_LEN; i++) {
    storedPassword[i] = EEPROM.read(EEPROM_PASS_ADDR + i);
  }
  storedPassword[MAX_PASSWORD_LEN] = '\0';  // Ensure null-termination
}

// On first boot (no EEPROM data), write default password

void initEEPROM() {
  if (EEPROM.read(EEPROM_FLAG_ADDR) != EEPROM_INIT_FLAG) {
    savePasswordToEEPROM("1234");
    EEPROM.write(EEPROM_FLAG_ADDR, EEPROM_INIT_FLAG);
  }
  loadPasswordFromEEPROM();
}

// Display 

void showEnterPassword() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter Password:");
  lcd.setCursor(0, 1);
}

void showMessage(const char* line1, const char* line2 = "") {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  if (strlen(line2) > 0) {
    lcd.setCursor(0, 1);
    lcd.print(line2);
  }
}

// Input 

void appendToBuffer(char* buffer, char key) {
  if (inputIndex < MAX_PASSWORD_LEN) {
    buffer[inputIndex] = key;
    inputIndex++;
    buffer[inputIndex] = '\0';
    lcd.setCursor(inputIndex - 1, 1);
    lcd.print('*');
  }
}

void clearInputBuffer(char* buffer) {
  memset(buffer, 0, MAX_PASSWORD_LEN + 1);
  inputIndex = 0;
}

// State Handlers

void handleIdleState(char key) {
  if (key >= '0' && key <= '9') {
    appendToBuffer(inputBuffer, key);
  }
  else if (key == '#') {
    // Verify password
    if (strcmp(inputBuffer, storedPassword) == 0) {
      // Correct password
      wrongAttempts = 0;
      lockServo.write(SERVO_UNLOCKED);
      showMessage("Access Granted", "Door Open...");
      unlockStartTime = millis();
      currentState = STATE_UNLOCKED;
    } else {
      // Wrong password
      wrongAttempts++;
      if (wrongAttempts >= MAX_ATTEMPTS) {
        showMessage("LOCKED OUT!", "Wait 30 sec...");
        lockoutStartTime = millis();
        currentState = STATE_LOCKED_OUT;
      } else {
        char attemptsMsg[16];
        snprintf(attemptsMsg, sizeof(attemptsMsg), "Attempt %d of %d", wrongAttempts, MAX_ATTEMPTS);
        showMessage("Wrong Password", attemptsMsg);
        delay(2000);
        clearInputBuffer(inputBuffer);
        showEnterPassword();
      }
    }
    clearInputBuffer(inputBuffer);
  }
  else if (key == '*') {
    // Begin password change flow
    clearInputBuffer(oldPassBuffer);
    showMessage("Current Pass:", "");
    lcd.setCursor(0, 1);
    currentState = STATE_CHANGE_OLD;
  }
}

void handleUnlockedState() {
  // Non-blocking: auto-lock after UNLOCK_DURATION_MS
  if (millis() - unlockStartTime >= UNLOCK_DURATION_MS) {
    lockServo.write(SERVO_LOCKED);
    showMessage("Door Locked");
    delay(1500);
    showEnterPassword();
    currentState = STATE_IDLE;
  }
}

void handleLockedOutState() {
  // Non-blocking: auto-release after LOCKOUT_DURATION_MS
  if (millis() - lockoutStartTime >= LOCKOUT_DURATION_MS) {
    wrongAttempts = 0;
    showEnterPassword();
    currentState = STATE_IDLE;
  }
  // Show countdown on LCD row 2
  unsigned long remaining = (LOCKOUT_DURATION_MS - (millis() - lockoutStartTime)) / 1000;
  lcd.setCursor(0, 1);
  lcd.print("Wait: ");
  lcd.print(remaining);
  lcd.print("s   ");  // Extra spaces to clear old digits
}

void handleChangeOldState(char key) {
  if (key >= '0' && key <= '9') {
    appendToBuffer(oldPassBuffer, key);
  }
  else if (key == '#') {
    if (strcmp(oldPassBuffer, storedPassword) == 0) {
      clearInputBuffer(inputBuffer);
      showMessage("New Password:", "");
      lcd.setCursor(0, 1);
      currentState = STATE_CHANGE_NEW;
    } else {
      showMessage("Wrong Password", "Change Cancelled");
      delay(2000);
      showEnterPassword();
      currentState = STATE_IDLE;
    }
    clearInputBuffer(oldPassBuffer);
  }
}

void handleChangeNewState(char key) {
  if (key >= '0' && key <= '9') {
    appendToBuffer(inputBuffer, key);
  }
  else if (key == '#') {
    if (inputIndex >= 4) {   // Minimum 4-digit password enforced
      savePasswordToEEPROM(inputBuffer);
      loadPasswordFromEEPROM();
      showMessage("Password Saved!", "");
    } else {
      showMessage("Min 4 digits!", "Try Again");
    }
    delay(2000);
    clearInputBuffer(inputBuffer);
    showEnterPassword();
    currentState = STATE_IDLE;
  }
}

// Setup & Loop

void setup() {
  lcd.begin(16, 2);
  lockServo.attach(SERVO_PIN);
  lockServo.write(SERVO_LOCKED);

  initEEPROM();   // Load or initialize password from EEPROM

  showMessage("Smart Door Lock", "Initializing...");
  delay(2000);
  showEnterPassword();
}

void loop() {
  char key = keypad.getKey();

  switch (currentState) {
    case STATE_IDLE:
      if (key) handleIdleState(key);
      break;

    case STATE_UNLOCKED:
      handleUnlockedState();   // No key needed — time-based
      break;

    case STATE_LOCKED_OUT:
      handleLockedOutState();  // No key needed — time-based with countdown
      break;

    case STATE_CHANGE_OLD:
      if (key) handleChangeOldState(key);
      break;

    case STATE_CHANGE_NEW:
      if (key) handleChangeNewState(key);
      break;
  }
}
