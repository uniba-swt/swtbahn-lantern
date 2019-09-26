# SWTbahn-lantern

The SWTbahn-lantern is a synchronized LED driven point lantern based on an Attiny84A.
It is deployed on the SWTbahn Standard and connected with the BiDiB OneControl.
The OneControl supplies power and a sync signal via the GPIO to the ATtiny.


## Getting started


### Prerequisites

  - ISP- Programmer
  - Arduino IDE
  - [damellis ATtiny libary for Arduino](https://github.com/damellis/attiny)

### Programming

 alskdjflkasjdf

  1. Open lantern.ino in Arduino IDE
  2. Select "Tools" in the upper drop-down bar and configure the settings regarding the programmer

  Example:

  ```
  Board: "ATtiny/24/48/84"
  Processor: "ATtiny84"
  Clock: "Internal 8 MHz"
  Port: "COM4"
  ```

  3. Click on compile (tick) and then click on upload (arrow)

If any errors occurs check the power supply or the ISP header.

### Notes

Interrupt for the Sync Pin (see Datasheet p. 50)

```
GIMSK = (1 << PCIE1); // Pin Change Interrupt Enable 1
GIFR = (1 << PCIF1); // Pin Change Interrupt Flag 1
PCMSK1 = (1 << SYNC_INT); //Pin Change Enable Mask 9
```

Setup for Sync-Timer (see Datasheet p. 85)

```
// Timer/Counter 1 at 800
TCCR1A = 0; // Clear register TCCR1A
TCCR1B = 0; // Clear register TCCR1B
TCNT1  = 0; // Reset counter value
// Set compare match register for 600Hz increments
OCR1A = 40;  // = 8MHz/(3kHz*64 prescaler) - 1, (OCR0A value must be less than 65536)
TCCR1B |= (1 << WGM12); // Enable Clear Timer on Capture mode
TCCR1B |= (1 << CS11) | (1 << CS10);  // Prescaler of 64
TIMSK1 |= (1 << OCIE1A);  // Enable timer compare interrupt
```

For more details check out the [SWTbahn documentation](https://vc.uni-bamberg.de/moodle/course/view.php?id=26901 "SWTbahn").
