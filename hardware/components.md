## Hardware Components

|     Component     |  Quantity  |          Purpose        |
|-------------------|------------|-------------------------|
| Arduino Uno       | 1          | Main microcontroller    |
| 4×4 Matrix Keypad | 1          | Password input          |
| 16×2 LCD Display  | 1          | User feedback display   |
| Servo Motor       | 1          | Physical lock actuation |
| Breadboard        | 1          | Prototyping             |
| Jumper Wires      |As required | Connections             |
| USB Cable         | 1          | Programming and power   |
| Power Supply      | 1          | System power            |

## Pin Connections

| Component | Component Pin | Arduino Pin |
|-----------|---------------|-------------|
| LCD       | RS            | 12          |
| LCD       | EN            | 11          |
| LCD       | D4            | 5           |
| LCD       | D5            | 4           |
| LCD       | D6            | 3           |
| LCD       | D7            | 2           |
| Servo     | Signal        | 10 (PWM)    |
| Keypad    | Row 1–4       | A0, A1, A2, A3 |
| Keypad    | Col 1–4       | A4, A5, 6, 7 |

## Notes
- Servo SG90 draws up to 500mA at stall — power directly from Arduino 5V pin for prototyping only. For a permanent setup, used an external 5V supply.
- LCD contrast is controlled via a 10kΩ potentiometer on the V0 pin.
- No pull-up resistors needed for the keypad — the Keypad library enables internal pull-ups.
