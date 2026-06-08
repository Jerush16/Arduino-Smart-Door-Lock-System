# Arduino-Based Smart Door Lock System

A hardware access-control system built on Arduino Uno with keypad authentication,
servo-actuated locking, EEPROM-persisted password storage, and automatic security lockout.

> **Status:** Completed and tested on physical hardware
> **Demo:** [Watch Demo](#) ← paste your YouTube or Google Drive link here

---

## The Problem It Solves

Standard mechanical locks offer no protection against repeated forced-entry attempts
and no runtime reconfigurability. This system implements a layered security model
on an ATmega328P with only 2KB RAM:

- Digit-masked keypad input
- Configurable attempt limit with timed lockout
- Runtime password change with current-password verification
- EEPROM persistence so credentials survive power cycles

---

## Circuit Diagram

![Circuit Diagram](image/circuit_diagram.png)

---

## Technical Highlights

- **EEPROM persistence** — Password saved to non-volatile memory with magic-byte
  first-boot detection. Only written on actual change, preserving write cycle life
- **State machine architecture** — Five states (IDLE, UNLOCKED, LOCKED_OUT,
  CHANGE_OLD, CHANGE_NEW) with non-blocking transitions via `millis()`
- **No heap fragmentation** — `char` arrays with `strcmp()` used throughout instead
  of Arduino's `String` class, which causes crashes on 2KB RAM through heap fragmentation
- **Input bounds enforcement** — Buffer hard-capped at `MAX_PASSWORD_LEN` to prevent overflow
- **Live lockout countdown** — LCD shows remaining lockout seconds in real time

---

## System State Machine


---

## Hardware

See [`hardware/components.md`](hardware/components.md) for  pin connections.


|      Component           |          Role              |
|--------------------------|----------------------------|
| Arduino Uno (ATmega328P) | Main microcontroller       |
| 4×4 Matrix Keypad        | Password input             |
| 16×2 LCD (HD44780)       | User feedback              |
| Servo Motor SG90         | Physical lock actuation    |
|                          | (0° locked / 90° unlocked) |

---

## How to Run

1. Wire components per [`hardware/components.md`](hardware/components.md)
2. Install libraries via Arduino Library Manager:
   - `Keypad` by Mark Stanley
   - `LiquidCrystal` (built-in)
   - `Servo` (built-in)
   - `EEPROM` (built-in)
3. Open `SmartDoorLock/SmartDoorLock.ino` in Arduino IDE
4. Select **Board:** Arduino Uno → **Port:** your COM port → Upload
5. Default password on first boot: `1234`
6. Change password immediately: press `*`, enter current password, enter new password

---

## Challenges & Design Decisions

**Why EEPROM instead of hardcoded password?**
A lock that resets to `"1234"` on every power cycle is a functional security failure.
EEPROM stores the password non-volatilely. The magic-byte flag at address 0 ensures
default initialization only happens once on first boot.

**Why `char` arrays instead of Arduino `String`?**
Arduino's `String` uses dynamic heap allocation. On a 2KB RAM device, repeated
`+=` operations fragment the heap and cause unpredictable crashes after extended
runtime. Fixed-size `char` arrays eliminate this entirely.

**Why a state machine instead of blocking `while` loops?**
The original implementation used blocking `while` loops inside `changePassword()`,
which completely froze the main loop. A timer-based lockout or any background task
was impossible. The state machine keeps `loop()` running at all times and enables
the non-blocking countdown display during lockout.

---

## Author

**Jerush Thanusha T**
B.E. Electronics and Communication Engineering
Government College of Engineering, Tirunelveli
