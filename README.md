# Arduino-Based Smart Door Lock System

A hardware access-control system built on Arduino Uno with keypad authentication,
servo-actuated locking, EEPROM-persisted password storage, and automatic security lockout.

> **Status:** Completed and tested on physical hardware
> **Demo:** [Watch 30-second demo](#) 

---

## The Problem It Solves

Standard mechanical locks offer no protection against brute-force attempts and no
runtime reconfigurability. This system implements a layered security model on an
ATmega328P microcontroller with only 2KB of RAM:

- Digit-masked keypad input
- Configurable attempt limit with timed lockout
- Runtime password change with current-password verification
- EEPROM persistence so credentials survive power cycles

---

## Circuit Diagram



---

## Technical Highlights

- **EEPROM persistence** — Password saved to non-volatile memory with magic-byte
  first-boot detection. Only written on actual password change, preserving EEPROM
  write cycle life (~100,000 writes)
- **State machine architecture** — Five states (IDLE, UNLOCKED, LOCKED_OUT,
  CHANGE_OLD, CHANGE_NEW) with non-blocking transitions via `millis()` — the main
  loop never freezes
- **No heap fragmentation** — `char` arrays with `strcmp()` / `strncpy()` used
  throughout instead of Arduino's `String` class, which causes unpredictable crashes
  on 2KB RAM systems through heap fragmentation
- **Input bounds enforcement** — Input buffer hard-capped at `MAX_PASSWORD_LEN` to
  prevent overflow
- **Live lockout countdown** — LCD displays remaining lockout seconds in real time
  using non-blocking `millis()` arithmetic

---

## System State Machine
